#include "FF7BattleModel.h"

#include <cassert>

#include "FF7BattleModelAnimation.h"

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
FF7::BattleModel::FModelHeader::FModelHeader( FILE* in_File )
{
    constexpr uint32 START_OFFSET = 0x00;
    fseek( in_File, START_OFFSET, 0 );
    Read( m_SectionCount,                   in_File );
    Read( m_OffsetToBoneDescriptionSection, in_File );
    Read( m_OffsetToModelSettings,          in_File );
    for( uint32 i = 0; i <m_SectionCount - 2; i++ )
    {
        uint32 Offset;
        Read( Offset, in_File );
        m_DataOffsets.push_back( Offset );
    }
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
void FF7::BattleModel::FBoneDescriptionSection::FBoneData::ReadFileFromCurrentLocation( FILE* in_File )
{
    Read( m_VertexPoolSize, in_File );
    const uint32 VertexCount = m_VertexPoolSize / sizeof( FVertex );
    for( uint32 i = 0; i < VertexCount; i++ )
    {
        FVertex Vertex;
        Read( Vertex, in_File );
        m_Vertices.push_back( Vertex );
    }

    ReadPolygonsIntoArray<FTexTriangle    >( in_File, m_TexuredTriangles );
    ReadPolygonsIntoArray<FTexQuad        >( in_File, m_TexuredQuads     );
    ReadPolygonsIntoArray<FColoredTriangle>( in_File, m_ColoredTriangles );
    ReadPolygonsIntoArray<FColoredQuad    >( in_File, m_ColoredQuads     );
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
FF7::BattleModel::FBoneDescriptionSection::FBoneDescriptionSection( FILE* in_File, const uint32 in_SectionOffset )
{
    fseek( in_File, in_SectionOffset, 0 );
    Read( m_BoneCount, in_File );
        
    // Add 1 because the first bone is always Root, which is not counted in m_BoneCount
    for( uint32 i = 0; i < m_BoneCount + 1; i++ )
    {
        FBoneDescription* BoneDescription = (FBoneDescription*)malloc( sizeof(FBoneDescription) );
        Read( BoneDescription->Parent, in_File );
        Read( BoneDescription->Length, in_File );
        Read( BoneDescription->Offset, in_File );
        m_BoneDescriptions.push_back( BoneDescription );
    }
        
    for( const FBoneDescription* BoneDescription : m_BoneDescriptions )
    {
        if( BoneDescription->IsJoint() ) continue;

        fseek( in_File, in_SectionOffset, SEEK_SET );
        fseek( in_File, BoneDescription->Offset, SEEK_CUR );
        FBoneData Data;
        Data.ReadFileFromCurrentLocation( in_File );
        m_BoneData.insert_or_assign( BoneDescription, Data );
    }
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
FF7::BattleModel::FAnimationData::FAnimationData( FModel* in_Owner, FILE* in_File, const uint32 in_DataOffset )
{
    m_OwningModel = in_Owner;
    fseek( in_File, in_DataOffset, SEEK_SET );
    Read( m_FrameCount, in_File );
    Read( m_AnimationSize, in_File );
    Read( m_DecodeKey, in_File );

    /**
     * http://www.memoryhacking.com/Docs/Battle%20Animation%20File%20Format.pdf
     * I mentioned a type of special animation data set that is in the header file. These data sets, when filled with
     * the “FF7FrameHeader” header, will have a “dwChunkSize” less than eleven, we skip them by jumping over the next 8
     * bytes that follow. 
     */
    if( m_AnimationSize > 10 )
    {
        ReadAnimation( in_File );
    }
}

//----------------------------------------------------------------------------------------------------------------------
uint32 FF7::BattleModel::FAnimationData::LoadFrames( FF7::BattleModel::Animation::FrameBuffer* out_FrameBuffer, uint32 in_BoneCount, uint8* in_AnimBuffer, uint32 in_BufferOffset )
{
    if( in_BufferOffset == 0 )
    {
        // First frame?
        // The first frame is uncompressed and each value is the actual rotation.
        out_FrameBuffer->PosOffset.sX = FF7::BattleModel::Animation::GetBitsFixed( in_AnimBuffer, in_BufferOffset, 16 ); //Always 16 bits
        out_FrameBuffer->PosOffset.sY = FF7::BattleModel::Animation::GetBitsFixed( in_AnimBuffer, in_BufferOffset, 16 );
        out_FrameBuffer->PosOffset.sZ = FF7::BattleModel::Animation::GetBitsFixed( in_AnimBuffer, in_BufferOffset, 16 );

        // This function will set the FLOAT values for the positions.
        // Any scaling that needs to be done would be done here.
        out_FrameBuffer->PosOffset.fX = static_cast<float>( out_FrameBuffer->PosOffset.sX );
        out_FrameBuffer->PosOffset.fY = static_cast<float>( out_FrameBuffer->PosOffset.sY );
        out_FrameBuffer->PosOffset.fZ = static_cast<float>( out_FrameBuffer->PosOffset.sZ );

        for( uint32 i = 0; i < in_BoneCount; i++ )
        {
            // Now get each bone rotation (the first bone is actually the root, not part of the skeleton).
            // During the first frame, the rotations are always (12 - bKeyBits).
            // We shift by bKeyBits to align it to 12 bits.
            out_FrameBuffer->Rotations[i].sX = ( FF7::BattleModel::Animation::GetBitsFixed( in_AnimBuffer, in_BufferOffset, 12 - m_DecodeKey ) << m_DecodeKey );
            out_FrameBuffer->Rotations[i].sY = ( FF7::BattleModel::Animation::GetBitsFixed( in_AnimBuffer, in_BufferOffset, 12 - m_DecodeKey ) << m_DecodeKey );
            out_FrameBuffer->Rotations[i].sZ = ( FF7::BattleModel::Animation::GetBitsFixed( in_AnimBuffer, in_BufferOffset, 12 - m_DecodeKey ) << m_DecodeKey );

            // Store the uint32 version as the absolute value of the uint16 version.
            out_FrameBuffer->Rotations[i].iX = ( out_FrameBuffer->Rotations[i].sX < 0 ) ? out_FrameBuffer->Rotations[i].sX + 0x1000 : out_FrameBuffer->Rotations[i].sX;
            out_FrameBuffer->Rotations[i].iY = ( out_FrameBuffer->Rotations[i].sY < 0 ) ? out_FrameBuffer->Rotations[i].sY + 0x1000 : out_FrameBuffer->Rotations[i].sY;
            out_FrameBuffer->Rotations[i].iZ = ( out_FrameBuffer->Rotations[i].sZ < 0 ) ? out_FrameBuffer->Rotations[i].sZ + 0x1000 : out_FrameBuffer->Rotations[i].sZ;
        }
    }
    else
    {
        // All other frames.
        uint16 sX, sY, sZ; // Get the positional offsets.
        sX = FF7::BattleModel::Animation::GetDynamicFrameOffsetBits( in_AnimBuffer, in_BufferOffset );
        sY = FF7::BattleModel::Animation::GetDynamicFrameOffsetBits( in_AnimBuffer, in_BufferOffset );
        sZ = FF7::BattleModel::Animation::GetDynamicFrameOffsetBits( in_AnimBuffer, in_BufferOffset );

        // When we come to this area of the function, pfbFrameBuffer will have the previous frame still stored in it.  Just add the offsets.
        out_FrameBuffer->PosOffset.sX += sX;
        out_FrameBuffer->PosOffset.sY += sY;
        out_FrameBuffer->PosOffset.sZ += sZ;

        out_FrameBuffer->PosOffset.fX = static_cast<float>( out_FrameBuffer->PosOffset.sX );
        out_FrameBuffer->PosOffset.fY = static_cast<float>( out_FrameBuffer->PosOffset.sY );
        out_FrameBuffer->PosOffset.fZ = static_cast<float>( out_FrameBuffer->PosOffset.sZ );
        for( uint32 i = 0; i < in_BoneCount; i++ )
        {
            // The same applies here.  Add the offsets and convert to uint32 form, adding 0x1000 if it is less than 0.
            // When Final FantasyÂ® VII loads these animations, it is possible for the value to sneak up above
            //	the 4095 boundary through a series of positive offsets. 
            sX = FF7::BattleModel::Animation::GetEncryptedRotationBits( in_AnimBuffer, in_BufferOffset, m_DecodeKey );
            sY = FF7::BattleModel::Animation::GetEncryptedRotationBits( in_AnimBuffer, in_BufferOffset, m_DecodeKey );
            sZ = FF7::BattleModel::Animation::GetEncryptedRotationBits( in_AnimBuffer, in_BufferOffset, m_DecodeKey );

            out_FrameBuffer->Rotations[i].sX += sX;
            out_FrameBuffer->Rotations[i].sY += sY;
            out_FrameBuffer->Rotations[i].sZ += sZ;

            out_FrameBuffer->Rotations[i].iX = ( out_FrameBuffer->Rotations[i].sX < 0 ) ? out_FrameBuffer->Rotations[i].sX + 0x1000 : out_FrameBuffer->Rotations[i].sX;
            out_FrameBuffer->Rotations[i].iY = ( out_FrameBuffer->Rotations[i].sY < 0 ) ? out_FrameBuffer->Rotations[i].sY + 0x1000 : out_FrameBuffer->Rotations[i].sY;
            out_FrameBuffer->Rotations[i].iZ = ( out_FrameBuffer->Rotations[i].sZ < 0 ) ? out_FrameBuffer->Rotations[i].sZ + 0x1000 : out_FrameBuffer->Rotations[i].sZ;
        }
    }

    // If we did not read as many bits as there are in the frame,
    //	return the location where the bits should start for the
    //	next frame.
    if( static_cast<uint16>( in_BufferOffset / 8 ) < m_AnimationSize )
    {
        return in_BufferOffset;
    }
    // Otherwise, return 0.
    return 0;
}

void FF7::BattleModel::FAnimationData::ReadAnimation( FILE* in_File )
{
    assert( m_OwningModel );
    // Load frames functions really want a big chunk of animation bytes, not a filestream
    uint8* AnimationBytes = (uint8*)malloc( m_AnimationSize );
    fread( AnimationBytes, sizeof(uint8), m_AnimationSize, in_File );

    // This will be our buffer to hold one frame. We will only buffer one frame at a time, so to
    //	to fully load the animations, you would need to write your own routine to store the data in
    //	fbFrameBuffer after each loaded frame.
    Animation::FrameBuffer TempBuffer;
    uint32 BoneCount = m_OwningModel->m_BoneDescriptionSection.m_BoneDescriptions.size();
    TempBuffer.SetBones( BoneCount );
    uint32 iBits = 0;
    for ( uint32 J = 0; J < m_FrameCount; J++ )
    {
        // We pass a pointer to fbFrameBuffer. The first frame will load diretly into it.
        // Every frame after that will actually use it with the	offsets loaded to determine the final result of that frame. 
		iBits = LoadFrames( &TempBuffer, BoneCount, AnimationBytes, iBits );
		// Reverse the Y offset (required).
		TempBuffer.PosOffset.fY = 0.0f - TempBuffer.PosOffset.fY;

        // The first rotation set is skipped.  It is not part of the skeleton.  Skipping is optional, but
        //	Final FantasyÂ® VII skips it; it is always 0, 0, 0.
        // I believe the actual use for the "root" rotation is to dynamically make the model point at its target
        //	or face different directions during battle.
        // UPDATE: Although the value of this field is 0,0,0 for most animations, some actually store a base rotation here
        // so it shouldn't be ignored.
		for ( uint32 i = 0; i < BoneCount; i++ )
		{
			TempBuffer.Rotations[i].fX = static_cast<float>( TempBuffer.Rotations[i].iX ) / 4096.0f * 360.0f;
			TempBuffer.Rotations[i].fY = static_cast<float>( TempBuffer.Rotations[i].iY ) / 4096.0f * 360.0f;
			TempBuffer.Rotations[i].fZ = static_cast<float>( TempBuffer.Rotations[i].iZ ) / 4096.0f * 360.0f;
		}
        // Store the data for this frame here (in your own routine).
        FFrameData FrameData;
        FrameData.m_BoneCount = BoneCount;
        FrameData.m_PositionOffset.X = TempBuffer.PosOffset.fX;
        FrameData.m_PositionOffset.Y = TempBuffer.PosOffset.fY;
        FrameData.m_PositionOffset.Z = TempBuffer.PosOffset.fZ;
        for( uint32 i = 0; i < BoneCount; i++ )
        {
            FVector Rotation;
            Rotation.X = TempBuffer.Rotations[i].fX;
            Rotation.Y = TempBuffer.Rotations[i].fY;
            Rotation.Z = TempBuffer.Rotations[i].fZ;
            FrameData.m_Rotations.push_back( Rotation );
        }
        m_AnimFrameBuffers.push_back( FrameData );

	}
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
FF7::BattleModel::FModel::FModel( const FString& in_Filename )
{
    if( FILE* pFile = fopen( in_Filename.c_str(), "rb" ) )
    {
        m_Header = FModelHeader( pFile );
        m_BoneDescriptionSection = FBoneDescriptionSection( pFile, m_Header.m_OffsetToBoneDescriptionSection );
        for( const uint32 DataOffset : m_Header.m_DataOffsets )
        {
            FAnimationData AnimData = FAnimationData( this, pFile, DataOffset) ;
            m_AnimationData.push_back( AnimData );
        }
    }
}
