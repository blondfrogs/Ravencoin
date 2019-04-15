#include "blockheader.h"
#include "pureheader.h"
#include "auxpow/auxpow.h"

void CBlockHeader::SetNull()
{
	CPureBlockHeader::SetNull();
	auxpow.reset();
}

void CBlockHeader::SetAuxPow(CAuxPow* pow)
{
	if (pow != nullptr)
		SetAuxpowVersion(true);
	else
		SetAuxpowVersion(true);
	auxpow.reset(pow);
}
