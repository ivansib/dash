
#include "rpc/server.h"

#include "clientversion.h"
#include "net.h"
#include "netbase.h"
#include "protocol.h"
#include "sync.h"
#include "timedata.h"
#include "util.h"
#include "utilstrencodings.h"
#include "version.h"
#include <univalue.h>
#include "streams.h"

#include "dexoffer.h"
#include "random.h"
#include "dex/dexdb.h"

using namespace std;


UniValue dexoffer(const JSONRPCRequest& request)
{

    if (request.fHelp)
        throw runtime_error(
            "dexoffer \n"
            "Create TEST dex offer and broadcast it.\n"
        );

    CDexOffer offer;

    if (!offer.Create(GetRandHash(), CDexOffer::SELL, "RU", "RUB", 1, 123456, 1233, 10, "short info", "details"))
        throw runtime_error("dexoffer not create\n someting wrong\n");

    dex::DexDB db(strDexDbFile);
    if (offer.isBuy())  db.addOfferBuy(offer);
    if (offer.isSell()) db.addOfferSell(offer);

    LOCK2(cs_main, cs_vNodes);

    BOOST_FOREACH(CNode* pNode, vNodes) {
        pNode->PushMessage(NetMsgType::DEXOFFBCST, offer);
    }


    return NullUniValue;
}

static const CRPCCommand commands[] =
{ //  category              name                        actor (function)           okSafeMode
    //  --------------------- ------------------------    -----------------------    ----------
    { "dex",    "dexoffer",       &dexoffer,       true,  {} }
};

void RegisterDexRPCCommands(CRPCTable &t)
{
    for (unsigned int vcidx = 0; vcidx < ARRAYLEN(commands); vcidx++)
        t.appendCommand(commands[vcidx].name, &commands[vcidx]);
}
