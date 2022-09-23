#pragma once

#include "Core.h"

namespace FF7 {
namespace BattleModel {
namespace Animation {

/*
	Notes for rotational delta compression scheme
	=============================================

	The delta values are stored in compressed form in a bit stream. Consecutive
	values share no bit correlation or encoding dependencies, rather they are
	encoded separately using a scheme designed to optimize small-scale rotations.

	Rotations are traditionally given in normalized PSX 4.12 fixed-point compatible
	values, where a full rotation is the integer value 4096. Values above 4095 simply
	map down into the [0,4095] range, as expected from rotational arithmetics. When
	encoded, only the required 12 bits of precision are ever considered.

	In some animations, the author can choose to forcibly lower the precision of
	rotational delta values below 12 bits. Though these animations are naturally not
	as precise, they encode far more efficiently, both because of the smaller size of
	'raw' values, and also because of the increased relative span of the 'close-range'
	encodings available for small deltas. The smallest 128 [-64,63] sizes of deltas
	can be stored in compressed form instead of as raw values. This method is
	efficient
	since a majority of the rotational deltas involved in skeletal character animation
	will be small, and thus doubly effective if used with reduced precision. Precision
	can be reduced by either 2 or 4 bits (down to 10 or 8 bits).

	The encoding scheme is capable of encoding any 12-bit value as follows:

	First, a single bit tells us if the delta is non-zero. If this bit is zero, there
	is no delta value (0) and the decoding is done. Otherwise, a 3 bit integer
	follows, detailing how the delta value is encoded. This has 8 different meanings,
	as follows:

	Type   Meaning
	------ -------------------------------------------------------
	0      The delta is the smallest possible decrement (under the
		current precision)
	1-6    The delta is encoded using this many uint8s
	7      The delta is stored in raw form (in the current precision)

	The encoding of small deltas works as follows: The encoded delta can be stored
	using 1-6 bits, giving us a total of 2+4+8+16+32+64 = 126 possible different
	values, which during this explanation will be explained as simple integers (the
	lowest bits of the delta, in current precision). The values 0 (no change) and -1
	(minimal decrement) are already covered, leaving the other 126 values to neatly
	fill out the entire 7 bit range. We do this by encoding each value like follows:

	- The magnitude of the delta is defined as the value of its most significant
	  value bit (in two's complement, so the highest bit not equal to the sign bit).
	  For example, the values '1' and '-2' have magnitude 1, while the value '30' will
	  have a magnitude of 32. For simplicity, we also define the 'signed magnitude' as
	  the magnitude multiplied by the sign of the value (so '-2' has a signed
 	  magnitude of -1).
	- When encoding a value, we subtract its signed magnitude; essentially pushing
	  everything down one notch towards zero, setting the most significant value bit
	  to equal the sign bit and thus ensuring that none of the transformed values
	  require more than six bits to accurately represent in two's complement.
	- The transformed value is then stored starting from its magnitude bit (normally,
	  you would have to start one bit higher to include the sign bit and prevent
 	  signed integer overflow). Small values will be stored using fewer bits, while
 	  larger values use more bits. The two smallest values, 0 and -1, are not
 	  encodeable but are instead handled using the previously mentioned scheme.

	When decoding, you only need to know the number of bits of the encoded value, use
	the value of its most significant bit (not the most significant value bit!) as
	the magnitude, multiply it by the sign of the encoded value to get the signed
	magnitude, and then add that to the encoded value to get the actual delta value.

	Some examples of encodings:

	Delta value *      Encoded            *) in current precision, as integer
	------------------ ------------------
	 0                 0
	-1                 1 000
	-5                 1 011 111
	15                 1 100 0111
	128                1 111 xxxx10000000  (length depends on precision)


	(Note: The reduced precision is treated as rounding towards negative infinity)

*/

struct ShortVec
{
    int16  sX, sY, sZ; // Signed short versions.	0x00
    uint32 iX, iY, iZ; // Integer representation.	0x06
    float  fX, fY, fZ; // Float version after math.	0x12
};                     // Size = 30 bytes.

struct FrameBuffer
{
	uint32          dwBones;
	ShortVec        PosOffset;
	ShortVec*       Rotations;

	FrameBuffer();
    ~FrameBuffer();

    void SetBones( uint32 dwTotal );
};

int32 GetBitsFixed( uint8* pbBuffer, uint32& dwStartBit, uint32 dwTotalBits );
uint16 GetValueFromStream( uint8* pStreamuint8s, uint32 *pdwStreamBitOffset );
uint16 GetDynamicFrameOffsetBits( uint8* pBuffer, uint32& dwBitStart );
uint16 GetEncryptedRotationBits( uint8* pBuffer, uint32& dwBitStart, uint32 iKeyBits );

}
}
}




