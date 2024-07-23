#pragma once

#include "Matrix4D.h"

class Canvas
{
public:
    // Data types
    union PIXEL
    {
        struct
        {
            unsigned char r, g, b, a;
        };
        struct
        {
            unsigned char c, m, y, b;
        };
        long p;
    };

    struct VERTEX
    {
        VECTOR4D P;
        PIXEL Color;
    };

    // Methods
    static Canvas* CreateCanvas(int nSizeX, int nSizeY);
    static void DestroyCanvas(Canvas* pDestroy);

    PIXEL& operator()(int i, int j);
    void Clear(PIXEL color);

    void* GetBuffer() { return m_pBuffer; }
    unsigned int GetPitch() { return sizeof(PIXEL) * m_nSizeX; }
    int GetSizeX() { return m_nSizeX; }
    int GetSizeY() { return m_nSizeY; }

    static PIXEL Lerp(PIXEL A, PIXEL B, short c);

    typedef void (*SHADER)(PIXEL* pDest, int i, int j, int x, int y);
    void Shade(SHADER pShader);

    void Line(int x0, int y0, int x1, int y1, PIXEL Color);
    void ResetLimits();
    void SetLimit(int i, int j);
    void FillLimits(PIXEL Color);
    void FillLimits(SHADER pShader);
    void LineLimits(int x0, int y0, int x1, int y1);
    void Circle(int xc, int yc, int r, PIXEL Color);
    void CircleLimits(int xc, int yx, int r);

    typedef void (*VERTEXSHADER)(MATRIX4D*, Canvas::VERTEX&, Canvas::VERTEX&);
    static void VertexProcessor(MATRIX4D* ctx, VERTEXSHADER pVS, VERTEX* pInput, VERTEX* pOutput, int nVertices);

    void DrawVertexList(VERTEX* pVertex, int nVertices, PIXEL color);
    void DrawPointList(VERTEX* pVertex, int nVertices);
    void DrawLinesList(VERTEX* pVertex, int nVertices);
    void DrawLinesStrip(VERTEX* pVertex, int nVertices);
    void DrawVertexStrip(VERTEX* pVertex, int nVertices);

protected:
    // Internal data
    struct LIMIT
    {
        int left;
        int right;
    };

    int m_nSizeX; // Tamaño horizontal del canvas en píxeles
    int m_nSizeY; // Tamaño vertical del canvas en píxeles
    PIXEL* m_pBuffer; // Buffer de píxeles
    LIMIT* m_pLimits; // Buffer de límites de relleno

    // Constructors and destructors
    Canvas();
    ~Canvas();
};
