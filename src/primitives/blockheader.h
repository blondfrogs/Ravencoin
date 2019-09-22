#ifndef BITCOIN_BLOCKHEADER_H
#define BITCOIN_BLOCKHEADER_H

#include "primitives/pureheader.h"
#include "versionbits.h"
#include <iostream>

#include <boost/shared_ptr.hpp>

class CAuxPow;

/** Nodes collect new transactions into a block, hash them into a hash tree,
 * and scan through nonce values to make the block's hash satisfy proof-of-work
 * requirements.  When they solve the proof-of-work, they broadcast the block
 * to everyone and the block is added to the block chain.  The first transaction
 * in the block is a special one that creates a new coin owned by the creator
 * of the block.
 */
class CBlockHeader : public CPureBlockHeader
{
public:
    
    std::shared_ptr<CAuxPow> auxpow;

    CBlockHeader() {
        SetNull();
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
		READWRITE(*(CPureBlockHeader*)this);


		if (this->IsAuxPow()) {
			READWRITE(auxpow);
		}



    }

	void SetNull();

    void SetAuxPow(CAuxPow*);
};


#endif //BITCOIN_BLOCKHEADER_H
