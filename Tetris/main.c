#include <windows.h>
#include <stdlib.h>

HINSTANCE hInst;
HWND hWnd;
HBRUSH hBackgroundBrush;
HBRUSH hSquareBrush;
HPEN hSquareBorderPen;
HPEN hSceneBorderPen;
HDC hDC;
HDC hPaintDC;
HBITMAP hBitmap;
LPCSTR pTitle       = "Tetris";
LPCSTR pWindowClass = "Tetris";
static const COLORREF backgroundColor = RGB(  0,   0,   0);
static const COLORREF borderColor     = RGB(255, 255, 255);
static const COLORREF squareColor     = RGB(255,   0,   0);
static const int width            = 480;
static const int height           = 540;
static const int sceneBorder      = 2;
static const int squareBorder     = 1;
static const int squareSideLength = 20;
static const int topPadding       = 40;
static const int leftPadding      = 40;
static const int tetrisMoveInterval = 200;
static const int timerID    = 2015;
struct SquarePosition
{
    int row;
    int col;
};
enum {squarePerTetris = 4};
struct TetrisPosition
{
    struct SquarePosition squares[squarePerTetris];
} tetris;
enum {rowNumber = 20};
enum {colNumber = 15};
BOOL squareFilled[rowNumber][colNumber];
int minFilledRow, minFilledCol, maxFilledCol;

RECT GetRect(int minRow, int minCol, int maxRow, int maxCol)
{
    RECT rect;
    rect.left = leftPadding+(minCol-1)*squareSideLength;
    rect.top = topPadding+(minRow-2)*squareSideLength;
    rect.right = leftPadding+maxCol*squareSideLength+2*squareBorder;
    rect.bottom = topPadding+maxRow*squareSideLength+2*squareBorder;
    return rect;
}

void InvalidateScene()
{
    RECT rect = GetRect(1, 1, rowNumber, colNumber);
    InvalidateRect(hWnd, &rect, FALSE);
}

void GenerateTetris()
{
    int i = 0;
    static struct TetrisPosition IShape = {{
        {-3, colNumber / 2},
        {-2, colNumber / 2},
        {-1, colNumber / 2},
        {0, colNumber / 2},
    }};
    static struct TetrisPosition OShape = {{
        {-1, colNumber / 2 - 1},
        {0, colNumber / 2 - 1},
        {-1, colNumber / 2},
        {0, colNumber / 2},
    }};
    static struct TetrisPosition LShape = {{
        {-2, colNumber / 2 - 1},
        {-1, colNumber / 2 - 1},
        {0, colNumber / 2 - 1},
        {0, colNumber / 2},
    }};
    static struct TetrisPosition TShape = {{
        {-1, colNumber / 2 - 1},
        {-1, colNumber / 2},
        {0, colNumber / 2},
        {-1, colNumber / 2 + 1},
    }};
    static struct TetrisPosition ZShape = {{
        {-1, colNumber / 2 - 1},
        {-1, colNumber / 2},
        {0, colNumber / 2},
        {0, colNumber / 2 + 1},
    }};
    static struct TetrisPosition *pShapes[] =
    {&IShape, &OShape, &LShape, &TShape, &ZShape};
    int index = (int)(rand()*(sizeof(pShapes)/sizeof(pShapes[0]))/(RAND_MAX+1.0f));

    struct TetrisPosition *pShape = pShapes[index];
    for (; i!= squarePerTetris; ++i)
    {
        tetris.squares[i].row = pShape->squares[i].row;
        tetris.squares[i].col = pShape->squares[i].col;
    }
}

void InitDraw(HDC hDC)
{
    hPaintDC = CreateCompatibleDC(hDC);
    hBitmap = CreateCompatibleBitmap(hDC, width, height);
    SelectObject(hPaintDC, hBitmap);
    hBackgroundBrush = CreateSolidBrush(backgroundColor);
    hSceneBorderPen = CreatePen(PS_SOLID, sceneBorder, borderColor);
    hSquareBorderPen = CreatePen(PS_SOLID, squareBorder, borderColor);
    hSquareBrush = CreateSolidBrush(squareColor);
}

