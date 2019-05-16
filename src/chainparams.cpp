// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2017 The Raven Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "consensus/merkle.h"

#include "tinyformat.h"
#include "util.h"
#include "utilstrencodings.h"
#include "arith_uint256.h"

#include <assert.h>
#include <auxpow/consensus.h>

#include "chainparamsseeds.h"

//TODO: Take these out
extern double algoHashTotal[16];
extern int algoHashHits[16];


static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
 *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: 4a5e1e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "The Merkle 03/Dec/2017 Three Crypto Projects Attempting to Penetrate the Video Game Industry";
    const CScript genesisOutputScript = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

void CChainParams::UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    consensus.vDeployments[d].nStartTime = nStartTime;
    consensus.vDeployments[d].nTimeout = nTimeout;
}

void CChainParams::TurnOffSegwit() {
	consensus.nSegwitEnabled = false;
}

void CChainParams::TurnOffCSV() {
	consensus.nCSVEnabled = false;
}

void CChainParams::TurnOffBIP34() {
	consensus.nBIP34Enabled = false;
}

void CChainParams::TurnOffBIP65() {
	consensus.nBIP65Enabled = false;
}

void CChainParams::TurnOffBIP66() {
	consensus.nBIP66Enabled = false;
}

bool CChainParams::BIP34() {
	return consensus.nBIP34Enabled;
}

bool CChainParams::BIP65() {
	return consensus.nBIP34Enabled;
}

bool CChainParams::BIP66() {
	return consensus.nBIP34Enabled;
}

bool CChainParams::CSVEnabled() const{
	return consensus.nCSVEnabled;
}


