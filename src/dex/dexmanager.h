
#ifndef __DEX_MANAGER_H__
#define __DEX_MANAGER_H__

#include "key.h"
#include "net.h"
#include "dex/db/dexdto.h"
#include "dex/db/dexdb.h"
#include "dex.h"
#include "dexoffer.h"
#include "unconfirmedoffers.h"


namespace dex {

class CDexManager;
extern CDexManager dexman;

static const int MIN_DEX_PROTO_VERSION = 70207;

class CDexManager
{
public:
    CDexManager();
    ~CDexManager();

    void ProcessMessage(CNode* pfrom, const std::string &strCommand, CDataStream& vRecv, CConnman& connman);

    void addOrEditDraftMyOffer(MyOfferInfo &myOffer);
    void prepareAndSendMyOffer(MyOfferInfo &myOffer, std::string &error);
    void sendNewOffer(const CDexOffer &offer, const CTransaction &tx);
    void sendEditedOffer(const CDexOffer &offer);
    void checkUncOffers();
    void setStatusExpiredForMyOffers();
    void deleteOldUncOffers();
    void deleteOldOffers();
    void initDB();

    std::list<std::pair<uint256, uint32_t> > availableOfferHashAndVersion() const;
    std::list<std::pair<uint256, uint32_t> > availableOfferHashAndVersionFromBD(const DexDB::OffersPeriod &from, const uint64_t &timeMod) const;
    std::list<std::pair<uint256, uint32_t> > availableOfferHashAndVersionFromUnc() const;
    CDexOffer getOfferInfo(const uint256 &hash) const;
    UnconfirmedOffers *getUncOffers() const;
    UnconfirmedOffers *getBcstUncOffers() const;

    boost::signals2::signal<void()> startSyncDex;

private:
    DexDB *db;
    UnconfirmedOffers *uncOffers;
    UnconfirmedOffers *uncBcstOffers;

    void getAndSendNewOffer(CNode* pfrom, CDataStream& vRecv, CConnman &connman);
    void getAndDelOffer(CNode* pfrom, CDataStream& vRecv, CConnman &connman);
    void getAndSendEditedOffer(CNode* pfrom, CDataStream& vRecv, CConnman &connman);
    void checkUncBcstOffers();

    void saveMyOffer(const MyOfferInfo &info, bool usethread = true);
};

}

void CheckDexMasternode(const std::vector<CNode *> &vNodes);

#endif // __DEX_MANAGER_H__