void DrawScene()
{
    const int sceneWidth = colNumber * squareSideLength;
    const int sceneHeight = rowNumber * squareSideLength;
    POINT points[5];
    points[0].x = leftPadding-squareBorder;
    points[0].y = topPadding-squareBorder;
    points[1].x = leftPadding+sceneWidth+squareBorder;
    points[1].y = points[0].y;
    points[2].x = points[1].x;
    points[2].y = topPadding+sceneHeight+squareBorder;
    points[3].x = points[0].x;
    points[3].y = points[2].y;
    points[4] = points[0];
    SelectObject(hPaintDC, hSceneBorderPen);
    SelectObject(hPaintDC, hBackgroundBrush);
    Polygon(hPaintDC, points, 5);
}

BOOL DrawSquare(int row, int col)
{
    if (row > rowNumber || row < 1 ||
            col > colNumber || col < 1) {
        return FALSE;
    }
    SelectObject(hPaintDC, hSquareBorderPen);
    SelectObject(hPaintDC, hSquareBrush);
    POINT points[5];
    points[0].x = leftPadding+(col-1)*squareSideLength;
    points[0].y = topPadding+(row-1)*squareSideLength;
    points[1].x = leftPadding+col*squareSideLength;
    points[1].y = points[0].y;
    points[2].x = points[1].x;
    points[2].y = topPadding+row*squareSideLength;
    points[3].x = points[0].x;
    points[3].y = points[2].y;
    points[4] = points[0];
    Polygon(hPaintDC, points, 5);
}

void DrawTetris()
{
    int i = 0;
    for (; i!=squarePerTetris; ++i)
    {
        DrawSquare(tetris.squares[i].row, tetris.squares[i].col);
    }
}

void DrawFilledSquares()
{
    int i, j;
    for (i = 0; i!=rowNumber; ++i) {
        for (j = 0; j!=colNumber; j++) {
            if (squareFilled[i][j])
            {
                DrawSquare(i+1, j+1);
            }
        }
    }
}

void OnDraw(HDC hDC)
{
    int i = 1;
    static BOOL initialed = FALSE;
    if (!initialed) {
        InitDraw(hDC);
        initialed = TRUE;
    }

    DrawScene();
    DrawTetris();
    DrawFilledSquares();

    BitBlt(hDC, 0, 0, width, height, hPaintDC, 0, 0, SRCCOPY);
}

BOOL FillSquares()
{
    int i = 0;
    int row, col;
    for (; i!=squarePerTetris; ++i)
    {
        row = tetris.squares[i].row;
        col = tetris.squares[i].col;
        if (row < 1) {
            return FALSE;
        }
        else
        {
            if (row < minFilledRow)
            {
                minFilledRow = row;
            }
            if (col < minFilledCol)
            {
                minFilledCol = col;
            }
            else if (col > maxFilledCol) {
                maxFilledCol = col;
            }
            squareFilled[row-1][col-1] = TRUE;
        }
    }
    return TRUE;
}

BOOL MoveTetrisDown()
{
    struct TetrisPosition temp = tetris;
    int i = 0;
    for (; i!=squarePerTetris; ++i)
    {
        if (temp.squares[i].row < rowNumber)
        {
            if (temp.squares[i].row > 0 &&
                    squareFilled[temp.squares[i].row][temp.squares[i].col-1])
            {
                return FALSE;
            }
            ++temp.squares[i].row;
        }
        else
        {
            return FALSE;
        }
    }
    tetris = temp;
    return TRUE;
}

BOOL MoveTetrisLeft()
{
    struct TetrisPosition temp = tetris;
    int i = 0;
    for (; i!=squarePerTetris; ++i)
    {
         if ((temp.squares[i].col > 1) &&
        !squareFilled[temp.squares[i].row][temp.squares[i].col-1])
        {
            --temp.squares[i].col;
        }
        else
        {
            return FALSE;
        }
    }
    tetris = temp;
    return TRUE;
}

BOOL MoveTetrisRight()
{
    struct TetrisPosition temp = tetris;
    int i = 0;
    for (; i!=squarePerTetris; ++i)
    {
         if ((temp.squares[i].col < colNumber) &&
        !squareFilled[temp.squares[i].row][temp.squares[i].col+1])
        {
            ++temp.squares[i].col;
        }
        else
        {
            return FALSE;
        }
    }
    tetris = temp;
    return TRUE;
}

BOOL MoveTetrisToBottom()
{
    int i;
    BOOL canMoveDown = TRUE;
    BOOL tetrisMoved = FALSE;
    while (canMoveDown)
    {
        for (i=0; i!=squarePerTetris; ++i)
        {
            if (tetris.squares[i].row >= rowNumber ||
                    squareFilled[tetris.squares[i].row][tetris.squares[i].col-1])
            {
                canMoveDown = FALSE;
                break;
            }
        }
        if (canMoveDown)
        {
            for (i=0; i!=squarePerTetris; ++i)
            {
                ++tetris.squares[i].row;
            }
            tetrisMoved = TRUE;
        }
    }
    return tetrisMoved;
}

