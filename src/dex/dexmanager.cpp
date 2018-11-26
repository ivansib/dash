
#include "dexmanager.h"
#include "netmessagemaker.h"

#include "init.h"
#include "util.h"

#include "dexoffer.h"
#include "dex/dexdb.h"


CDexManager dexman;




CDexManager::CDexManager()
{
}



void CDexManager::ProcessMessage(CNode* pfrom, const std::string& strCommand, CDataStream& vRecv)
{
    if (strCommand == NetMsgType::DEXOFFBCST) {
        CDexOffer offer;
        vRecv >> offer;
        if (offer.Check(true)) {
            dex::DexDB db(strDexDbFile);
            bool bFound = false;
            if (offer.isBuy())  {
                if (db.isExistOfferBuy(offer.idTransaction)) {
                  bFound = true;
                } else {
                    db.addOfferBuy(offer);
                }
            }

            if (offer.isSell())  {
                if (db.isExistOfferSell(offer.idTransaction)) {
                  bFound = true;
                } else {
                    db.addOfferSell(offer);
                }
            }

            if (!bFound) { // need to save and relay
                auto vNodes = g_connman->CopyNodeVector(CConnman::FullyConnectedOnly);
                for (auto pNode : vNodes) {
                    CNetMsgMaker msgMaker(pNode->GetSendVersion());
                    g_connman->PushMessage(pNode, msgMaker.Make(NetMsgType::DEXOFFBCST, offer));
                }
                g_connman->ReleaseNodeVector(vNodes);
            }
            LogPrintf("DEXOFFBCST --\n%s\nfound %d\n", offer.dump().c_str(), bFound);
        }
    }
}