/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        consensus.nSubsidyHalvingInterval = 640000; // Halving every 640,000 blocks
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.nMasternodePaymentsStartBlock = 2; // not true, but it's ok as long as it's less then nMasternodePaymentsIncreaseBlock
        consensus.nMasternodePaymentsIncreaseBlock = 2; // actual historical value
        consensus.nMasternodePaymentsIncreasePeriod = 576*30; // 17280 - actual historical value
        consensus.nMasternodeMinimumConfirmations = 15;
        consensus.nGovernanceMinQuorum = 10;
        consensus.nGovernanceFilterElements = 20000;
        consensus.nShareFeeBlock = 175000;  // TODO: sandip set the share fee block
        consensus.nSeniorityInterval = 43800 * 4; // seniority increases every 4
		consensus.nTotalSeniorityIntervals = 9;

        consensus.minerRewardPercent = 80;
        consensus.masternodeRewardPercent = 15;
        consensus.devRewardPercent = 5;
        consensus.devWalletAddress = "8QD1FZNN41npesU92uRAs4ZBCZ2ccviFP7";

        consensus.BIP34Height = 0;
        consensus.BIP34Hash = uint256S("0x000000001d03315ec89ac7895ad122e3ae9c3c92a2ec5f63000794cdd6c18095"); // Genesis hash (height 0)

        consensus.nBIP34Enabled = true; // 00000000a6a47e28b4fea2ab47262d9a420bb1600dee375cad30fa54c9f6ec90
        consensus.nBIP65Enabled = false; // 000000000000000004c2b624ed5d7756c508d90fd0da2c7c679febfa6c4735f0
        consensus.nBIP66Enabled = true;
        consensus.nSegwitEnabled = false;
        consensus.nCSVEnabled = true;
        consensus.powLimit = uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff"); // ~uint256(0) >> 32;
        consensus.nPowTargetTimespan = 32 * 60; // 32 min
        consensus.nPowTargetSpacing = 1 * 32;  // 32 sec
        
        consensus.nAuxpowChainId = 0x1940; // Auxpow Chain ID = 6464
        consensus.nAlternateChainId = 0x00A4; // Auxpow Chain ID = 164
        consensus.nChainIdUpgradeHeight = 796000; // Switch to Chain ID 164 after block 796000
        consensus.fStrictChainId = false;

        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 54; // 95% of 60
        consensus.nMinerConfirmationWindow = 60; // nPowTargetTimespan / nPowTargetSpacing

        consensus.nBlockV4UpgradeHeight = 470000; // Miners produce v4 blocks after height 470000
        consensus.nBlockVersionRevertHeight = 2000000; // Stop forcing block v4 to allow asset activation

        consensus.nAuxPowStartHeight = AuxPow::START_MAINNET;

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_ASSETS].bit = 6;  //Assets (RIP2)
        consensus.vDeployments[Consensus::DEPLOYMENT_ASSETS].nStartTime = 9999999999; // Oct 31, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_ASSETS].nTimeout = 9999999999; // Oct 31, 2019

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1512550966; // December 6th, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1519146928; // February 20th, 2018

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1512550966; // December 6th, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1519146928; // February 20th, 2018

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000007bcf1a7820259035581");  // Height 741975

        //TODO - Set this to genesis block
        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00"); // 

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xb3;
        pchMessageStart[1] = 0xc4;
        pchMessageStart[2] = 0xfd;
        pchMessageStart[3] = 0xa2;
        nDefaultPort = 64640;
        nPruneAfterHeight = 100000;

        genesis = CreateGenesisBlock(1512550966, 476339731, 0x1d00ffff, 1, 1 * COIN);

        consensus.hashGenesisBlock = genesis.GetHash();

        assert(consensus.hashGenesisBlock == uint256S("0x00000000a6a47e28b4fea2ab47262d9a420bb1600dee375cad30fa54c9f6ec90"));
        assert(genesis.hashMerkleRoot == uint256S("0xccc9ed60efe7224e7ea404369d246390d1b8f09f33268dcb6d66f3c5707232ae"));

        vSeeds.emplace_back("seed.blastblastblast.com", false);
        vSeeds.emplace_back("seed1.blastblastblast.com", false);
        vSeeds.emplace_back("seed2.blastblastblast.com", false);

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,25);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,18);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fMiningRequiresPeers = true;
        fAllowMultiplePorts = false;
        nFulfilledRequestExpireTime = 60*60; // fulfilled requests expire in 1 hour

        strSporkAddress = "bLa2RK9P7MqBLUGN4jcZXUM9nSgaVaYyxw";
        checkpointData = (CCheckpointData) {
            {
                {      0, uint256S("0x00000000a6a47e28b4fea2ab47262d9a420bb1600dee375cad30fa54c9f6ec90")},
                {  88238, uint256S("0x0000000000012ed053d9c1f2221803df06f57c982b264c9f6289cd5db88404d2")},
                { 136700, uint256S("0x9f5833e664b0d283ef89ca8e6cf2a1ca0355199475da2f29679e11e07733ef19")},
                { 289999, uint256S("0xcffbc77ecbf3c6b208405192d70acf600dff4adaaf985785d9a366b2595032d1")},
                { 429998, uint256S("0x0000000000000211e981e385f0bb709b6793deed2dfd06ca2597f06f6020b90d")},
                { 470001, uint256S("0x0000000000000091e9201aac0d7de4644d840f77343a383abcbfe4cdb143d00c")},
                { 741975, uint256S("0xc0e8ef621936909064f627478d7031adda683bbcd7477cffd8eca571cf868c32")},
            }
        };

        chainTxData = ChainTxData{
            // Update as we know more about the contents of the Raven chain
            // Stats as of c0e8ef621936909064f627478d7031adda683bbcd7477cffd8eca571cf868c32
            1538172088, // * UNIX timestamp of last known number of transactions
            768047,     // * total number of transactions between genesis and that timestamp
                        //   (the tx=... number in the SetBestChain debug.log lines)
            2700.0      // * estimated number of transactions per second after that timestamp
        };

        /** BLAST Start **/
        // Burn Amounts
        nIssueAssetBurnAmount = 500 * COIN;
        nReissueAssetBurnAmount = 100 * COIN;
        nIssueSubAssetBurnAmount = 100 * COIN;
        nIssueUniqueAssetBurnAmount = 5 * COIN;

        // Burn Addresses
        strIssueAssetBurnAddress = "RXissueAssetXXXXXXXXXXXXXXXXXhhZGt";
        strReissueAssetBurnAddress = "RXReissueAssetXXXXXXXXXXXXXXVEFAWu";
        strIssueSubAssetBurnAddress = "RXissueSubAssetXXXXXXXXXXXXXWcwhwL";
        strIssueUniqueAssetBurnAddress = "RXissueUniqueAssetXXXXXXXXXXWEAe58";

        //Global Burn Address
        strGlobalBurnAddress = "RXBurnXXXXXXXXXXXXXXXXXXXXXXWUo9FV";

        // DGW Activation
        nDGWActivationBlock = 0;

        nMaxReorganizationDepth = 60; // 60 at 32 second block timespan is +/- 32 minutes.
        nMinReorganizationPeers = 4;
        nMinReorganizationAge = 60 * 60 * 12; // 12 hours
        mnCollateralAmount = 25000;
        /** BLAST End **/
    }
};
static CMainParams mainParams;

