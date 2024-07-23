#include "Canvas.h"

Canvas::Canvas()
{
	m_nSizeX = 0;
	m_nSizeY = 0;
	m_pBuffer = 0;
}
Canvas::~Canvas()
{
}
Canvas* Canvas::CreateCanvas(int nSizeX, int nSizeY)
{
	Canvas* pNewCanvas = new Canvas();
	pNewCanvas->m_pBuffer = new PIXEL[nSizeX * nSizeY];
	pNewCanvas->m_nSizeX = nSizeX;
	pNewCanvas->m_nSizeY = nSizeY;
	pNewCanvas->m_pLimits = new LIMIT[nSizeY];
	return pNewCanvas;
}
void Canvas::DestroyCanvas(Canvas* pDestroy)
{
	delete[] pDestroy->m_pBuffer;
	delete[] pDestroy->m_pLimits;
	delete pDestroy;
}
Canvas::PIXEL& Canvas::operator()(int i, int j)
{
	static PIXEL dummy;
	if (i >= 0 && i < m_nSizeX &&
		j >= 0 && j < m_nSizeY)
		return m_pBuffer[i + j * m_nSizeX];
	return dummy;
}
void Canvas::Clear(PIXEL color)
{
	PIXEL* pPixel = m_pBuffer;
	int c = m_nSizeX * m_nSizeY;
	while (c--)
		*pPixel++ = color;
}

Canvas::PIXEL Canvas::Lerp(PIXEL A, PIXEL B, short c)
{
	return {
		(unsigned char)(A.r + (((c * ((short)B.r - A.r)) >> 8))),
		(unsigned char)(A.g + (((c * ((short)B.g - A.g)) >> 8))),
		(unsigned char)(A.b + (((c * ((short)B.b - A.b)) >> 8))),
		(unsigned char)(A.a + (((c * ((short)B.a - A.a)) >> 8)))
	};
}

void Canvas::Shade(SHADER pShader)
{
	PIXEL* pDest = m_pBuffer;
	int dx, dy;
	dx = (1 << 16) / m_nSizeX;
	dy = (1 << 16) / m_nSizeY;
	int y = 0;
	for (int j = 0; j < m_nSizeY; j++)
	{
		int x = 0;
		for (int i = 0; i < m_nSizeX; i++)
		{
			pShader(pDest++, i, j, x, y);
			x += dx;
		}
		y += dy;
	}
}

void Canvas::Line(int x0, int y0, int x1, int y1,
	PIXEL Color)
{
	int dx, dy, p, x, y;
	x = x0;
	y = y0;
	dx = x1 - x0;
	dy = y1 - y0;
	int incx = dx >= 0 ? 1 : -1;
	int incy = dy >= 0 ? 1 : -1;
	dx = dx < 0 ? -dx : dx;
	dy = dy < 0 ? -dy : dy;
	
	if (dy <= dx)
	{
		p = dx - 2 * dy;
		int dp0 = -2 * dy;
		int dp1 = 2 * dx - 2 * dy;
		
		while (dx--)
		{
			(*this)(x, y) = Color;
			if (p > 0)
			{
				p += dp0;
			}
			else
			{
				p += dp1;
				y += incy;
			}
			x += incx;
		}
	}
	else // m > 1    x <---> y
	{
		p = dy - 2 * dx;
		int dp0 = -2 * dx;
		int dp1 = 2 * dy - 2 * dx;
		while (dy--)
		{
			(*this)(x, y) = Color;
			if (p > 0)
			{
				p += dp0;
			}
			else
			{
				p += dp1;
				x += incx;
			}
			y += incy;
		}
	}
}

void Canvas::ResetLimits()
{
	for (int j = 0; j < m_nSizeY; j++)
	{
		m_pLimits[j].left = m_nSizeX;
		m_pLimits[j].right = -1;
	}
}
#include <algorithm>
using namespace std;
void Canvas::SetLimit(int i, int j)
{
	if (j >= 0 && j < m_nSizeY)
	{
		auto lim = &m_pLimits[j];
		if (lim->left > i)
			lim->left = max(0, i);
		if (lim->right < i)
			lim->right = min(m_nSizeX, i);
	}
}
void Canvas::FillLimits(PIXEL Color)
{
	PIXEL* pRow = m_pBuffer;
	for (int j = 0; j < m_nSizeY; j++)
	{
		auto lim = &m_pLimits[j];
		if (lim->left < lim->right)
		{
			int c = lim->right - lim->left;
			PIXEL* pPixel = pRow + lim->left;
			while (c--) *pPixel++ = Color;
		}
		pRow += m_nSizeX;
	}
}
void Canvas::FillLimits(SHADER pShader)
{
	int x, y;
	PIXEL* pRow = m_pBuffer;
	int dx = (1 << 16) / m_nSizeX;
	int dy = (1 << 16) / m_nSizeY;
	y = 0;
	for (int j = 0; j < m_nSizeY; j++)
	{
		auto lim = &m_pLimits[j];
		if (lim->left < lim->right)
		{
			int i = lim->left;
			x = i * dx;
			int c = lim->right - lim->left;
			PIXEL* pPixel = pRow + lim->left;
			while (c--)
			{
				pShader(pPixel++, i++, j, x, y);
				x += dx;
			}
		}
		pRow += m_nSizeX;
		y += dy;
	}
}

