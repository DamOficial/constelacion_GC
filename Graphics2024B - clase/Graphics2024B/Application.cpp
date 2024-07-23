#include "Application.h"
#include "Canvas.h"
#include "Matrix4D.h"
#include <stdio.h>
#include <time.h>
Application* Application::sm_pApp;
Application* Application::GetApplication()
{
    if (!sm_pApp)
        sm_pApp = new Application();
    return sm_pApp;
}
Application::Application()
{
    //Initialize application data!!!
}
Application::~Application()
{
    //Uninitalize application data
}
void Application::Run()
{
    MSG msg;
    if (!Initialize())
    {
        MessageBox(NULL,
            TEXT("Error al inicializar"),
            TEXT("Application::Run()"),
            MB_ICONERROR);
        return;
    }
    ShowWindow(m_hWnd, m_nCmdShow);
    while (1)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                break;
        }
        Update();
    }
    Uninitialize();
}
bool Application::Initialize()
{
    m_pAdapter = m_DXGIManager.ChooseAdapter();
    if (m_pAdapter)
    {
        if (!m_DXGIManager.InitializeSwapChain(m_pAdapter))
            return false;
    }
    else
        return false;
    return true;
}
bool Application::Uninitialize()
{
    return true;
}

void ShaderCircle(Canvas::PIXEL* pDest, int i, int j,
    int unused_x, int unused_y)
{
    int x, y, r = 100 * 100;
    x = 400 - i;
    y = 400 - j;
    x *= x;
    y *= y;
    if (r > (x + y))
        *pDest = { 255,255,255,0 };
    else
        *pDest = { 255,0,0,0 };
}
void ShaderChess(Canvas::PIXEL* pDest,
    int i, int j, int x, int y)
{
    int p, q;
    Canvas::PIXEL Black = { 0,0,0,0 }, White = { 255,255,255,0 };
    p = i / 16;
    q = j / 16;
    *pDest = (p & 1) ^ (q & 1) ? Black : White;
}

void ShaderColors(Canvas::PIXEL* pDest,
    int i, int j, int x, int y)
{
    Canvas::PIXEL
        A = { 255,0,0,0 },
        B = { 0,255,0,0 },
        C = { 255,255,0,0 },
        D = { 0,0,255,0 };
    *pDest = Canvas::Lerp(
        Canvas::Lerp(A, B, x >> 8),
        Canvas::Lerp(C, D, x >> 8), y >> 8);
}
#include <math.h>

void VertexShaderSimple(MATRIX4D* pM, Canvas::VERTEX& Input, Canvas::VERTEX& Output)
{
    Output.P = Input.P * *pM;
}

//Diego Alejandro Mejía Ramírez
void Application::Update()
{
    auto pSwapChain = m_DXGIManager.GetSwapChain();
    auto pCanvas = m_DXGIManager.GetCanvas();
    static float hour = 8.0f;
    static float min = 20.0f;
    static float time = hour * 3600.0f + min * 60.0f;
    float theta = 2 * 3.141592 * time;
    pCanvas->Clear({ 0, 0, 0, 0 });

    Canvas::VERTEX CapricornioConstellation[] =
    {
        {{150, 180, 0, 1}, {255, 255, 255, 255}},  // Estrella 1
        {{170, 250, 0, 1}, {255, 0, 0, 255}},       // Estrella 2
        {{200, 200, 0, 1}, {0, 255, 0, 255}},       // Estrella 3
        {{250, 180, 0, 1}, {0, 0, 255, 255}},       // Estrella 4
        {{300, 160, 0, 1}, {255, 255, 0, 255}},    // Estrella 5
        {{350, 140, 0, 1}, {0, 255, 255, 255}},    // Estrella 6
        {{300, 120, 0, 1}, {255, 0, 255, 255}},    // Estrella 7
        {{250, 100, 0, 1}, {128, 128, 128, 255}},  // Estrella 8
        {{200, 80, 0, 1}, {255, 192, 203, 255}},  // Estrella 9
        {{150, 60, 0, 1}, {192, 192, 192, 255}}   // Estrella 10
    };

    MATRIX4D translateToOrigin = Translation(-225, -150, 0);
    MATRIX4D rotate = RotationZ(theta / 60);
    MATRIX4D translateBack = Translation(225, 150, 0);
    MATRIX4D transform = translateToOrigin * rotate * translateBack * Scaling(1.5f, 1.5f, 1);

    Canvas::VERTEX TransformedCapricornio[10];
    for (int i = 0; i < 10; i++)
    {
        TransformedCapricornio[i].P = CapricornioConstellation[i].P * transform;
        TransformedCapricornio[i].Color = CapricornioConstellation[i].Color;

        // Mensajes de depuración
        printf("Original Vertex %d: (%f, %f)\n", i, CapricornioConstellation[i].P.x, CapricornioConstellation[i].P.y);
        printf("Transformed Vertex %d: (%f, %f)\n", i, TransformedCapricornio[i].P.x, TransformedCapricornio[i].P.y);
        printf("Vertex Color %d: (%d, %d, %d, %d)\n", i, TransformedCapricornio[i].Color.r, TransformedCapricornio[i].Color.g, TransformedCapricornio[i].Color.b, TransformedCapricornio[i].Color.a);
    }

    // Dibujar los puntos
    pCanvas->DrawPointList(TransformedCapricornio, 10);

    // Dibujar las líneas conectando los puntos
    for (int i = 0; i < 9; i++)
    {
        pCanvas->Line(
            TransformedCapricornio[i].P.x, TransformedCapricornio[i].P.y,
            TransformedCapricornio[i + 1].P.x, TransformedCapricornio[i + 1].P.y,
            TransformedCapricornio[i].Color
        );
    }
    // Conectar el primer punto con el penúltimo
    pCanvas->Line(
        TransformedCapricornio[0].P.x, TransformedCapricornio[0].P.y,
        TransformedCapricornio[8].P.x, TransformedCapricornio[8].P.y,
        TransformedCapricornio[0].Color
    );

    m_DXGIManager.SendData(pCanvas->GetBuffer(), pCanvas->GetPitch());
    Canvas::DestroyCanvas(pCanvas);
    pSwapChain->Present(1, 0);
    time += 1.0f / 60;
}