BOOL RotateSquareCloskwize(
        struct SquarePosition *pSquare, struct SquarePosition circle)
{
    struct SquarePosition temp;
    temp.row = circle.row + (pSquare->col - circle.col);
    temp.col = circle.col + (circle.row - pSquare->row);
    if (temp.row < 1 || temp.row > rowNumber
            || temp.col < 1 || temp.col > colNumber)
    {
        return FALSE;
    }
    else
    {
        *pSquare = temp;
        return TRUE;
    }
}

BOOL RotateTetrisClockwize()
{
    struct SquarePosition circle = tetris.squares[0];
    struct TetrisPosition temp = tetris;
    int i=1;
    for (; i!=squarePerTetris; ++i)
    {
        circle.row += tetris.squares[i].row;
        circle.col += tetris.squares[i].col;
    }
    circle.row /= squarePerTetris;
    circle.col /= squarePerTetris;
    for (i=0; i!=squarePerTetris; ++i)
    {
        if (!RotateSquareCloskwize(&temp.squares[i], circle))
        {
            return FALSE;
        }
    }
    tetris = temp;
    return TRUE;
}

BOOL ClearRow(int row)
{
    int j = 0;
    for (; j!=colNumber; ++j)
    {
        if (!squareFilled[row-1][j])
        {
            return FALSE;
        }
    }
    return TRUE;
}

BOOL MoveFilledSquaresDown(int maxRow)
{
    int i = maxRow;
    int j = 0;
    if (maxRow >= rowNumber || maxRow < 1)
    {
        return FALSE;
    }
    for (; i!=0; --i)
    {
        for (j=0; j!=colNumber; ++j)
        {
            squareFilled[i][j] = squareFilled[i-1][j];
        }
    }
    return TRUE;
}

LRESULT OnCreate()
{
    int i, j;
    srand(time(NULL));
    for (i=0; i!=rowNumber; ++i)
    {
        for (j=0; j!=colNumber; ++j)
        {
            squareFilled[i][j] = FALSE;
        }
    }
    GenerateTetris();
    return 0;
}

LRESULT OnDestroy()
{
    KillTimer(hWnd, timerID);
    PostQuitMessage(0);
}

LRESULT OnPaint()
{
    PAINTSTRUCT ps;
    HDC hdc;
    hdc = BeginPaint(hWnd, &ps);
    OnDraw(hdc);
    EndPaint(hWnd, &ps);
    return 0;
}

LRESULT OnTimer()
{
    if (!MoveTetrisDown())
    {
        if (FillSquares())
        {
            int row = rowNumber;
            while (row != 0)
            {
                if (ClearRow(row))
                {
                    MoveFilledSquaresDown(row-1);
                }
                else
                {
                    --row;
                }
            }
            GenerateTetris();
        }
        else
        {
            DestroyWindow(hWnd);
            return;
        }
    }
    InvalidateScene();
    return 0;
}

LRESULT OnKeyDown(WPARAM wParam)
{
    if (wParam == VK_LEFT)
    {
        if (MoveTetrisLeft())
        {
            InvalidateScene();
        }
    }
    else if (wParam == VK_RIGHT)
    {
        if (MoveTetrisRight())
        {
            InvalidateScene();
        }
    }
    else if (wParam == VK_DOWN)
    {
        if (MoveTetrisToBottom())
        {
            InvalidateScene();
        }
    }
    else if (wParam == VK_UP)
    {
        if (RotateTetrisClockwize())
        {
            InvalidateScene();
        }
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE:
            return OnCreate();
            break;
        case WM_KEYDOWN:
            return OnKeyDown(wParam);
            break;
        case WM_TIMER:
            return OnTimer();
            break;
        case WM_PAINT:
            return OnPaint();
            break;
        case WM_DESTROY:
            return OnDestroy();
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
    hInst = hInstance;
    hWnd = CreateWindow(pWindowClass, pTitle, WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL,
                        hInstance, NULL);
    if (NULL == hWnd)
    {
        return FALSE;
    }

    SetTimer(hWnd, timerID, tetrisMoveInterval, NULL);

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

