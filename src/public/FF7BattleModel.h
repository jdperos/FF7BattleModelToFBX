#pragma once

#include "Core.h"

#include "FF7BattleModelAnimation.h"
#include "Polygons.h"

#define Read(Variable, File) fread( &Variable, sizeof(Variable), 1, File )

namespace FF7 {
namespace BattleModel {

struct FModelHeader
{
    uint32         m_SectionCount;
    uint32         m_OffsetToBoneDescriptionSection;
    uint32         m_OffsetToModelSettings;
    // TODO(jperos): Need to determine what of these are animations and what are not.
    TArray<uint32> m_DataOffsets;

    FModelHeader() {}
    FModelHeader( FILE* in_File );
};

// This is the first section and consists of real model data and bone structure. Without the animation information the model CANNOT be displayed properly.
struct FBoneDescriptionSection
{
    uint32 m_BoneCount;

    // If Offset is 0 then the object is a Joint otherwise it's a bone. All FF7 bone lengths are negative.
    struct FBoneDescription
    {
        uint16 Parent;
        int16  Length;
        // Offset of Bone Data Object from the Bone Description section
        uint32 Offset;

        bool IsJoint() const { return Offset == 0; }
    };
    TArray<FBoneDescription*> m_BoneDescriptions;

    struct FBoneData
    {
        uint32 m_VertexPoolSize;
        
        struct FVertex
        {
            int16 X;
            int16 Y;
            int16 Z;
            int16 Unused;
        };
        TArray<FVertex> m_Vertices;
        
        TArray<FTexTriangle> m_TexuredTriangles;
        TArray<FTexQuad> m_TexuredQuads;
        TArray<FColoredTriangle> m_ColoredTriangles;
        TArray<FColoredQuad> m_ColoredQuads;

        FBoneData(){}

        
        template<typename T>
        void ReadPolygonsIntoArray( FILE* in_File, TArray<T>& in_Array )
        {
            FPolyCount PolyCount;
            Read( PolyCount, in_File );
            for( uint32 i = 0; i < PolyCount.m_Count; i++ )
            {
                T Poly;
                Read( Poly, in_File );
                in_Array.push_back( Poly );
            }
        }

        void ReadFileFromCurrentLocation( FILE* in_File );
    };
    TMap<const FBoneDescription*, FBoneData> m_BoneData;

    FBoneDescriptionSection(){}
    FBoneDescriptionSection( FILE* in_File, const uint32 in_SectionOffset );
};

struct FFrameData
{
    uint32          m_BoneCount;
    FVector         m_PositionOffset;
    TArray<FVector> m_Rotations;
    
};

struct FAnimationData
{
    struct FModel*     m_OwningModel;
    uint16             m_FrameCount;
    uint16             m_AnimationSize;
    uint8              m_DecodeKey;
    TArray<FFrameData> m_AnimFrameBuffers;
    
    FAnimationData(){}
    FAnimationData( struct FModel* in_Owner, FILE* in_File, const uint32 in_DataOffset );

    uint32 LoadFrames( Animation::FrameBuffer* out_FrameBuffer, uint32 in_BoneCount, uint8* in_AnimBuffer, uint32 in_Offset );
    void ReadAnimation( FILE* in_File );
};

struct FModel
{
    FModelHeader            m_Header;
    FBoneDescriptionSection m_BoneDescriptionSection;
    TArray<FAnimationData>  m_AnimationData;

    FModel() {}
    FModel( const FString& in_Filename );
};

}
}