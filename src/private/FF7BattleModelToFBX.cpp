#include "Core.h"

#include "FF7BattleModel.h"
#include "FF7BMReader.h"

int main()
{
    uint8* Buffer;
    uint32 FileSize = FF7::Reader::BattleModel::ReadBattleModel( Buffer, "D:\\FF7\\CLOUD.BIN" );
    FF7::BattleModel::FModel Model( "D:\\FF7\\CLOUD.BIN" );
    
    return 0;
}
