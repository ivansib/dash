
#ifndef __DEX_MANAGER_H__
#define __DEX_MANAGER_H__

#include "key.h"
#include "net.h"
#include "dex/db/dexdto.h"
#include "dex/db/dexdb.h"
#include "dex.h"
#include "dexoffer.h"
#include "unconfirmedoffers.h"

class CDexManager;
extern CDexManager dexman;

using namespace dex;

class CDexManager
{
public:
    CDexManager();
    ~CDexManager();

    void ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv);

    void addOrEditDraftMyOffer(MyOfferInfo &myOffer);
    void prepareAndSendMyOffer(MyOfferInfo &myOffer, std::string &error);
    void sendNewOffer(const CDexOffer &offer);
    void sendEditedOffer(const CDexOffer &offer);
    void checkUncOffers();
    void setStatusExpiredForMyOffers();
    void deleteOldUncOffers();
    void deleteOldOffers();

    std::list<std::pair<uint256, int> > availableOfferHashAndVersion() const;
    CDexOffer getOfferInfo(const uint256 &hash) const;
    UnconfirmedOffers *getUncOffers() const;

private:
    DexDB *db;
    UnconfirmedOffers *uncOffers;

    void initDB();
    void getAndSendNewOffer(CNode* pfrom, CDataStream& vRecv);
    void getAndSendEditedOffer(CDataStream& vRecv);
    void getAndDelOffer(CNode* pfrom, CDataStream& vRecv);
    void getAndSendEditedOffer(CNode* pfrom, CDataStream& vRecv);

    void saveMyOffer(const MyOfferInfo &info);
};

void ThreadDexManager();

#endif // __DEX_MANAGER_H__