void Canvas::LineLimits(int x0, int y0, int x1, int y1)
{
	int dx, dy, p, x, y;
	x = x0;
	y = y0;
	dx = x1 - x0;
	dy = y1 - y0;
	int incx = dx >= 0 ? 1 : -1;
	int incy = dy >= 0 ? 1 : -1;
	dx = dx < 0 ? -dx : dx;
	dy = dy < 0 ? -dy : dy;

	if (dy <= dx)
	{
		p = dx - 2 * dy;
		int dp0 = -2 * dy;
		int dp1 = 2 * dx - 2 * dy;
		
		while (dx--)
		{
			this->SetLimit(x, y);
			if (p > 0)
			{
				p += dp0;
			}
			else
			{
				p += dp1;
				y += incy;
			}
			x += incx;
		}
	}
	else 
	{
		p = dy - 2 * dx;
		int dp0 = -2 * dx;
		int dp1 = 2 * dy - 2 * dx;
		while (dy--)
		{
			this->SetLimit(x, y);
			if (p > 0)
			{
				p += dp0;
			}
			else
			{
				p += dp1;
				x += incx;
			}
			y += incy;
		}
	}
}

void Canvas::Circle(int xc, int yc, int r, PIXEL Color)
{
	int x = 0;
	int y = r;
	int p = 5 - 4 * r;
	int _8x, _8y;
	_8x = 0;
	_8y = 8 * y;
	while (x < y)
	{
		(*this)(x + xc, y + yc) = Color;
		(*this)(-x + xc, y + yc) = Color;
		(*this)(x + xc, -y + yc) = Color;
		(*this)(-x + xc, -y + yc) = Color;
		(*this)(y + xc, x + yc) = Color;
		(*this)(-y + xc, x + yc) = Color;
		(*this)(y + xc, -x + yc) = Color;
		(*this)(-y + xc, -x + yc) = Color;
		if (p > 0)
		{
			p += _8x - _8y + 20;
			y--;
			_8y -= 8;
		}
		else
			p += _8x + 12;
		_8x += 8;
		x++;
	}
}

void Canvas::CircleLimits(int xc, int yc, int r)
{
	int x = 0;
	int y = r;
	int p = 5 - 4 * r;
	int _8x, _8y;
	_8x = 0;
	_8y = 8 * y;
	while (x < y)
	{
		SetLimit(x + xc, y + yc);
		SetLimit(-x + xc, y + yc);
		SetLimit(x + xc, -y + yc);
		SetLimit(-x + xc, -y + yc);
		SetLimit(y + xc, x + yc);
		SetLimit(-y + xc, x + yc);
		SetLimit(y + xc, -x + yc);
		SetLimit(-y + xc, -x + yc);
		if (p > 0)
		{
			p += _8x - _8y + 20;
			y--;
			_8y -= 8;
		}
		else
			p += _8x + 12;
		_8x += 8;
		x++;
	}
}


void Canvas::VertexProcessor(MATRIX4D* ctx, VERTEXSHADER pVS, VERTEX* pInput, VERTEX* pOutput, int nVertices)
{
	for (int i = 0; i < nVertices; i++)
	{
		pVS(ctx, *pInput++, *pOutput++);
	}
}

void Canvas::DrawPointList(VERTEX* pVertex, int nVertices)
{
	for (int i = 0; i < nVertices; i++, pVertex++)
	{
		operator()(pVertex->P.x, pVertex->P.y) = pVertex->Color;
	}
}

void Canvas::DrawVertexList(VERTEX* pVertex, int nVertices, PIXEL color)
{
	for (int i = 0; i < nVertices; i += 5)
	{
		if (i + 5 <= nVertices)
		{
			VERTEX* pT = pVertex + i;
			Line(pT[0].P.x, pT[0].P.y, pT[1].P.x, pT[1].P.y, color);
			Line(pT[1].P.x, pT[1].P.y, pT[2].P.x, pT[2].P.y, color);
			Line(pT[2].P.x, pT[2].P.y, pT[3].P.x, pT[3].P.y, color);
			Line(pT[3].P.x, pT[3].P.y, pT[4].P.x, pT[4].P.y, color);
			Line(pT[4].P.x, pT[4].P.y, pT[0].P.x, pT[0].P.y, color);
		}
	}
}


void Canvas::DrawLinesList(VERTEX* pVertex, int nVertices)
{
	for (int i = 0; i < nVertices; i += 2)
	{
		Line(pVertex[0].P.x, pVertex[0].P.y, pVertex[1].P.x, pVertex[1].P.y, pVertex->Color);
	}
}

void Canvas::DrawLinesStrip(VERTEX* pVertex, int nVertices)
{
	for (int i = 0; i < nVertices - 1; i++)
	{
		VERTEX* pV = pVertex;
		pVertex++;
		Line(pV->P.x, pV->P.y, pVertex->P.x, pVertex->P.y, pV->Color);
	}
}

void Canvas::DrawVertexStrip(VERTEX* pVertex, int nVertices)
{
	if (nVertices < 3) return;

	for (int i = 0; i < nVertices - 2; i++)
	{
		Line(pVertex[i].P.x, pVertex[i].P.y, pVertex[i + 1].P.x, pVertex[i + 1].P.y, pVertex[i].Color);
		Line(pVertex[i + 1].P.x, pVertex[i + 1].P.y, pVertex[i + 2].P.x, pVertex[i + 2].P.y, pVertex[i + 1].Color);
		Line(pVertex[i + 2].P.x, pVertex[i + 2].P.y, pVertex[i].P.x, pVertex[i].P.y, pVertex[i + 2].Color);
	}
}
