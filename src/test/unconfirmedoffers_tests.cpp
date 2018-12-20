#include <boost/test/unit_test.hpp>
#include "test/test_sibcoin.h"
#include "dex/unconfirmedoffers.h"

using namespace dex;

void checkPutOffers()
{
    UnconfirmedOffers unc;

    std::vector<CDexOffer> listOffers;
    for (int i = 0; i < 10; i++) {
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

        listOffers.push_back(offer);
    }

    unc.putOffer(listOffers[0]);
    unc.putOffer(listOffers[0]);

    unc.putOffer(listOffers[1]);
    unc.putOffer(listOffers[1]);

    unc.putOffer(listOffers[2]);
    unc.putOffer(listOffers[2]);

    auto size = unc.getSize();

    BOOST_CHECK(size == 3);

    unc.putOffers(listOffers);
    size = unc.getSize();

    BOOST_CHECK(size == 10);

    bool isRemove = unc.removeOffer(listOffers[5]);

    BOOST_CHECK(isRemove);

    size = unc.getSize();
    auto uncList = unc.getAllOffers();

    BOOST_CHECK(size == 9);
    BOOST_CHECK(uncList.size() == size);

    auto offer1 = listOffers[7];

    BOOST_CHECK(unc.hasOfferWithHash(offer1.hash));

    auto offer2 = unc.getOfferByHash(offer1.hash);

    BOOST_CHECK(offer1.pubKey == offer2.pubKey);
    BOOST_CHECK(offer1.idTransaction == offer2.idTransaction);
    BOOST_CHECK(offer1.hash == offer2.hash);
    BOOST_CHECK(offer1.type == offer2.type);
    BOOST_CHECK(offer1.status == offer2.status);
    BOOST_CHECK(offer1.price == offer2.price);
    BOOST_CHECK(offer1.minAmount == offer2.minAmount);
    BOOST_CHECK(offer1.shortInfo == offer2.shortInfo);
    BOOST_CHECK(offer1.countryIso == offer2.countryIso);
    BOOST_CHECK(offer1.currencyIso == offer2.currencyIso);
    BOOST_CHECK(offer1.paymentMethod == offer2.paymentMethod);
    BOOST_CHECK(offer1.timeCreate == offer2.timeCreate);
    BOOST_CHECK(offer1.timeExpiration == offer2.timeExpiration);
    BOOST_CHECK(offer1.timeModification == offer2.timeModification);
    BOOST_CHECK(offer1.editingVersion == offer2.editingVersion);
    BOOST_CHECK(offer1.bcst_tx == offer2.bcst_tx);

    unc.removeOffers(listOffers);
    size = unc.getSize();

    BOOST_CHECK(size == 0);
}


BOOST_FIXTURE_TEST_SUITE(unconfirmedoffers_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(unconfirmedoffers_test1)
{
    checkPutOffers();
}

BOOST_AUTO_TEST_SUITE_END()
