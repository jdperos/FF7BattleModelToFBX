#pragma once

#include <cstdio>
#include <cstdlib>

#include "Core.h"

namespace FF7 {
namespace Reader {
namespace BattleModel {

uint32 ReadBattleModel( uint8*& out_Data, const FString& in_Filename )
{
    if( FILE* pFile = fopen( in_Filename.c_str(), "rb" ) )
    {
        fseek( pFile, 0, SEEK_END );
        uint32 FileSize = ftell( pFile );
        rewind( pFile );
        out_Data = static_cast<uint8*>(malloc( FileSize ));
        fread( out_Data, sizeof(uint8), FileSize, pFile );
        return FileSize;
    }
}

    
}
}
}
    