/**
 * Testnet (v6)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.nSubsidyHalvingInterval = 640000; // Halving every 640,000 blocks
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.nMasternodePaymentsStartBlock = 2;
        consensus.nMasternodePaymentsIncreaseBlock = 2; 
        consensus.nMasternodePaymentsIncreasePeriod = 10;
        consensus.nMasternodeMinimumConfirmations = 1;
        consensus.nGovernanceMinQuorum = 1;
        consensus.nGovernanceFilterElements = 500;
        consensus.nShareFeeBlock = 1000;
        consensus.nSeniorityInterval = 60; // seniority increases every hour

        consensus.minerRewardPercent = 80;
        consensus.masternodeRewardPercent = 15;
        consensus.devRewardPercent = 5;
        consensus.devWalletAddress = "8QD1FZNN41npesU92uRAs4ZBCZ2ccviFP7";

		consensus.nTotalSeniorityIntervals = 9;
        consensus.BIP34Height = 0;
        consensus.BIP34Hash = uint256S("0x0000000077062645faa71f32cd3fe871f4ca4f145d16781ff3ccaedf3ad2f1cd"); // Genesis hash (height 0)
        
        consensus.nBIP34Enabled = true; //  
        consensus.nBIP65Enabled = false; // 000000000000000004c2b624ed5d7756c508d90fd0da2c7c679febfa6c4735f0
        consensus.nBIP66Enabled = true;
        consensus.nSegwitEnabled = false;
        consensus.nCSVEnabled = true;
        consensus.powLimit = uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff"); // ~uint256(0) >> 32;
        consensus.nPowTargetTimespan = 32 * 60; // 32 min
        consensus.nPowTargetSpacing = 1 * 32;  // 32 sec

        consensus.nAuxpowChainId = 0x1940; // Auxpow Chain ID = 6464
        consensus.nAlternateChainId = 0x00A4; // Auxpow Chain ID = 164
        consensus.nChainIdUpgradeHeight = 30; // Switch to Chain ID 164 after block 30
        consensus.fStrictChainId = false;

        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 54; // 95% of 60
        consensus.nMinerConfirmationWindow = 60; // nPowTargetTimespan / nPowTargetSpacing

        consensus.nBlockV4UpgradeHeight = 200; // Miners produce v4 blocks after height 200 (after CSV/SEGWIT activation at 180)
        consensus.nBlockVersionRevertHeight = 300; // Stop forcing block v4 to allow asset activation
        
        consensus.nAuxPowStartHeight = AuxPow::START_TESTNET;

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_ASSETS].bit = 6;
        consensus.vDeployments[Consensus::DEPLOYMENT_ASSETS].nStartTime = 1553835600; // GMT: Friday, March 29, 2019 5:00:00 AM
        consensus.vDeployments[Consensus::DEPLOYMENT_ASSETS].nTimeout = 1584403653; // GMT: Tuesday, March 17, 2020 12:07:33 AM

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1552781253; // GMT: Sunday, March 17, 2019 12:07:33 AM
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1584403653; // GMT: Tuesday, March 17, 2020 12:07:33 AM

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1552781253; // GMT: Sunday, March 17, 2019 12:07:33 AM
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1584403653; // GMT: Tuesday, March 17, 2020 12:07:33 AM


        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");


        pchMessageStart[0] = 0xb3;
        pchMessageStart[1] = 0xc4;
        pchMessageStart[2] = 0xfd;
        pchMessageStart[3] = 0xa2;
        nDefaultPort = 64320;
        nPruneAfterHeight = 1000;

        uint32_t nGenesisTime = 1552795319;  // GMT: Sunday, March 17, 2019 4:01:59 AM

        // This is used inorder to mine the genesis block. Once found, we can use the nonce and block hash found to create a valid genesis block
//        /////////////////////////////////////////////////////////////////


//        arith_uint256 test;
//        bool fNegative;
//        bool fOverflow;
//        test.SetCompact(0x1e00ffff, &fNegative, &fOverflow);
//        std::cout << "Test threshold: " << test.GetHex() << "\n\n";
//
//        int genesisNonce = 0;
//        uint256 TempHashHolding = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");
//        uint256 BestBlockHash = uint256S("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
//        for (int i=0;i<40000000;i++) {
//            genesis = CreateGenesisBlock(nGenesisTime, i, 0x1e00ffff, 2, 5000 * COIN);
//            //genesis.hashPrevBlock = TempHashHolding;
//            consensus.hashGenesisBlock = genesis.GetHash();
//
//            arith_uint256 BestBlockHashArith = UintToArith256(BestBlockHash);
//            if (UintToArith256(consensus.hashGenesisBlock) < BestBlockHashArith) {
//                BestBlockHash = consensus.hashGenesisBlock;
//                std::cout << BestBlockHash.GetHex() << " Nonce: " << i << "\n";
//                std::cout << "   PrevBlockHash: " << genesis.hashPrevBlock.GetHex() << "\n";
//            }
//
//            TempHashHolding = consensus.hashGenesisBlock;
//
//            if (BestBlockHashArith < test) {
//                genesisNonce = i - 1;
//                break;
//            }
//            //std::cout << consensus.hashGenesisBlock.GetHex() << "\n";
//        }
//        std::cout << "\n";
//        std::cout << "\n";
//        std::cout << "\n";
//
//        std::cout << "hashGenesisBlock to 0x" << BestBlockHash.GetHex() << std::endl;
//        std::cout << "Genesis Nonce to " << genesisNonce << std::endl;
//        std::cout << "Genesis Merkle " << genesis.hashMerkleRoot.GetHex() << std::endl;
//
//        std::cout << "\n";
//        std::cout << "\n";
//        int totalHits = 0;
//        double totalTime = 0.0;
//
//        for(int x = 0; x < 16; x++) {
//            totalHits += algoHashHits[x];
//            totalTime += algoHashTotal[x];
//            std::cout << "hash algo " << x << " hits " << algoHashHits[x] << " total " << algoHashTotal[x] << " avg " << algoHashTotal[x]/algoHashHits[x] << std::endl;
//        }
//
//        std::cout << "Totals: hash algo " <<  " hits " << totalHits << " total " << totalTime << " avg " << totalTime/totalHits << std::endl;
//
//        genesis.hashPrevBlock = TempHashHolding;
//
//        return;

//        /////////////////////////////////////////////////////////////////

        genesis = CreateGenesisBlock(nGenesisTime, 497372947, 0x1d00ffff, 1, 1 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();

        //Test MerkleRoot and GenesisBlock
        assert(consensus.hashGenesisBlock == uint256S("0x0000000077062645faa71f32cd3fe871f4ca4f145d16781ff3ccaedf3ad2f1cd"));
        assert(genesis.hashMerkleRoot == uint256S("0xccc9ed60efe7224e7ea404369d246390d1b8f09f33268dcb6d66f3c5707232ae"));

        vFixedSeeds.clear();
        vSeeds.clear();

        vSeeds.emplace_back("testseed.blastblastblast.com", false);

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,85);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,18);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;
        fMiningRequiresPeers = true;
        fAllowMultiplePorts = false;
        nFulfilledRequestExpireTime = 5*60; // fulfilled requests expire in 5 minutes

        strSporkAddress = "bWYetMgxDgut45aaDNF7ySPJ66NTwjTDZc";
        checkpointData = (CCheckpointData) {
            {
                {      0, uint256S("0x000000001d03315ec89ac7895ad122e3ae9c3c92a2ec5f63000794cdd6c18095")},
            }
        };

        chainTxData = ChainTxData{
            // Update as we know more about the contents of the Raven chain
            // Stats as of 000000001d03315ec89ac7895ad122e3ae9c3c92a2ec5f63000794cdd6c18095
            1536875774, // * UNIX timestamp of last known number of transactions
            0,          // * total number of transactions between genesis and that timestamp
                        //   (the tx=... number in the SetBestChain debug.log lines)
            1           // * estimated number of transactions per second after that timestamp
        };

        /** BLAST Start **/
        // Burn Amounts
        nIssueAssetBurnAmount = 2 * COIN;
        nReissueAssetBurnAmount = 1 * COIN;
        nIssueSubAssetBurnAmount = 1 * COIN;
        nIssueUniqueAssetBurnAmount = 1 * COIN;

        // Burn Addresses
        strIssueAssetBurnAddress = "8R6xzzuVcDWM4KM1xC47AgvWF3nGSp5E4b"; // "n1issueAssetXXXXXXXXXXXXXXXXWdnemQ";
        strReissueAssetBurnAddress = "8R6xzzuVcDWM4KM1xC47AgvWF3nGSp5E4b"; // "n1ReissueAssetXXXXXXXXXXXXXXWG9NLd";
        strIssueSubAssetBurnAddress = "8R6xzzuVcDWM4KM1xC47AgvWF3nGSp5E4b"; // "n1issueSubAssetXXXXXXXXXXXXXbNiH6v";
        strIssueUniqueAssetBurnAddress = "8R6xzzuVcDWM4KM1xC47AgvWF3nGSp5E4b"; // "n1issueUniqueAssetXXXXXXXXXXS4695i";

        // Global Burn Address
        strGlobalBurnAddress = "8R6xzzuVcDWM4KM1xC47AgvWF3nGSp5E4b"; // "n1BurnXXXXXXXXXXXXXXXXXXXXXXU1qejP";

        // DGW Activation
        nDGWActivationBlock = 0;

        nMaxReorganizationDepth = 60; // 60 at 32 second block timespan is +/- 32 minutes.
        nMinReorganizationPeers = 4;
        mnCollateralAmount = 10;
        nMinReorganizationAge = 60 * 60 * 12; // 12 hours
        /** BLAST End **/

    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        consensus.nSubsidyHalvingInterval = 100; // Halving every 100 blocks
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.nMasternodePaymentsStartBlock = 4010; // not true, but it's ok as long as it's less then nMasternodePaymentsIncreaseBlock
        consensus.nMasternodePaymentsIncreaseBlock = 4030;
        consensus.nMasternodePaymentsIncreasePeriod = 10;
        consensus.nMasternodeMinimumConfirmations = 1;
        consensus.nGovernanceMinQuorum = 1;
        consensus.nGovernanceFilterElements = 100;
        consensus.nShareFeeBlock = 1;
        consensus.nSeniorityInterval = 60; // seniority increases every hour

        consensus.minerRewardPercent = 80;
        consensus.masternodeRewardPercent = 15;
        consensus.devRewardPercent = 5;
        consensus.devWalletAddress = "8QD1FZNN41npesU92uRAs4ZBCZ2ccviFP7";

		consensus.nTotalSeniorityIntervals = 9;
        consensus.BIP34Height = -1; // BIP34 has not necessarily activated on regtest
        consensus.BIP34Hash = uint256();
        
        consensus.nBIP34Enabled = true;
        consensus.nBIP65Enabled = true; // 000000000000000004c2b624ed5d7756c508d90fd0da2c7c679febfa6c4735f0
        consensus.nBIP66Enabled = true;
        consensus.nSegwitEnabled = false;
        consensus.nCSVEnabled = true;
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 32 * 60; // 32 min
        consensus.nPowTargetSpacing = 1 * 32;  // 32 sec

        consensus.nAuxpowChainId = 0x1940; // Auxpow Chain ID = 6464
        consensus.nAlternateChainId = 0x00A4; // Auxpow Chain ID = 164
        consensus.nChainIdUpgradeHeight = 0; // Switch to Chain ID 164 after block 0
        consensus.fStrictChainId = false;

        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 24; // 75% for testchains
        consensus.nMinerConfirmationWindow = 32; // Faster than normal for regtest (32 instead of 60)

        consensus.nBlockV4UpgradeHeight = 100; // Miners produce v4 blocks after height 100
        consensus.nBlockVersionRevertHeight = 120; // Stop forcing block v4 to allow asset activation

        consensus.nAuxPowStartHeight = AuxPow::START_REGTEST;
        
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_ASSETS].bit = 6;
        consensus.vDeployments[Consensus::DEPLOYMENT_ASSETS].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_ASSETS].nTimeout = 999999999999ULL;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        pchMessageStart[0] = 0xc5;
        pchMessageStart[1] = 0xc7;
        pchMessageStart[2] = 0xd1;
        pchMessageStart[3] = 0xee;
        nDefaultPort = 32320;
        nPruneAfterHeight = 1000;

