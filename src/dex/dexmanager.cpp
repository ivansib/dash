
#include "dexmanager.h"

#include "consensus/validation.h"
#include "net_processing.h"
#include "init.h"
#include "wallet/wallet.h"
#include "util.h"
#include "utilstrencodings.h"
#include "masternodeman.h"
#include "masternode-sync.h"
#include "netmessagemaker.h"

#include "dexsync.h"
#include "txmempool.h"
#include "base58.h"


namespace dex {

CDexManager dexman;


CDexManager::CDexManager()
{
    db = nullptr;
    uncOffers = new UnconfirmedOffers();
    uncBcstOffers = new UnconfirmedOffers();
}

CDexManager::~CDexManager()
{
    if (db != nullptr) {
        db->freeInstance();
    }

    delete uncOffers;
    delete uncBcstOffers;
}

void CDexManager::ProcessMessage(CNode* pfrom, const std::string& strCommand, CDataStream& vRecv, CConnman &connman)
{
    initDB();

    if (strCommand == NetMsgType::DEXOFFBCST) {
        getAndSendNewOffer(pfrom, vRecv, connman);
    } else if (strCommand == NetMsgType::DEXDELOFFER) {
        getAndDelOffer(pfrom, vRecv, connman);
    } else if (strCommand == NetMsgType::DEXOFFEDIT) {
        getAndSendEditedOffer(pfrom, vRecv, connman);
    }
}

void CDexManager::addOrEditDraftMyOffer(MyOfferInfo &myOffer)
{
    initDB();

    myOffer.status = Draft;

    CDexOffer dexOffer;
    uint256 oldHash = myOffer.hash;
    dexOffer.Create(myOffer);

    if (!oldHash.IsNull() && oldHash != dexOffer.hash) {
        db->deleteMyOfferByHash(oldHash);
    }

    myOffer.setOfferInfo(dexOffer);

    saveMyOffer(myOffer);
}

void CDexManager::prepareAndSendMyOffer(MyOfferInfo &myOffer, std::string &error)
{
    initDB();

    CDexOffer dexOffer;

    if (myOffer.status == Indefined || myOffer.status == Draft) {
        uint256 oldHash = myOffer.hash;
        dexOffer.Create(myOffer);

        if (oldHash != dexOffer.hash) {
            db->deleteMyOfferByHash(oldHash);
        }
    } else if (myOffer.status == Active) {
        myOffer.editingVersion++;

        dexOffer = CDexOffer(myOffer);
    }

    CPubKey pubKey = dexOffer.getPubKeyObject();
    if (!pwalletMain->HaveKey(pubKey.GetID())) {
        error = "Don't find pub key";
        return;
    }

    CDex dex(dexOffer);

    CKey key;
    if (!dex.FindKey(key, error)) {
        return;
    }
    if (!dex.MakeEditSign(key, error)) {
        return;
    }

    uint256 tx;

    if (!dexOffer.idTransaction.IsNull()) {
        if (!dex.CheckOfferTx(error)) {
            return;
        }

        dexman.sendEditedOffer(dex.offer);

        if (dex.offer.isBuy()) {
            db->editOfferBuy(dex.offer);
        }
        if (dex.offer.isSell()) {
            db->editOfferSell(dex.offer);
        }
    } else if (dex.PayForOffer(tx, error)) {
        dexman.sendNewOffer(dex.offer, dex.getPayTx());

        myOffer.setOfferInfo(dex.offer);
        myOffer.status = Unconfirmed;
        if (!uncOffers->putOffer(dex.offer)) {
            error = "Error adding to unconfirmed offers";
            return;
        }
    }

    if (error.empty()) {
        saveMyOffer(myOffer);
    }
}

void CDexManager::sendNewOffer(const CDexOffer &offer, const CTransaction &tx)
{
    auto vNodesCopy = g_connman->CopyNodeVector();

    for (auto pNode : vNodesCopy) {
        if (pNode->nVersion < MIN_DEX_VERSION) {
            continue;
        }

        g_connman->PushMessage(pNode, CNetMsgMaker(pNode->GetSendVersion()).Make(NetMsgType::DEXOFFBCST, offer, tx));
    }

    g_connman->ReleaseNodeVector(vNodesCopy);
}

void CDexManager::sendEditedOffer(const CDexOffer &offer)
{
    std::vector<unsigned char> vchSign = ParseHex(offer.editsign);

    auto vNodesCopy = g_connman->CopyNodeVector();

    for (auto pNode : vNodesCopy) {
        if (pNode->nVersion < MIN_DEX_VERSION) {
            continue;
        }

        g_connman->PushMessage(pNode, CNetMsgMaker(pNode->GetSendVersion()).Make(NetMsgType::DEXOFFEDIT, offer, vchSign));
    }

    g_connman->ReleaseNodeVector(vNodesCopy);
}

void CDexManager::checkUncOffers()
{
    checkUncBcstOffers();

    auto list = uncOffers->getAllOffers();

    for (auto it = list.cbegin(); it != list.cend(); ) {
        std::vector<std::list<CDexOffer>::const_iterator> offersBuy;
        std::vector<std::list<CDexOffer>::const_iterator> offersSell;
        std::vector<std::list<CDexOffer>::const_iterator> myOffers;
        std::vector<CDexOffer> offersRemove;
        for (int cnt = 0; it != list.cend() && cnt < 100; cnt++, it++)  {
            CDex dex((*it));
            std::string error;
            if (dex.CheckOfferTx(error)) {
                if ((*it).isBuy())  {
                    if (!db->isExistOfferBuyByHash((*it).hash)) {
                      offersBuy.push_back(it);
                    }
                }

                if ((*it).isSell())  {
                    if (!db->isExistOfferSellByHash((*it).hash)) {
                      offersSell.push_back(it);
                    }
                }

                if ((*it).isMyOffer())  {
                    myOffers.push_back(it);
                }

                offersRemove.push_back(*it);
            }
        }

        db->begin();

        for (auto offer : offersBuy) {
            db->addOfferBuy(*offer);
        }
        for (auto offer : offersSell) {
            db->addOfferSell(*offer);
        }
        for (auto offer : myOffers) {
            db->editStatusForMyOffer((*offer).idTransaction, Active);
        }

        if (db->commit() == 0) {
            uncOffers->removeOffers(offersRemove);
        }
    }

}

void CDexManager::setStatusExpiredForMyOffers()
{
    db->setStatusExpiredForMyOffers();
}

void CDexManager::deleteOldUncOffers()
{
    uncOffers->deleteOldOffers();
}

void CDexManager::deleteOldOffers()
{
    db->deleteOldOffersBuy();
    db->deleteOldOffersSell();
}

void CDexManager::initDB()
{
    if (db == nullptr) {
        db = DexDB::instance();
    }
}

void CDexManager::getAndSendNewOffer(CNode *pfrom, CDataStream &vRecv, CConnman &connman)
{
    LogPrint("dex", "DEXOFFBCST -- get new offer from = %s\n", pfrom->addr.ToString());

    CDexOffer offer;
    CTransactionRef tx;
    vRecv >> offer >> tx;
    int fine = 0;
    if (offer.Check(true, fine)) {
        CDex dex(offer);
        std::string error;

        if (uncOffers->hasOffer(offer) || uncBcstOffers->hasOffer(offer)) {
            LogPrint("dex", "DEXOFFBCST -- uncOffers already has offer: %s\n", offer.hash.GetHex().c_str());
        } else {
            bool bFound = false;
            if (offer.isBuy() && db->isExistOfferBuyByHash(offer.hash)) {
                bFound = true;
            }
            if (offer.isSell() && db->isExistOfferSellByHash(offer.hash)) {
                bFound = true;
            }

            LogPrint("dex", "DEXOFFBCST --\n%s\nfound %d\n", offer.dump().c_str(), bFound);
            if (!bFound) {
                if (dex.CheckBRCSTOfferTx(tx, error)) {
                    offer.bcst_tx = tx;
                    if (uncBcstOffers->putOffer(offer)) {
                        LogPrint("dex", "DEXOFFBCST -- added in uncOffes and send a offer: %s\n", offer.hash.GetHex().c_str());
                        auto vNodesCopy = connman.CopyNodeVector();
                        for (auto pNode : vNodesCopy) {
                            if (pNode->nVersion < MIN_DEX_VERSION) {
                                continue;
                            }

                            if (pNode->addr != pfrom->addr) {
                                connman.PushMessage(pNode, CNetMsgMaker(pNode->GetSendVersion()).Make(NetMsgType::DEXOFFBCST, offer, tx));
                            }
                        }
                        connman.ReleaseNodeVector(vNodesCopy);
                    }

                } else {
                    LogPrint("dex", "DEXOFFBCST -- check offer transaction fail\n");
                    Misbehaving(pfrom->GetId(), 20);
                }
            }
        }
    } else {
        LogPrint("dex", "DEXOFFBCST -- offer check fail\n");
        Misbehaving(pfrom->GetId(), fine);
    }
}

void CDexManager::getAndDelOffer(CNode *pfrom, CDataStream &vRecv, CConnman &connman)
{
    LogPrint("dex", "DEXDELOFFER -- receive request on delete offer from = %s\n", pfrom->addr.ToString());

    std::vector<unsigned char> vchSign;
    CDexOffer offer;
    vRecv >> offer;
    vRecv >> vchSign;

    int fine = 0;
    if (offer.Check(true, fine)) {
        CDex dex(offer);
        std::string error;
        if (dex.CheckOfferSign(vchSign, error)) {
            bool bFound = false;
            if (offer.isBuy())  {
                if (db->isExistOfferBuyByHash(offer.hash)) {
                    db->deleteOfferBuyByHash(offer.hash);
                    bFound = true;
                }
            }

            if (offer.isSell())  {
                if (db->isExistOfferSellByHash(offer.hash)) {
                    db->deleteOfferSellByHash(offer.hash);
                    bFound = true;
                }
            }
            if (!bFound) {
                bFound = uncBcstOffers->removeOffer(offer);
                if (!bFound) {
                    bFound = uncOffers->removeOffer(offer);
                }
            }

            if (bFound) { // need to delete and relay
                auto vNodesCopy = connman.CopyNodeVector();
                for (auto pNode : vNodesCopy) {
                    if (pNode->nVersion < MIN_DEX_VERSION) {
                        continue;
                    }

                    if (pNode->addr != pfrom->addr) {
                        connman.PushMessage(pNode, CNetMsgMaker(pNode->GetSendVersion()).Make(NetMsgType::DEXDELOFFER, offer, vchSign));
                    }
                }

                connman.ReleaseNodeVector(vNodesCopy);
            }
            LogPrint("dex", "DEXDELOFFER --\n%s\nfound %d\n", offer.dump().c_str(), bFound);
        } else {
            LogPrint("dex", "DEXDELOFFER --check offer sign fail(%s)\n", offer.hash.GetHex().c_str());
        }
    } else {
        LogPrint("dex", "DEXDELOFFER -- offer check fail\n");
        Misbehaving(pfrom->GetId(), fine);
    }
}



void CDexManager::getAndSendEditedOffer(CNode *pfrom, CDataStream& vRecv, CConnman &connman)
{
    LogPrint("dex", "DEXOFFEDIT -- receive request on edited offer from = %s\n", pfrom->addr.ToString());

    std::vector<unsigned char> vchSign;
    CDexOffer offer;
    vRecv >> offer;
    vRecv >> vchSign;
    offer.editsign = HexStr(vchSign);

    int fine = 0;
    if (offer.Check(true, fine) && offer.CheckModTimeOnEdit()) {
        if (offer.CheckEditSign()) {
            CDex dex(offer);
            std::string error;
            bool isActual = false;
            if (dex.CheckOfferTx(error)) {
                if (offer.isBuy()) {
                    if (db->isExistOfferBuyByHash(offer.hash)) {
                        OfferInfo existOffer = db->getOfferBuyByHash(offer.hash);
                        if (offer.editingVersion > existOffer.editingVersion) {
                            db->editOfferBuy(offer);
                            isActual = true;
                        }
                    } else {
                        db->addOfferBuy(offer);
                        isActual = true;
                    }
                }

                if (offer.isSell()) {
                    if (db->isExistOfferSellByHash(offer.hash)) {
                        OfferInfo existOffer = db->getOfferSellByHash(offer.hash);
                        if (offer.editingVersion > existOffer.editingVersion) {
                            db->editOfferSell(offer);
                            isActual = true;
                        }
                    } else {
                        db->addOfferSell(offer);
                        isActual = true;
                    }
                }
                LogPrint("dex", "DEXOFFEDIT --\n%s\nactual %d\n", offer.dump().c_str(), isActual);
            } else {
                uncBcstOffers->updateOffer(offer);
                isActual = uncOffers->updateOffer(offer);
                LogPrint("dex", "DEXOFFEDIT --check offer tx fail(%s)\n", offer.idTransaction.GetHex().c_str());
            }
            if (isActual) {
                auto vNodesCopy = connman.CopyNodeVector();
                for (auto pNode : vNodesCopy) {
                    if (pNode->nVersion < MIN_DEX_VERSION) {
                        continue;
                    }

                    if (pNode->addr != pfrom->addr) {
                        connman.PushMessage(pNode, CNetMsgMaker(pNode->GetSendVersion()).Make(NetMsgType::DEXOFFEDIT, offer, vchSign));
                    }
                }

                connman.ReleaseNodeVector(vNodesCopy);
            }
        } else {
            LogPrint("DEXOFFEDIT -- invalid signature, hash: %s \n", offer.hash.GetHex().c_str());
            Misbehaving(pfrom->GetId(), 5);
        }
    } else {
        LogPrint("DEXOFFEDIT -- offer check fail: %s\n", offer.hash.GetHex().c_str());
        Misbehaving(pfrom->GetId(), fine);
    }
}

void CDexManager::checkUncBcstOffers()
{
    auto list = uncBcstOffers->getAllOffers();
    std::vector<CDexOffer> voffers;

    for (auto i : list) {
        if (!i.bcst_tx->IsNull()) {
            CTransactionRef ptx;
            uint256 hashBlock;
            if (!GetTransaction(i.bcst_tx->GetHash(), ptx, Params().GetConsensus(), hashBlock, true)) {
                CValidationState state;
                bool fMissingInputs = false;
                if (AcceptToMemoryPool(mempool, state, i.bcst_tx, true, &fMissingInputs)) {
                    const CTransaction& tx = *ptx;
                    g_connman->RelayTransaction(tx);
                } else {
                    LogPrint("dex", "Add broadcast tx to mempool error: %s\n", FormatStateMessage(state).c_str());
                    continue;
                }
            }

            voffers.push_back(i);
        }
    }
    uncOffers->putOffers(voffers);
    uncBcstOffers->removeOffers(list);
}

std::list<std::pair<uint256, uint32_t>> CDexManager::availableOfferHashAndVersion() const
{
    auto list = db->getHashsAndEditingVersionsSell();
    auto buyList = db->getHashsAndEditingVersionsBuy();
    list.sort();
    buyList.sort();
    list.merge(buyList);

    for (auto offer : uncOffers->getAllOffers()) {
        list.push_back(std::make_pair(offer.hash, offer.editingVersion));
    }

    for (auto offer : uncBcstOffers->getAllOffers()) {
        list.push_back(std::make_pair(offer.hash, offer.editingVersion));
    }

    return list;
}

std::list<std::pair<uint256, uint32_t> > CDexManager::availableOfferHashAndVersionFromBD(const DexDB::OffersPeriod &from, const uint64_t &timeMod) const
{
    auto list = db->getHashsAndEditingVersionsSell(from, timeMod);
    auto buyList = db->getHashsAndEditingVersionsBuy(from, timeMod);
    list.sort();
    buyList.sort();
    list.merge(buyList);

    return list;
}

std::list<std::pair<uint256, uint32_t> > CDexManager::availableOfferHashAndVersionFromUnc() const
{
    std::list<std::pair<uint256, uint32_t> > list;
    for (auto offer : uncOffers->getAllOffers()) {
        list.push_back(std::make_pair(offer.hash, offer.editingVersion));
    }
    for (auto offer : uncBcstOffers->getAllOffers()) {
        list.push_back(std::make_pair(offer.hash, offer.editingVersion));
    }

    return list;
}

CDexOffer CDexManager::getOfferInfo(const uint256 &hash) const
{
    if (db->isExistOfferSellByHash(hash)) {
        return CDexOffer(db->getOfferSellByHash(hash), Sell);
    }

    if (db->isExistOfferBuyByHash(hash)) {
        return CDexOffer(db->getOfferBuyByHash(hash), Buy);
    }

    CDexOffer uncOffer = uncBcstOffers->getOfferByHash(hash);
    if (uncOffer.IsNull()) {
        uncOffer = uncOffers->getOfferByHash(hash);
    }
    if (!uncOffer.IsNull()) {
        return uncOffer;
    }

    return CDexOffer();
}

UnconfirmedOffers *CDexManager::getUncOffers() const
{
    return uncOffers;
}

UnconfirmedOffers *CDexManager::getBcstUncOffers() const
{
    return uncBcstOffers;
}


void CDexManager::saveMyOffer(const MyOfferInfo &info, bool usethread)
{
    if (db->isExistMyOfferByHash(info.hash)) {
        db->editMyOffer(info);
    } else {
        db->addMyOffer(info);
    }
}

}

