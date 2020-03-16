// Copyright (c) 2019 Veil developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include <test/test_raven.h>

#include <boost/test/unit_test.hpp>

#include <crypto/ethash/lib/ethash/endianness.hpp>
#include <crypto/ethash/include/ethash/progpow.hpp>

#include "crypto/ethash/helpers.hpp"
#include "crypto/ethash/progpow_test_vectors.hpp"

#include <array>

BOOST_FIXTURE_TEST_SUITE(progpow_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(progpow_l1_cache)
{
    auto& context = get_ethash_epoch_context_0();

    constexpr auto test_size = 20;
    std::array<uint32_t, test_size> cache_slice;
    for (size_t i = 0; i < cache_slice.size(); ++i)
    cache_slice[i] = ethash::le::uint32(context.l1_cache[i]);

    const std::array<uint32_t, test_size> expected{
        {690150178, 1181503948, 2248155602, 2118233073, 2193871115, 1791778428, 1067701239,
                    724807309, 530799275, 3480325829, 3899029234, 1998124059, 2541974622, 1100859971,
                    1297211151, 3268320000, 2217813733, 2690422980, 3172863319, 2651064309}};
    int i = 0;
    for (auto item : cache_slice) {
        BOOST_CHECK(item == expected[i]);
        i++;
    }
}

BOOST_AUTO_TEST_CASE(progpow_hash_empty)
{
    auto& context = get_ethash_epoch_context_0();

    printf("Starting block 1000 %u\n", GetTimeMillis());
    int count = 1000;
    ethash_result result;
    while (count > 0) {
        result = progpow::hash(context, count, {}, 0);
        --count;
    }

    const auto mix_hex = "4b0050b03f6823e8ed6aabfceb2debca378682f987c7127a1ade109621b90afd";
    const auto final_hex = "412ce3770325c5802cf27cab26df1525fcd3f20b4c026b8d798120fffb96243c";
    printf("Ending block 0 %u\n", GetTimeMillis());
    BOOST_CHECK_EQUAL(to_hex(result.mix_hash), mix_hex);
    BOOST_CHECK_EQUAL(to_hex(result.final_hash), final_hex);
}

BOOST_AUTO_TEST_CASE(progpow_hash_1000_times)
{
        const int block_number = 5000000;
        const auto header =
        to_hash256("0xffeeddccbbaa9988776655443322110000112233445566778899aabbccddeeff");
        const uint64_t nonce = 0x123456789abcdef0;

        int start = GetTime();
        printf("Creating epoch context\n");
        auto context = ethash::create_epoch_context(ethash::get_epoch_number(block_number));
        int nTime2 = GetTime();
        printf("Finished epoch context: %d seconds\n", nTime2 - start);

    printf("Starting Hashing\n");
    int count = 1000;
    const auto mix_hex = "a385016758034394b1dbd37e5209e80124533724326996abb42eb6248d82a214";
    const auto final_hex = "9510408e88b5a0dd8856d44d04602cbf35ff2a418b78c92011319003bacfb9ee";
    while (count > 0) {
        const auto result = progpow::hash(*context, block_number, header, nonce);
        BOOST_CHECK_EQUAL(to_hex(result.mix_hash), mix_hex);
        BOOST_CHECK_EQUAL(to_hex(result.final_hash), final_hex);
        --count;
    }
    auto finished = GetTime() - nTime2;
    printf("Finished hashing %d seconds\n", finished);

}

BOOST_AUTO_TEST_CASE(progpow_hash_and_verify)
{
    ethash::epoch_context_ptr context{nullptr, nullptr};

    for (auto& t : progpow_hash_test_cases)
    {
        const auto epoch_number = ethash::get_epoch_number(t.block_number);
        if (!context || context->epoch_number != epoch_number)
            context = ethash::create_epoch_context(epoch_number);

        const auto header_hash = to_hash256(t.header_hash_hex);
        const auto nonce = std::stoull(t.nonce_hex, nullptr, 16);
        const auto result = progpow::hash(*context, t.block_number, header_hash, nonce);
        BOOST_CHECK_EQUAL(to_hex(result.mix_hash), t.mix_hash_hex);
        BOOST_CHECK_EQUAL(to_hex(result.final_hash), t.final_hash_hex);

        auto success = progpow::verify(
                *context, t.block_number, header_hash, result.mix_hash, nonce, result.final_hash);
        BOOST_CHECK(success);

        auto lower_boundary = result.final_hash;
        --lower_boundary.bytes[31];
        auto final_failure = progpow::verify(
                *context, t.block_number, header_hash, result.mix_hash, nonce, lower_boundary);
        BOOST_CHECK(!final_failure);

        auto different_mix = result.mix_hash;
        ++different_mix.bytes[7];
        auto mix_failure = progpow::verify(
                *context, t.block_number, header_hash, different_mix, nonce, result.final_hash);
        BOOST_CHECK(!mix_failure);
    }
}

BOOST_AUTO_TEST_CASE(progpow_search)
{
    auto ctxp = ethash::create_epoch_context_full(0);
    auto& ctx = *ctxp;
    auto& ctxl = reinterpret_cast<const ethash::epoch_context&>(ctx);

    auto boundary = to_hash256("00ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
    auto sr = progpow::search(ctx, 0, {}, boundary, 100, 100);
    auto srl = progpow::search_light(ctxl, 0, {}, boundary, 100, 100);

    BOOST_CHECK(sr.mix_hash == ethash::hash256{});
    BOOST_CHECK(sr.final_hash == ethash::hash256{});
    BOOST_CHECK(sr.nonce == 0x0);
    BOOST_CHECK(sr.mix_hash == srl.mix_hash);
    BOOST_CHECK(sr.final_hash == srl.final_hash);
    BOOST_CHECK(sr.nonce == srl.nonce);

    // Switch it to a different starting nonce and find another solution
    sr = progpow::search(ctx, 0, {}, boundary, 0, 100);
    srl = progpow::search_light(ctxl, 0, {}, boundary, 0, 100);

    BOOST_CHECK(sr.mix_hash != ethash::hash256{});
    BOOST_CHECK(sr.final_hash != ethash::hash256{});
    BOOST_CHECK(sr.nonce == 74);
    BOOST_CHECK(sr.mix_hash == srl.mix_hash);
    BOOST_CHECK(sr.final_hash == srl.final_hash);
    BOOST_CHECK(sr.nonce == srl.nonce);

    auto r = progpow::hash(ctx, 0, {}, 74);
    BOOST_CHECK(sr.final_hash == r.final_hash);
    BOOST_CHECK(sr.mix_hash == r.mix_hash);
}

BOOST_AUTO_TEST_SUITE_END()