// This is used inorder to mine the genesis block. Once found, we can use the nonce and block hash found to create a valid genesis block
//        /////////////////////////////////////////////////////////////////
//
//
//        arith_uint256 test;
//        bool fNegative;
//        bool fOverflow;
//        test.SetCompact(0x207fffff, &fNegative, &fOverflow);
//        std::cout << "Test threshold: " << test.GetHex() << "\n\n";
//
//        int genesisNonce = 0;
//        uint256 TempHashHolding = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");
//        uint256 BestBlockHash = uint256S("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
//        for (int i=0;i<40000000;i++) {
//            genesis = CreateGenesisBlock(1533751200, i, 0x207fffff, 2, 5000 * COIN);
//            //genesis.hashPrevBlock = TempHashHolding;
//            consensus.hashGenesisBlock = genesis.GetHash();
//
//            arith_uint256 BestBlockHashArith = UintToArith256(BestBlockHash);
//            if (UintToArith256(consensus.hashGenesisBlock) < BestBlockHashArith) {
//                BestBlockHash = consensus.hashGenesisBlock;
//                std::cout << BestBlockHash.GetHex() << " Nonce: " << i << "\n";
//                std::cout << "   PrevBlockHash: " << genesis.hashPrevBlock.GetHex() << "\n";
//            }
//
//            TempHashHolding = consensus.hashGenesisBlock;
//
//            if (BestBlockHashArith < test) {
//                genesisNonce = i - 1;
//                break;
//            }
//            //std::cout << consensus.hashGenesisBlock.GetHex() << "\n";
//        }
//        std::cout << "\n";
//        std::cout << "\n";
//        std::cout << "\n";
//
//        std::cout << "hashGenesisBlock to 0x" << BestBlockHash.GetHex() << std::endl;
//        std::cout << "Genesis Nonce to " << genesisNonce << std::endl;
//        std::cout << "Genesis Merkle " << genesis.hashMerkleRoot.GetHex() << std::endl;
//
//        std::cout << "\n";
//        std::cout << "\n";
//        int totalHits = 0;
//        double totalTime = 0.0;
//
//        for(int x = 0; x < 16; x++) {
//            totalHits += algoHashHits[x];
//            totalTime += algoHashTotal[x];
//            std::cout << "hash algo " << x << " hits " << algoHashHits[x] << " total " << algoHashTotal[x] << " avg " << algoHashTotal[x]/algoHashHits[x] << std::endl;
//        }
//
//        std::cout << "Totals: hash algo " <<  " hits " << totalHits << " total " << totalTime << " avg " << totalTime/totalHits << std::endl;
//
//        genesis.hashPrevBlock = TempHashHolding;
//
//        return;

