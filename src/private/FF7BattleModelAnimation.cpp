#include "FF7BattleModelAnimation.h"

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
FF7::BattleModel::Animation::FrameBuffer::FrameBuffer()
{
    dwBones = 0;
    Rotations = nullptr;
}

//----------------------------------------------------------------------------------------------------------------------
FF7::BattleModel::Animation::FrameBuffer::~FrameBuffer()
{
    dwBones = 0;
    delete [] Rotations;
    Rotations = nullptr;
}

//----------------------------------------------------------------------------------------------------------------------
void FF7::BattleModel::Animation::FrameBuffer::SetBones( uint32 dwTotal )
{
    // Delete the old.
    dwBones = 0;
    // delete [] Rotations;
		
    // Create the new.
    Rotations = new ShortVec[dwTotal];
    if ( Rotations )
    {
        dwBones = dwTotal;
    }
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
int32 FF7::BattleModel::Animation::GetBitsFixed( uint8* pbBuffer, uint32& dwStartBit, uint32 dwTotalBits )
{
    int32 iReturn = 0;

    for ( uint32 i = 0; i < dwTotalBits; i++ )
    {
        iReturn <<= 1;

        __asm mov eax, dwStartBit
        __asm mov eax, [eax]
        __asm cdq
        __asm and edx, 7
        __asm add eax, edx
        __asm sar eax, 3
        __asm mov ecx, pbBuffer
        __asm xor edx, edx
        __asm mov dl, byte ptr ds:[ecx+eax]
        __asm mov eax, dwStartBit
        __asm mov ecx, [eax]
        __asm and ecx, 7
        __asm mov eax, 7
        __asm sub eax, ecx
        __asm mov esi, 1
        __asm mov ecx, eax
        __asm shl esi, cl
        __asm and edx, esi
        __asm test edx, edx
        __asm je INCBIT
        iReturn++;
    INCBIT:
        dwStartBit++;
    }

    // Force the sign bit to extend across the 32-bit boundary.
    iReturn <<= (0x20 - dwTotalBits);
    iReturn >>= (0x20 - dwTotalBits);
    return iReturn;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
uint16 FF7::BattleModel::Animation::GetValueFromStream( uint8* pStreamuint8s, uint32* pdwStreamBitOffset )
{
    // The return value;
    uint16 out_Value;
    // The number of whole uint8s already consumed in the stream.
    uint32 dwStreamuint8Offset = *pdwStreamBitOffset / 8;
    // The number of bits already consumed in the current stream uint8.
    uint32 dwCurrentBitsEaten = *pdwStreamBitOffset % 8;
    // The distance from dwNextStreamuint8s' LSB to the 'type' bit.
    uint32 dwTypeBitShift = 7 - dwCurrentBitsEaten;
    // A copy of the next two uint8s in the stream (from big-endian).
    uint32 dwNextStreamuint8s = pStreamuint8s[dwStreamuint8Offset] << 8 | pStreamuint8s[dwStreamuint8Offset + 1];

    // Test the first bit (the 'type' bit) to determine the size of the value.
    if (dwNextStreamuint8s & (1 << (dwTypeBitShift + 8)))
    { // Sixteen-bit value:
        // Collect one more uint8 from the stream.
        dwNextStreamuint8s = dwNextStreamuint8s << 8 |
            pStreamuint8s[dwStreamuint8Offset + 2];
        // Shift the delta value into place.
        out_Value = (dwNextStreamuint8s << (dwCurrentBitsEaten + 1)) >> 8;
        // Update the stream offset.
        *pdwStreamBitOffset += 17;
    }
    else
    { // Seven-bit value
        // Shift the delta value into place (taking care to preserve the sign).
        out_Value = ((uint16)(dwNextStreamuint8s << (dwCurrentBitsEaten + 1))) >> 9;
        // Update the stream offset.
        *pdwStreamBitOffset += 8;
    }

    // Return the value.
    return out_Value;
}

uint16 FF7::BattleModel::Animation::GetDynamicFrameOffsetBits( uint8* pBuffer, uint32& dwBitStart )
{
    uint32 dwFirstuint8, dwConsumedBits, dwBitsRemainingToNextuint8, dwTemp;
    uint16 sReturn;
    __asm {
        mov eax, dwBitStart
        mov eax, [eax]
        cdq
        and edx, 7
        add eax, edx
        sar eax, 3
        mov dwFirstuint8, eax
        mov ecx, dwBitStart
        mov edx, [ecx]
        and edx, 7
        mov dwConsumedBits, edx
        mov eax, 7
        sub eax, dwConsumedBits
        mov dwBitsRemainingToNextuint8, eax
        mov ecx, pBuffer
        add ecx, dwFirstuint8 // Go to the first uint8 that
        // has the bit where we
        // want to begin.
        xor edx, edx
        mov dl, byte ptr ds:[ecx]
        shl edx, 8
        mov eax, pBuffer
        add eax, dwFirstuint8
        xor ecx, ecx
        mov cl, byte ptr ds:[eax+1]
        or edx, ecx
        mov dwTemp, edx
        mov ecx, dwBitsRemainingToNextuint8
        add ecx, 8
        mov edx, 1
        shl edx, cl
        mov eax, dwTemp
        and eax, edx
        test eax, eax
        jnz SeventeenBits

        EightBits :
        mov ecx, dwConsumedBits
        add ecx, 1
        mov edx, dwTemp
        shl edx, cl
        movsx eax, dx
        sar eax, 9
        mov sReturn, ax
        mov ecx, dwBitStart //
        mov edx, [ecx]      //
        add edx, 8          //
        mov eax, dwBitStart //
        mov [eax], edx      // Increase dwBitStart by 0x8 (8).
        jmp End

        SeventeenBits :
        mov ecx, dwTemp
        shl ecx, 8
        mov edx, pBuffer
        add edx, dwFirstuint8
        xor eax, eax
        mov al, byte ptr ds:[edx+2]
        or ecx, eax
        mov dwTemp, ecx
        mov ecx, dwConsumedBits
        add ecx, 1
        mov edx, dwTemp
        shl edx, cl
        shr edx, 8
        mov sReturn, dx
        mov eax, dwBitStart //
        mov ecx, [eax]      //
        add ecx, 0x11       //
        mov edx, dwBitStart //
        mov [edx], ecx      // Increase dwBitStart by 0x11
        //	(17).

        End :
        }
    return sReturn;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
uint16 FF7::BattleModel::Animation::GetEncryptedRotationBits( uint8* pBuffer, uint32& dwBitStart, uint32 iKeyBits )
{
    uint32 dwNumBits, dwType;
    uint32 iTemp;
    uint16 sReturn;
    // Check the first bit.
    uint32 iBits = GetBitsFixed( pBuffer, dwBitStart, 1 );
    __asm mov eax, iBits // If the first bit is 0, return 0
    // and continue. It is not necessary
    // to mov iBits into EAX, but I do it
    // anyway.
    __asm test eax, eax
    __asm jnz SecondTest
    __asm jmp ReturnZero // Return 0

SecondTest :
    // Otherwise continue by getting the next 3 bits.
    iBits = GetBitsFixed( pBuffer, dwBitStart, 3 );
    __asm mov eax, iBits
    __asm and eax, 7
    __asm mov dwNumBits, eax
    __asm mov ecx, dwNumBits
    __asm mov dwType, ecx // dwType = ecx = dwNumBits = eax =
    //	(iBits & 7).
    //	When we get to the case, all of
    //	these values are the same.
    __asm cmp dwType, 7
    __asm ja ReturnZero // Is dwType above 7?  If so, return 0.
    //	This can never actually happen.

    // Otherwise, use it in a switch case.
    switch( dwType )
    {
    case 0:
        {
            __asm or eax, 0xFFFFFFFF // After this, EAX will
            // always be -1.
            __asm mov ecx, iKeyBits
            __asm shl eax, cl // Shift left by
            // precision.
            // (-1 << iKeyBits)
            __asm mov sReturn, ax // Return that number.
            __asm jmp End
        }
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
        {
            // Get a number of bits equal to the case switch (1,
            // 2, 3, 4, 5, or 6).
            iTemp = GetBitsFixed( pBuffer, dwBitStart,
                                  dwNumBits );
            __asm mov eax, iTemp
            __asm cmp iTemp, 0
            __asm jl IfLessThanZero
            // If greater than or equal to 0â€¦
            __asm mov ecx, dwNumBits // dwNumBits = (iBits &
            // 7) from before.
            __asm sub ecx, 1 // dwNumBits - 1.
            __asm mov eax, 1
            __asm shl eax, cl // (1 << (dwNumBits â€“
            // 1)).
            __asm mov ecx, iTemp
            __asm add ecx, eax // iTemp += (1 <<
            // (dwNumBits - 1)).
            __asm mov iTemp, ecx
            __asm jmp AfterTests
            // If less than 0â€¦
        IfLessThanZero :
            __asm mov ecx, dwNumBits // dwNumBits = (iBits &
            // 7) from before.
            __asm sub ecx, 1 // Decrease it by 1.
            __asm mov edx, 1
            __asm shl edx, cl // Shift â€œ1â€ left by
            // (dwNumBits - 1).
            __asm mov eax, iTemp // iTemp still has the
            // bits we read
            // from before.
            __asm sub eax, edx // iTemp - (1 <<
            // (dwNumBits - 1))
            __asm mov iTemp, eax

            // Now, whatever we set on iTemp, we need to shift it
            // up by the precision value.
        AfterTests :
            __asm mov eax, iTemp
            __asm mov ecx, iKeyBits
            __asm shl eax, cl // iTemp <<= iKeyBits
            __asm mov sReturn, ax
            __asm jmp End
        }

    case 7:
        {
            // Uncompressed bits.  Use standard decoding.
            iTemp = GetBitsFixed( pBuffer, dwBitStart,
                                  12 - iKeyBits );
            __asm mov ecx, iKeyBits
            __asm shl eax, cl // iTemp <<= iKeyBits.
            __asm mov sReturn, ax
            __asm jmp End
        }
    }


ReturnZero :
    __asm xor ax, ax
    __asm mov sReturn, ax

End :

    return sReturn;
}
