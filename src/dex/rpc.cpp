
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
#include <boost/foreach.hpp>
#include <univalue.h>

using namespace std;


UniValue dexoffer(const JSONRPCRequest& request)
{

    if (request.fHelp)
        throw runtime_error(
            "dexoffer \n"
            "Create TEST dex offer and broadcast it.\n"
        );

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