using namespace dex;

void CheckDexMasternode(const std::vector<CNode*> &vNodes)
{
    int nOutbound = 0;
    int nDex = 0;
    std::vector<CNode *> nodeToRemove;
    std::set<std::vector<unsigned char> > setConnected;

    for (auto pNode : vNodes) {
        if (!pNode->fInbound && !pNode->fMasternode) {
            nOutbound++;

            LogPrint("dex", "CheckDexMasternode -- outbound node: %s\n", pNode->addr.ToString());

            if (pNode->nVersion >= MIN_DEX_VERSION && mnodeman.isExist(pNode)) {
                LogPrint("dex", "CheckDexMasternode -- dex node: %s\n", pNode->addr.ToString());
                nDex++;
            } else {
                LogPrint("dex", "CheckDexMasternode -- node to remove: %s\n", pNode->addr.ToString());
                nodeToRemove.push_back(pNode);
            }
        }
    }

    int minNumDexNode = dexsync.minNumDexNode();
    if ((MAX_OUTBOUND_CONNECTIONS == nOutbound) && nDex < minNumDexNode) {
        unsigned nDis = minNumDexNode - nDex;

        for (unsigned i = 0; i < nDis; i++) {
            if (nodeToRemove.size() > i) {
                nodeToRemove[i]->fDisconnect = true;
            } else {
                break;
            }
        }
    }
}
