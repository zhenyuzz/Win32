#include <windows.h>

HINSTANCE hInst;
HBRUSH hSquareBrush;
HPEN hSquareBorderPen;
HPEN hSceneBorderPen;
/* HWND hWnd; */
HDC hDC;
HDC hPaintDC;
HBITMAP hBitmap;
LPCSTR pTitle       = "Tetris";
LPCSTR pWindowClass = "Tetris";
static const COLORREF backgroundColor = RGB(  0,   0,   0);
static const COLORREF borderColor     = RGB(255, 255, 255);
static const COLORREF squareColor     = RGB(255,   0,   0);
static const int width            = 640;
static const int height           = 640;
static const int sceneBorder      = 2;
static const int squareBorder     = 1;
static const int squareSideLength = 20;
static const int topPadding       = 40;
static const int leftPadding      = 40;
static const int rowNumber        = 25;
static const int colNumber        = 20;

void InitDraw(HDC hDC)
{
    hPaintDC = CreateCompatibleDC(hDC);
    hBitmap = CreateCompatibleBitmap(hDC, width, height);
    SelectObject(hPaintDC, hBitmap);
    hSceneBorderPen = CreatePen(PS_SOLID, sceneBorder, borderColor);
    hSquareBorderPen = CreatePen(PS_SOLID, squareBorder, borderColor);
    hSquareBrush = CreateSolidBrush(squareColor);
}

void DrawScene()
{
    const int sceneWidth = colNumber * squareSideLength;
    const int sceneHeight = rowNumber * squareSideLength;
    POINT points[5];
    points[0].x = leftPadding-sceneBorder;
    points[0].y = topPadding-sceneBorder;
    points[1].x = leftPadding+sceneWidth;
    points[1].y = points[0].y;
    points[2].x = points[1].x;
    points[2].y = topPadding+sceneHeight;
    points[3].x = points[0].x;
    points[3].y = points[2].y;
    points[4] = points[0];
    SelectObject(hPaintDC, hSceneBorderPen);
    Polyline(hPaintDC, points, 5);
}

void OnDraw(HDC hDC)
{
    static BOOL initialed = FALSE;
    if (!initialed) {
        InitDraw(hDC);
        initialed = TRUE;
    }

    DrawScene();

    BitBlt(hDC, 0, 0, width, height, hPaintDC, 0, 0, SRCCOPY);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        OnDraw(hdc);
        EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = NULL;
    wcex.hbrBackground = CreateSolidBrush(backgroundColor);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = pWindowClass;
    wcex.hIconSm = NULL;

    return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;
    hInst = hInstance;
    hWnd = CreateWindow(pWindowClass, pTitle, WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL,
                        hInstance, NULL);
    if (NULL == hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdline,
                     int nCmdShow)
{
    MSG msg;

    MyRegisterClass(hInstance);
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
