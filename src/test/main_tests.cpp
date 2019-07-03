// Copyright (c) 2014-2016 The Bitcoin Core developers
// Copyright (c) 2017 The Raven Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "validation.h"
#include "net.h"

#include "test/test_bitcoin.h"

#include <boost/signals2/signal.hpp>
#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(main_tests, TestingSetup)

    static void TestBlockSubsidyHalvings(const Consensus::Params &consensusParams)
    {
        int maxHalvings = 64;
        int nHeight = 1;
        CAmount nInitialSubsidy = 50 * COIN;
        CAmount nInitialLinearSubsidy = 1 * COIN;

        CAmount nPreviousSubsidy = nInitialSubsidy; // for height == 0
        for (nHeight = 1; nHeight < 65; nHeight++)
        {
            CAmount nSubsidy = GetTotalReward(nHeight, consensusParams);
            BOOST_CHECK_EQUAL(nSubsidy, nInitialLinearSubsidy * nHeight);
        }
        for (nHeight = 65; nHeight < 1000; nHeight+=5)
        {
            CAmount nSubsidy = GetTotalReward(nHeight, consensusParams);
            BOOST_CHECK_EQUAL(nSubsidy, nInitialSubsidy);
        }
        BOOST_CHECK_EQUAL(nPreviousSubsidy, nInitialSubsidy);
        for (int nHalvings = 1; nHalvings < maxHalvings; nHalvings++)
        {
            nHeight = nHalvings * consensusParams.nSubsidyHalvingInterval;
            // TODO: update GetBlockSubsidy call
            CAmount nSubsidy = GetTotalReward(nHeight, consensusParams);
            BOOST_CHECK(nSubsidy <= nInitialSubsidy);
            BOOST_CHECK_EQUAL(nSubsidy, nPreviousSubsidy / 2);
            nPreviousSubsidy = nSubsidy;
        }
        // TODO: update GetBlockSubsidy call
        BOOST_CHECK_EQUAL(GetTotalReward(maxHalvings * consensusParams.nSubsidyHalvingInterval, consensusParams), 0);
    }

    static void TestBlockSubsidyHalvings(int nSubsidyHalvingInterval)
    {
        Consensus::Params consensusParams;
        consensusParams.nSubsidyHalvingInterval = nSubsidyHalvingInterval;
        TestBlockSubsidyHalvings(consensusParams);
    }

    BOOST_AUTO_TEST_CASE(block_subsidy_test)
    {
        BOOST_TEST_MESSAGE("Running Block Subsidy Test");

        const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
        TestBlockSubsidyHalvings(chainParams->GetConsensus()); // As in main
        //TestBlockSubsidyHalvings(64); // As in regtest
        TestBlockSubsidyHalvings(1000); // Just another interval
    }

    BOOST_AUTO_TEST_CASE(subsidy_limit_test)
    {
        BOOST_TEST_MESSAGE("Running Subsidy Limit Test");

        const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
        CAmount nSum = 0;
        int nHeight = 0;
        for (nHeight = 0; nHeight < 65; nHeight += 1)
        {
            // TODO: update GetBlockSubsidy call
            CAmount nSubsidy = GetTotalReward(nHeight, chainParams->GetConsensus());
            if (nHeight == 0) {
                BOOST_CHECK_EQUAL(nSubsidy, 1 * COIN);
            } else {
                BOOST_CHECK_EQUAL(nSubsidy, nHeight * COIN);
            }
            nSum += nSubsidy;
            BOOST_CHECK(MoneyRange(nSum));
        }
        for (nHeight = 65; nHeight < 1000; nHeight += 5)
        {
            // TODO: update GetBlockSubsidy call
            CAmount nSubsidy = GetTotalReward(nHeight, chainParams->GetConsensus());
            BOOST_CHECK_EQUAL(nSubsidy, 50 * COIN);
            nSum += nSubsidy * 5;
            BOOST_CHECK(MoneyRange(nSum));
        }
        for (nHeight = 1000; nHeight < 41000000; nHeight += 1000)
        {
            // TODO: update GetBlockSubsidy call
            CAmount nSubsidy = GetTotalReward(nHeight, chainParams->GetConsensus());
            BOOST_CHECK(nSubsidy <= 50 * COIN);
            nSum += nSubsidy * 1000;
            BOOST_CHECK(MoneyRange(nSum));
        }
        BOOST_CHECK_EQUAL(nSum, 6399883092960000ULL);
    }

    bool ReturnFalse()
    { return false; }

    bool ReturnTrue()
    { return true; }

    BOOST_AUTO_TEST_CASE(combiner_all_test)
    {
        BOOST_TEST_MESSAGE("Running Combiner All Test");

        boost::signals2::signal<bool(), CombinerAll> Test;
        BOOST_CHECK(Test());
        Test.connect(&ReturnFalse);
        BOOST_CHECK(!Test());
        Test.connect(&ReturnTrue);
        BOOST_CHECK(!Test());
        Test.disconnect(&ReturnFalse);
        BOOST_CHECK(Test());
        Test.disconnect(&ReturnTrue);
        BOOST_CHECK(Test());
    }

BOOST_AUTO_TEST_SUITE_END()
