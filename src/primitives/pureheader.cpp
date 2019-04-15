// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "primitives/pureheader.h"
#include "hash.h"
#include "utilstrencodings.h"

uint256 CPureBlockHeader::GetHash() const
{
	return SerializeHash(*this);
}

void CPureBlockHeader::SetNull()
{
	nVersion = 0;
	hashPrevBlock.SetNull();
	hashMerkleRoot.SetNull();
	nTime = 0;
	nBits = 0;
	nNonce = 0;
}

void CPureBlockHeader::SetBaseVersion(int32_t nBaseVersion, int32_t nChainId)
{
	nVersion = nBaseVersion | (nChainId * AuxPow::BLOCK_VERSION_CHAIN_START);
}
int32_t CPureBlockHeader::GetChainID() const
{
	return (nVersion & ~VERSIONBITS_TOP_MASK) / AuxPow::BLOCK_VERSION_CHAIN_START;
}
