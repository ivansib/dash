#include <boost/test/unit_test.hpp>
#include "test/test_sibcoin.h"
#include "dex/unconfirmedoffers.h"

using namespace dex;

void checkPutOffers()
{
    UnconfirmedOffers unc;

    CDexOffer offer;
    offer.pubKey = GetRandHash().GetHex();
    offer.hash = GetRandHash();
    offer.idTransaction = GetRandHash();
    offer.type = Buy;
    offer.status = Active;
    offer.price = 1234567;
    offer.minAmount = 10000;
    offer.shortInfo = "first info";
    offer.countryIso = "RU";
    offer.currencyIso = "RUB";
    offer.paymentMethod = 1;
    offer.timeCreate = 1025668989;
    offer.timeExpiration = 1027778989;
    offer.timeModification = 1027770000;
    offer.editingVersion = 5;
    offer.bcst_tx = MakeTransactionRef();

    unc.putOffer(offer);

    CDexOffer offer1 = offer;
    offer1.price = 1234568;
    offer1.idTransaction = GetRandHash();
    offer1.bcst_tx = MakeTransactionRef();

    unc.putOffer(offer1);

    auto uncList = unc.getAllOffers();

    BOOST_CHECK(uncList.size() == 1);

    std::vector<CDexOffer> vec;
    vec.push_back(offer);
    vec.push_back(offer1);

    unc.putOffers(vec);

    uncList = unc.getAllOffers();

    BOOST_CHECK(uncList.size() == 1);

    CDexOffer offer2;
    offer2.pubKey = GetRandHash().GetHex();
    offer2.hash = GetRandHash();
    offer2.idTransaction = GetRandHash();
    offer2.type = Buy;
    offer2.status = Active;
    offer2.price = 1234567;
    offer2.minAmount = 10000;
    offer2.shortInfo = "first info";
    offer2.countryIso = "RU";
    offer2.currencyIso = "RUB";
    offer2.paymentMethod = 1;
    offer2.timeCreate = 1025668989;
    offer2.timeExpiration = 1027778989;
    offer2.timeModification = 1027770000;
    offer2.editingVersion = 5;
    offer2.bcst_tx = MakeTransactionRef();


    unc.putOffer(offer2);

    uncList = unc.getAllOffers();

    BOOST_CHECK(uncList.size() == 2);
}


BOOST_FIXTURE_TEST_SUITE(unconfirmedoffers_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(unconfirmedoffers_test1)
{
    checkPutOffers();
}

BOOST_AUTO_TEST_SUITE_END()
