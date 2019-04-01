#ifndef BITCOIN_BLOCKHEADER_H
#define BITCOIN_BLOCKHEADER_H

#include "auxpow/consensus.h"
#include "auxpow/serialize.h"
#include "versionbits.h"

/** Nodes collect new transactions into a block, hash them into a hash tree,
 * and scan through nonce values to make the block's hash satisfy proof-of-work
 * requirements.  When they solve the proof-of-work, they broadcast the block
 * to everyone and the block is added to the block chain.  The first transaction
 * in the block is a special one that creates a new coin owned by the creator
 * of the block.
 */
class CBlockHeader
{
public:
    // header
    int32_t nVersion;
    uint256 hashPrevBlock;
    uint256 hashMerkleRoot;
    uint32_t nTime;
    uint32_t nBits;
    uint32_t nNonce;
    std::shared_ptr<CAuxPow> auxpow;

    CBlockHeader()
    {
        SetNull();
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(this->nVersion);
        READWRITE(hashPrevBlock);
        READWRITE(hashMerkleRoot);
        READWRITE(nTime);
        READWRITE(nBits);
        READWRITE(nNonce);
        // the auxpow block is not serialized as part of the hash
        if ((!(s.GetType() & SER_GETHASH)) && this->IsAuxPow()) {
            READWRITE(auxpow);
        }
    }

    void SetNull()
    {
        nVersion = 0;
        hashPrevBlock.SetNull();
        hashMerkleRoot.SetNull();
        nTime = 0;
        nBits = 0;
        nNonce = 0;
    }

    bool IsNull() const
    {
        return (nBits == 0);
    }

    uint256 GetHash() const;

    uint256 GetPoWHash() const;

    int64_t GetBlockTime() const
    {
        return (int64_t)nTime;
    }

    /* Below are methods to interpret the version with respect to
       auxpow data and chain ID.  This used to be in the CBlockVersion
       class, but was moved here when we switched back to nVersion being
       a pure int member as preparation to undoing the "abuse" and
       allowing BIP9 to work.  */

    /**
     * Extract the base version (without modifiers and chain ID).
     * @return The base version./
     */
    inline int32_t GetBaseVersion() const
    {
        return GetBaseVersion(nVersion);
    }
    static inline int32_t GetBaseVersion(int32_t ver)
    {
        return ver % AuxPow::BLOCK_VERSION_AUXPOW;
    }

    /**
     * Set the base version (apart from chain ID and auxpow flag) to
     * the one given.  This should only be called when auxpow is not yet
     * set, to initialise a block!
     * @param nBaseVersion The base version.
     * @param nChainId The auxpow chain ID.
     */
    void SetBaseVersion(int32_t nBaseVersion, int32_t nChainId);

    /**
     * Set the chain ID.  This is used for the test suite.
     * @param ch The chain ID to set.
     */
    inline void SetChainId(int32_t chainId)
    {
        nVersion %= AuxPow::BLOCK_VERSION_CHAIN_START;
        nVersion |= chainId * AuxPow::BLOCK_VERSION_CHAIN_START;
    }

    /**
     * Check if the auxpow flag is set in the version.
     * @return True iff this block version is marked as auxpow.
     */

    inline int GetChainID() const
    {
        // return nVersion / AuxPow::BLOCK_VERSION_CHAIN_START;
        return (nVersion & ~VERSIONBITS_TOP_MASK) / AuxPow::BLOCK_VERSION_CHAIN_START;
    }

    inline bool IsAuxPow() const
    {
        return static_cast<bool>(nVersion & AuxPow::BLOCK_VERSION_AUXPOW);
    }

    /**
     * Check whether this is a "legacy" block without chain ID.
     * @return True iff it is.
     */
    inline bool IsLegacy() const
    {
        return nVersion == 1;
    }

    void SetAuxPow(CAuxPow*);
};


#endif //BITCOIN_BLOCKHEADER_H
