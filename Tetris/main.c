#include <windows.h>
#include <stdlib.h>

HINSTANCE hInst;
HWND      hWnd;
HBRUSH    hBackgroundBrush;
HBRUSH    hSquareBrush;
HPEN      hSquareBorderPen;
HPEN      hSceneBorderPen;
HDC       hDC;
HDC       hPaintDC;
HBITMAP   hBitmap;
LPCSTR pTitle       = "Tetris";
LPCSTR pWindowClass = "Tetris";
static const COLORREF backgroundColor = RGB(  0,   0,   0);
static const COLORREF borderColor     = RGB(255, 255, 255);
static const COLORREF squareColor     = RGB(255,   0,   0);
static const int width              = 480;
static const int height             = 480;
static const int sceneBorder        = 2;
static const int squareBorder       = 1;
static const int squareSideLength   = 20;
static const int topPadding         = 22;
static const int leftPadding        = 22;
static const int tetrisMoveInterval = 500;
static const int timerID            = 2015;
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
int shape;
int direction;
enum {rowNum = 20};
enum {colNum = 10};
BOOL squareFilled[rowNum][colNum];
int minFilledRow, minFilledCol, maxFilledCol;

static const struct TetrisPosition IShape[] = {
    {{{-3, colNum/2}, {-2, colNum/2}, {-1, colNum/2}, {0, colNum/2}}},
    {{{-2, colNum/2+2}, {-2, colNum/2+1}, {-2, colNum/2}, {-2, colNum/2-1}}},
    {{{0, colNum/2+1}, {-1, colNum/2+1}, {-2, colNum/2+1}, {-3, colNum/2+1}}},
    {{{-1, colNum/2-1}, {-1, colNum/2}, {-1, colNum/2+1}, {-1, colNum/2+2}}},
};
static const struct TetrisPosition OShape[] = {
    {{{-1, colNum/2}, {-1, colNum/2+1}, {0, colNum/2+1}, {0, colNum/2}}},
    {{{-1, colNum/2}, {-1, colNum/2+1}, {0, colNum/2+1}, {0, colNum/2}}},
    {{{-1, colNum/2}, {-1, colNum/2+1}, {0, colNum/2+1}, {0, colNum/2}}},
    {{{-1, colNum/2}, {-1, colNum/2+1}, {0, colNum/2+1}, {0, colNum/2}}},
};
static const struct TetrisPosition TShape[] = {
    {{{-1, colNum/2-1}, {-1, colNum/2}, {-1, colNum/2+1}, {0, colNum/2}}},
    {{{-2, colNum/2}, {-1, colNum/2-1}, {0, colNum/2}, {-1, colNum/2}}},
    {{{-1, colNum/2+1}, {-1, colNum/2}, {-1, colNum/2-1}, {-2, colNum/2}}},
    {{{0, colNum/2}, {-1, colNum/2}, {-2, colNum/2}, {-1, colNum/2+1}}},
};
static const struct TetrisPosition LShape[] = {
    {{{-2, colNum/2}, {-1, colNum/2}, {0, colNum/2}, {0, colNum/2+1}}},
    {{{-1, colNum/2+1}, {-1, colNum/2}, {-1, colNum/2-1}, {0, colNum/2-1}}},
    {{{0, colNum/2}, {-1, colNum/2}, {-2, colNum/2}, {-2, colNum/2-1}}},
    {{{-1, colNum/2-1}, {-1, colNum/2}, {-1, colNum/2+1}, {-2, colNum/2+1}}},
};
static const struct TetrisPosition JShape[] = {
    {{{-2, colNum/2}, {-1, colNum/2}, {0, colNum/2}, {0, colNum/2-1}}},
    {{{-1, colNum/2+1}, {-1, colNum/2}, {-1, colNum/2-1}, {-2, colNum/2-1}}},
    {{{0, colNum/2}, {-1, colNum/2}, {-2, colNum/2}, {-2, colNum/2+1}}},
    {{{-1, colNum/2-1}, {-1, colNum/2}, {-1, colNum/2+1}, {0, colNum/2+1}}},
};
static const struct TetrisPosition ZShape[] = {
    {{{-1, colNum/2-1}, {-1, colNum/2}, {0, colNum/2}, {0, colNum/2+1}}},
    {{{-2, colNum/2}, {-1, colNum/2}, {-1, colNum/2-1}, {0, colNum/2-1}}},
    {{{-1, colNum/2+1}, {-1, colNum/2}, {-2, colNum/2}, {-2, colNum/2-1}}},
    {{{0, colNum/2}, {-1, colNum/2}, {-1, colNum/2+1}, {-2, colNum/2+1}}},
};
static const struct TetrisPosition SShape[] = {
    {{{-1, colNum/2+1}, {-1, colNum/2}, {0, colNum/2}, {0, colNum/2-1}}},
    {{{0, colNum/2}, {-1, colNum/2}, {-1, colNum/2-1}, {-2, colNum/2-1}}},
    {{{-1, colNum/2-1}, {-1, colNum/2}, {-2, colNum/2}, {-2, colNum/2+1}}},
    {{{-2, colNum/2+1}, {-1, colNum/2+1}, {-1, colNum/2}, {0, colNum/2}}},
};
typedef const struct TetrisPosition (*pXShapes)[];
pXShapes shapesArray[] =
{&IShape, &OShape, &TShape, &LShape, &JShape, &ZShape, &SShape};
static const int directionNum = 4;

