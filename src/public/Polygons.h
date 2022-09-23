#pragma once

#include "Core.h"

//----------------------------------------------------------------------------------------------------------------------
struct FPolyCount
{
    uint16 m_Count;
    uint16 m_TexPage;
};

// What is this?
struct FPolyIndices
{
   uint16  A;
   uint16  B;
   uint16  C;
   uint16  D;
};

// Represents an R8G8B8A8 color. Alpha is currently unused.
struct FColor
{
   uint8  Red;
   uint8  Green;
   uint8  Blue;
   uint8  Unused;
};

//----------------------------------------------------------------------------------------------------------------------
// Polygons
struct FTexTriangle
{
   FPolyIndices  m_Vertices;
   uint8         m_U0, m_V0;
   uint16        m_Flags;
   uint8         m_U1, m_V1;
   uint8         m_U2, m_V2;
};

struct FTexQuad
{
   FPolyIndices  m_Vertices;
   uint8         m_U0, m_V0;
   uint16        m_Flags;
   uint8         m_U1, m_V1;
   uint8         m_U2, m_V2;
   uint8         m_U3, m_V3;
   uint16        m_Unused;
};

struct FColoredTriangle
{
    FPolyIndices m_Vertices;
    FColor       m_Colors[3];
};

struct FColoredQuad
{
    FPolyIndices m_Vertices;
    FColor       m_Colors[4];
};