//        /////////////////////////////////////////////////////////////////


        genesis = CreateGenesisBlock(1487000020, 0, 0x207fffff, 1, 1 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();

        assert(consensus.hashGenesisBlock == uint256S("0x487b10aae0cf4ce77082d200cb0dc0c7d90969b72af11a4bd0458d232b566c98"));
        assert(genesis.hashMerkleRoot == uint256S("0xccc9ed60efe7224e7ea404369d246390d1b8f09f33268dcb6d66f3c5707232ae"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fAllowMultiplePorts = false;
        nFulfilledRequestExpireTime = 5*60; // fulfilled requests expire in 5 minutes

        strSporkAddress = "bRLXw7eqUWMb9Uskk1pfvUozFgAkPZ2eMi";
        checkpointData = (CCheckpointData) {
            {
            }
        };

        chainTxData = ChainTxData{
            0,
            0,
            0
        };

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,45);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,48);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,50);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};


        /** BLAST Start **/
        // Burn Amounts
        nIssueAssetBurnAmount = 500 * COIN;
        nReissueAssetBurnAmount = 100 * COIN;
        nIssueSubAssetBurnAmount = 100 * COIN;
        nIssueUniqueAssetBurnAmount = 5 * COIN;

        // Burn Addresses
        strIssueAssetBurnAddress = "n1issueAssetXXXXXXXXXXXXXXXXWdnemQ";
        strReissueAssetBurnAddress = "n1ReissueAssetXXXXXXXXXXXXXXWG9NLd";
        strIssueSubAssetBurnAddress = "n1issueSubAssetXXXXXXXXXXXXXbNiH6v";
        strIssueUniqueAssetBurnAddress = "n1issueUniqueAssetXXXXXXXXXXS4695i";

        // Global Burn Address
        strGlobalBurnAddress = "n1BurnXXXXXXXXXXXXXXXXXXXXXXU1qejP";

        // DGW Activation
        nDGWActivationBlock = 0;

        nMaxReorganizationDepth = 60; // 60 at 32 second block timespan is +/- 32 minutes.
        nMinReorganizationPeers = 4;
        mnCollateralAmount = 10;
        nMinReorganizationAge = 60 * 60 * 12; // 12 hours
        /** BLAST End **/
    }
};
static CRegTestParams regTestParams;