#ifdef DEBUG
#define OutputString(string) OutputDebugString(string)
#include <stdio.h>
void _OutputTetrisPosition()
{
    char buffer[100];
    sprintf(buffer, "{row=%d, col=%d}, {row=%d, col=%d}, \
            {row=%d, col=%d}, {row=%d, col=%d}\n",
            tetris.squares[0].row, tetris.squares[0].col,
            tetris.squares[1].row, tetris.squares[1].col,
            tetris.squares[2].row, tetris.squares[2].col,
            tetris.squares[3].row, tetris.squares[3].col);
    OutputDebugString(buffer);
}
#define OutputTetrisPosition() _OutputTetrisPosition()
BOOL paused = FALSE;
#else
#define OutputString(string)
#define OutputTetrisPosition()
#endif

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
    RECT rect = GetRect(1, 1, rowNum, colNum);
    InvalidateRect(hWnd, &rect, FALSE);
}

void GenerateTetris()
{
    int i = 0;
    shape = (int)(1.0*rand()*(sizeof(shapesArray)/sizeof(shapesArray[0]))/(RAND_MAX+1.0));
    const pXShapes pShapes = shapesArray[shape];
    direction = (int)(rand()*4.0/(RAND_MAX+1.0));
    tetris = (*pShapes)[direction];
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
    const int sceneWidth = colNum * squareSideLength;
    const int sceneHeight = rowNum * squareSideLength;
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
    if (row > rowNum || row < 1 ||
            col > colNum || col < 1) {
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
    for (i = 0; i!=rowNum; ++i) {
        for (j = 0; j!=colNum; j++) {
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
BOOL IsSquareInScene(const struct SquarePosition *pSquare)
{
    return pSquare->row > 0 && pSquare->row <= rowNum
        && pSquare->col > 0 && pSquare->col <= colNum;
}

BOOL RotateTetrisClockwize()
{
    int i = 0;
    const pXShapes pShapes = shapesArray[shape];
    int oldDir = direction;
    int newDir = (oldDir+1)%directionNum;
    struct TetrisPosition temp = tetris;
    for (; i!=squarePerTetris; ++i)
    {
        temp.squares[i].row += (*pShapes)[newDir].squares[i].row
            - (*pShapes)[oldDir].squares[i].row;
        temp.squares[i].col += (*pShapes)[newDir].squares[i].col
            - (*pShapes)[oldDir].squares[i].col;
        if (!IsSquareInScene(&temp.squares[i]))
        {
            return FALSE;
        }
    }
    tetris = temp;
    direction = (direction+1) % directionNum;
    return TRUE;
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
        if (temp.squares[i].row < rowNum)
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
                !squareFilled[temp.squares[i].row-1][temp.squares[i].col-2])
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
        if ((temp.squares[i].col < colNum) &&
                !squareFilled[temp.squares[i].row-1][temp.squares[i].col])
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
            if (tetris.squares[i].row >= rowNum ||
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

BOOL ClearRow(int row)
{
    int j = 0;
    for (; j!=colNum; ++j)
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
    if (maxRow >= rowNum || maxRow < 1)
    {
        return FALSE;
    }
    for (; i!=0; --i)
    {
        for (j=0; j!=colNum; ++j)
        {
            squareFilled[i][j] = squareFilled[i-1][j];
        }
    }
    for (j=0; j!=colNum; ++j)
    {
        squareFilled[0][j] = FALSE;
    }
    return TRUE;
}

LRESULT OnCreate()
{
    int i, j;
    srand(time(NULL));
    for (i=0; i!=rowNum; ++i)
    {
        for (j=0; j!=colNum; ++j)
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
#ifdef DEBUG
    if (paused)
    {
        return 0;
    }
#endif
    if (!MoveTetrisDown())
    {
        if (FillSquares())
        {
            int row = rowNum;
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
            return 0;
        }
    }
    InvalidateScene();
    return 0;
}

LRESULT OnKeyDown(WPARAM wParam)
{
#ifdef DEBUG
    if (VK_SPACE == wParam)
    {
        paused = !paused;
        return 0;
    }
    if (paused)
    {
        return 0;
    }
#endif
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