static std::unique_ptr<CChainParams> globalChainParams;

const CChainParams &Params() {
    assert(globalChainParams);
    return *globalChainParams;
}

std::unique_ptr<CChainParams> CreateChainParams(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
        return std::unique_ptr<CChainParams>(new CMainParams());
    else if (chain == CBaseChainParams::TESTNET)
        return std::unique_ptr<CChainParams>(new CTestNetParams());
    else if (chain == CBaseChainParams::REGTEST)
        return std::unique_ptr<CChainParams>(new CRegTestParams());
    throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

CChainParams& Params(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
            return mainParams;
    else if (chain == CBaseChainParams::TESTNET)
            return testNetParams;
    else if (chain == CBaseChainParams::REGTEST)
            return regTestParams;
    else
        throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    globalChainParams = CreateChainParams(network);
}

void UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    globalChainParams->UpdateVersionBitsParameters(d, nStartTime, nTimeout);
}

void TurnOffSegwit(){
	globalChainParams->TurnOffSegwit();
}

void TurnOffCSV() {
	globalChainParams->TurnOffCSV();
}

void TurnOffBIP34() {
	globalChainParams->TurnOffBIP34();
}

void TurnOffBIP65() {
	globalChainParams->TurnOffBIP65();
}

void TurnOffBIP66() {
	globalChainParams->TurnOffBIP66();
}
