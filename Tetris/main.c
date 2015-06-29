#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

LPCSTR pTitle       = "Tetris";
LPCSTR pWindowClass = "Tetris";
static const COLORREF textColor       = RGB(255, 255, 255);
static const COLORREF gameOverColor   = RGB(255,   0,   0);
static const COLORREF backgroundColor = RGB(0,   0,   0);
static const COLORREF borderColor     = RGB(255, 255, 255);
static const int width            = 400;
static const int height           = 480;
static const int sceneBorder      = 2;
static const int squareBorder     = 1;
static const int squareSideLength = 20;
static const int topPadding       = 20;
static const int leftPadding      = 20;
static const int timerID          = 2015;
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
int nextShape;
int nextDirection;
enum {rowNum = 20};
enum {colNum = 10};
BOOL squareFilled[rowNum][colNum];
COLORREF filledShape[rowNum][colNum];
int minFilledRow, minFilledCol, maxFilledCol;

static const struct TetrisPosition IShape[] =
{
    {{{ -3, colNum / 2}, { -2, colNum / 2}, { -1, colNum / 2}, {0, colNum / 2}}},
    {{{ -2, colNum / 2 + 2}, { -2, colNum / 2 + 1}, { -2, colNum / 2}, { -2, colNum / 2 - 1}}},
    {{{0, colNum / 2 + 1}, { -1, colNum / 2 + 1}, { -2, colNum / 2 + 1}, { -3, colNum / 2 + 1}}},
    {{{ -1, colNum / 2 - 1}, { -1, colNum / 2}, { -1, colNum / 2 + 1}, { -1, colNum / 2 + 2}}},
};
static const struct TetrisPosition OShape[] =
{
    {{{ -1, colNum / 2}, { -1, colNum / 2 + 1}, {0, colNum / 2 + 1}, {0, colNum / 2}}},
    {{{ -1, colNum / 2}, { -1, colNum / 2 + 1}, {0, colNum / 2 + 1}, {0, colNum / 2}}},
    {{{ -1, colNum / 2}, { -1, colNum / 2 + 1}, {0, colNum / 2 + 1}, {0, colNum / 2}}},
    {{{ -1, colNum / 2}, { -1, colNum / 2 + 1}, {0, colNum / 2 + 1}, {0, colNum / 2}}},
};
static const struct TetrisPosition TShape[] =
{
    {{{ -1, colNum / 2 - 1}, { -1, colNum / 2}, { -1, colNum / 2 + 1}, {0, colNum / 2}}},
    {{{ -2, colNum / 2}, { -1, colNum / 2 - 1}, {0, colNum / 2}, { -1, colNum / 2}}},
    {{{ -1, colNum / 2 + 1}, { -1, colNum / 2}, { -1, colNum / 2 - 1}, { -2, colNum / 2}}},
    {{{0, colNum / 2}, { -1, colNum / 2}, { -2, colNum / 2}, { -1, colNum / 2 + 1}}},
};
static const struct TetrisPosition LShape[] =
{
    {{{ -2, colNum / 2}, { -1, colNum / 2}, {0, colNum / 2}, {0, colNum / 2 + 1}}},
    {{{ -1, colNum / 2 + 1}, { -1, colNum / 2}, { -1, colNum / 2 - 1}, {0, colNum / 2 - 1}}},
    {{{0, colNum / 2}, { -1, colNum / 2}, { -2, colNum / 2}, { -2, colNum / 2 - 1}}},
    {{{ -1, colNum / 2 - 1}, { -1, colNum / 2}, { -1, colNum / 2 + 1}, { -2, colNum / 2 + 1}}},
};
static const struct TetrisPosition JShape[] =
{
    {{{ -2, colNum / 2}, { -1, colNum / 2}, {0, colNum / 2}, {0, colNum / 2 - 1}}},
    {{{ -1, colNum / 2 + 1}, { -1, colNum / 2}, { -1, colNum / 2 - 1}, { -2, colNum / 2 - 1}}},
    {{{0, colNum / 2}, { -1, colNum / 2}, { -2, colNum / 2}, { -2, colNum / 2 + 1}}},
    {{{ -1, colNum / 2 - 1}, { -1, colNum / 2}, { -1, colNum / 2 + 1}, {0, colNum / 2 + 1}}},
};
static const struct TetrisPosition ZShape[] =
{
    {{{ -1, colNum / 2 - 1}, { -1, colNum / 2}, {0, colNum / 2}, {0, colNum / 2 + 1}}},
    {{{ -2, colNum / 2}, { -1, colNum / 2}, { -1, colNum / 2 - 1}, {0, colNum / 2 - 1}}},
    {{{ -1, colNum / 2 + 1}, { -1, colNum / 2}, { -2, colNum / 2}, { -2, colNum / 2 - 1}}},
    {{{0, colNum / 2}, { -1, colNum / 2}, { -1, colNum / 2 + 1}, { -2, colNum / 2 + 1}}},
};
static const struct TetrisPosition SShape[] =
{
    {{{ -1, colNum / 2 + 1}, { -1, colNum / 2}, {0, colNum / 2}, {0, colNum / 2 - 1}}},
    {{{0, colNum / 2}, { -1, colNum / 2}, { -1, colNum / 2 - 1}, { -2, colNum / 2 - 1}}},
    {{{ -1, colNum / 2 - 1}, { -1, colNum / 2}, { -2, colNum / 2}, { -2, colNum / 2 + 1}}},
    {{{ -2, colNum / 2 - 1}, { -1, colNum / 2 - 1}, { -1, colNum / 2}, {0, colNum / 2}}},
};
typedef const struct TetrisPosition(*pXShapes)[];
pXShapes shapesArray[] =
{&IShape, &OShape, &TShape, &LShape, &JShape, &ZShape, &SShape};
static const int directionNum = 4;
static const COLORREF squareColor[] = {RGB(255, 0, 0), RGB(255, 165, 0),
    RGB(222, 222, 0), RGB(0, 255, 0), RGB(0, 127, 255), RGB(0, 0, 255),
    RGB(139, 0, 255)
};

HINSTANCE hInst;
HWND      hWnd;
HBRUSH    hBackgroundBrush;
HBRUSH    hSquareBrush[sizeof(squareColor) / sizeof(squareColor[0])];
HPEN      hSquareBorderPen;
HPEN      hSceneBorderPen;
HFONT     hTextFont;
HFONT     hGameOverFont;
HDC       hDC;
HDC       hPaintDC;
HBITMAP   hBitmap;

int level;
int points;
int lines;
int moveInterval;
static const int linesToUpgrade   = 10;
static const int highestLevel     = 10;
static const int originalInterval = 500;
static const int intervalStep     = 50;
static const int pointsIncrease[] = {5, 15, 30, 50};
LPCSTR pNext     = "NEXT";
LPCSTR pLevel    = "LEVEL";
LPCSTR pPoints   = "POINTS";
LPCSTR pGameOver = "GAME OVER!";
BOOL stopped = FALSE;
BOOL paused  = FALSE;

RECT GetRect(int minRow, int minCol, int maxRow, int maxCol)
{
    RECT rect;
    rect.left   = leftPadding + (minCol - 1) * squareSideLength;
    rect.top    = topPadding + (minRow - 2) * squareSideLength;
    rect.right  = leftPadding + maxCol * squareSideLength + 2 * squareBorder;
    rect.bottom = topPadding + maxRow * squareSideLength + 2 * squareBorder;
    return rect;
}

void InvalidateScene()
{
    RECT rect = GetRect(1, 1, rowNum, colNum);
    InvalidateRect(hWnd, &rect, FALSE);
}

void InvalidateInfo()
{
    const int sceneWidth = colNum * squareSideLength;
    RECT rect = {leftPadding * 2 + sceneWidth, topPadding,
        width - leftPadding, height - topPadding };
    InvalidateRect(hWnd, &rect, FALSE);
}

void GenerateTetris()
{
    int i = 0;
    shape = nextShape;
    direction = nextDirection;
    nextShape = (int)(1.0 * rand() * (sizeof(shapesArray)
                / sizeof(shapesArray[0])) / (RAND_MAX + 1.0));
    nextDirection = (int)(1.0 * rand() * directionNum / (RAND_MAX + 1.0));
    tetris = (*shapesArray[shape])[direction];
}

void InitDraw(HDC hDC)
{
    int i = 0;
    hPaintDC = CreateCompatibleDC(hDC);
    hBitmap  = CreateCompatibleBitmap(hDC, width, height);
    SelectObject(hPaintDC, hBitmap);
    hBackgroundBrush = CreateSolidBrush(backgroundColor);
    hSceneBorderPen  = CreatePen(PS_SOLID, sceneBorder, borderColor);
    hSquareBorderPen = CreatePen(PS_SOLID, squareBorder, borderColor);
    for (; i != sizeof(hSquareBrush) / sizeof(hSquareBrush[0]); ++i)
    {
        hSquareBrush[i] = CreateSolidBrush(squareColor[i]);
    }
    hTextFont = CreateFont(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET,
                           OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY,
                           DEFAULT_PITCH | FF_SWISS, "Arial");
    hGameOverFont = CreateFont(32, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET,
                               OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY,
                               DEFAULT_PITCH | FF_SWISS, "Arial");
    SetBkMode(hPaintDC, TRANSPARENT);
}

void DrawGameOver()
{
    SetTextCharacterExtra(hPaintDC, 1);
    SelectObject(hPaintDC, hGameOverFont);
    SetTextColor(hPaintDC, gameOverColor);
    RECT rect = GetRect(1, 1, rowNum, colNum);
    DrawText(hPaintDC, pGameOver, -1, &rect,
             DT_SINGLELINE | DT_CENTER | DT_VCENTER);
}

void DrawNextTetris()
{
    int i = 0;
    const struct TetrisPosition nextTetris =
        (*shapesArray[nextShape])[nextDirection];
    int row, col;
    const int sceneWidth = colNum * squareSideLength;
    RECT infoRect = {leftPadding + sceneWidth, topPadding,
        width - leftPadding, height - topPadding };
    int textHeight = (infoRect.bottom - infoRect.top) / 8;
    int originalX = infoRect.left + (width - infoRect.left) / 2 - 2 *
                    squareSideLength;
    int originalY = infoRect.top + textHeight;

    SelectObject(hPaintDC, hSquareBorderPen);
    SelectObject(hPaintDC, hSquareBrush[nextShape]);
    for (; i != squarePerTetris; ++i)
    {
        row = nextTetris.squares[i].row + 4;
        col = nextTetris.squares[i].col - colNum / 2 + 2;
        POINT points[5];
        points[0].x = originalX + (col - 1) * squareSideLength;
        points[0].y = originalY + (row - 1) * squareSideLength;
        points[1].x = originalX + col * squareSideLength;
        points[1].y = points[0].y;
        points[2].x = points[1].x;
        points[2].y = originalY + row * squareSideLength;
        points[3].x = points[0].x;
        points[3].y = points[2].y;
        points[4]   = points[0];
        Polygon(hPaintDC, points, 5);
    }
}

void DrawInfo()
{
    SetTextCharacterExtra(hPaintDC, 5);
    SelectObject(hPaintDC, hTextFont);
    SetTextColor(hPaintDC, textColor);
    const int sceneWidth = colNum * squareSideLength;
    RECT infoRect = {leftPadding + sceneWidth, topPadding,
        width - leftPadding, height - topPadding };
    FillRect(hPaintDC, &infoRect, hBackgroundBrush);
    int textHeight = (infoRect.bottom - infoRect.top) / 8;
    int textWidth  = infoRect.right - infoRect.left;
    char buffer[10];
    RECT textRect = {infoRect.left, infoRect.top, infoRect.left + textWidth,
        infoRect.top + textHeight };
    DrawText(hPaintDC, pNext, -1, &textRect,
             DT_SINGLELINE | DT_CENTER | DT_VCENTER);
    textRect.top    = textRect.top + 3 * textHeight;
    textRect.bottom = textRect.top + textHeight;
    DrawText(hPaintDC, pLevel, -1, &textRect,
             DT_SINGLELINE | DT_CENTER | DT_VCENTER);
    textRect.top    = textRect.top + 1 * textHeight;
    textRect.bottom = textRect.top + textHeight;
    sprintf(buffer, "%d", level);
    DrawText(hPaintDC, buffer, -1, &textRect,
             DT_SINGLELINE | DT_CENTER | DT_VCENTER);
    textRect.top    = textRect.top + 1 * textHeight;
    textRect.bottom = textRect.top + textHeight;
    DrawText(hPaintDC, pPoints, -1, &textRect,
             DT_SINGLELINE | DT_CENTER | DT_VCENTER);
    textRect.top    = textRect.top + 1 * textHeight;
    textRect.bottom = textRect.top + textHeight;
    sprintf(buffer, "%d", points);
    DrawText(hPaintDC, buffer, -1, &textRect,
             DT_SINGLELINE | DT_CENTER | DT_VCENTER);
    DrawNextTetris();
}

void DrawScene()
{
    const int sceneWidth  = colNum * squareSideLength;
    const int sceneHeight = rowNum * squareSideLength;
    POINT points[5];
    points[0].x = leftPadding - squareBorder;
    points[0].y = topPadding - squareBorder;
    points[1].x = leftPadding + sceneWidth + squareBorder;
    points[1].y = points[0].y;
    points[2].x = points[1].x;
    points[2].y = topPadding + sceneHeight + squareBorder;
    points[3].x = points[0].x;
    points[3].y = points[2].y;
    points[4].x = points[0].x;
    points[4].y = points[0].y - 1;
    SelectObject(hPaintDC, hSceneBorderPen);
    SelectObject(hPaintDC, hBackgroundBrush);
    Polygon(hPaintDC, points, 5);
}

BOOL DrawSquare(int row, int col)
{
    if (row > rowNum || row < 1 || col > colNum || col < 1)
    {
        return FALSE;
    }
    POINT points[5];
    points[0].x = leftPadding + (col - 1) * squareSideLength;
    points[0].y = topPadding + (row - 1) * squareSideLength;
    points[1].x = leftPadding + col * squareSideLength;
    points[1].y = points[0].y;
    points[2].x = points[1].x;
    points[2].y = topPadding + row * squareSideLength;
    points[3].x = points[0].x;
    points[3].y = points[2].y;
    points[4]   = points[0];
    Polygon(hPaintDC, points, 5);
}

void DrawTetris()
{
    SelectObject(hPaintDC, hSquareBorderPen);
    SelectObject(hPaintDC, hSquareBrush[shape]);
    int i = 0;
    for (; i != squarePerTetris; ++i)
    {
        DrawSquare(tetris.squares[i].row, tetris.squares[i].col);
    }
}

void DrawFilledSquares()
{
    SelectObject(hPaintDC, hSquareBorderPen);
    int i, j;
    for (i = 0; i != rowNum; ++i)
    {
        for (j = 0; j != colNum; j++)
        {
            if (squareFilled[i][j])
            {
                SelectObject(hPaintDC, hSquareBrush[filledShape[i][j]]);
                DrawSquare(i + 1, j + 1);
            }
        }
    }
}

void OnDraw(HDC hDC)
{
    int i = 1;
    static BOOL initialed = FALSE;
    if (!initialed)
    {
        InitDraw(hDC);
        initialed = TRUE;
    }

    DrawInfo();
    DrawScene();
    DrawTetris();
    DrawFilledSquares();
    if (stopped)
    {
        DrawGameOver();
    }

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
    int newDir = (oldDir + 1) % directionNum;
    struct TetrisPosition temp = tetris;
    for (; i != squarePerTetris; ++i)
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
    direction = (direction + 1) % directionNum;
    return TRUE;
}

BOOL FillSquares()
{
    int i = 0;
    int row, col;
    for (; i != squarePerTetris; ++i)
    {
        row = tetris.squares[i].row;
        col = tetris.squares[i].col;
        if (row < 1)
        {
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
            else if (col > maxFilledCol)
            {
                maxFilledCol = col;
            }
            squareFilled[row - 1][col - 1] = TRUE;
            filledShape[row - 1][col - 1]  = shape;
        }
    }
    return TRUE;
}

BOOL MoveTetrisDown()
{
    struct TetrisPosition temp = tetris;
    int i = 0;
    for (; i != squarePerTetris; ++i)
    {
        if (temp.squares[i].row < rowNum)
        {
            if (temp.squares[i].row > 0 &&
                squareFilled[temp.squares[i].row][temp.squares[i].col - 1])
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
    for (; i != squarePerTetris; ++i)
    {
        if ((temp.squares[i].col > 1) &&
            !squareFilled[temp.squares[i].row - 1][temp.squares[i].col - 2])
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
    for (; i != squarePerTetris; ++i)
    {
        if ((temp.squares[i].col < colNum) &&
            !squareFilled[temp.squares[i].row - 1][temp.squares[i].col])
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
        for (i = 0; i != squarePerTetris; ++i)
        {
            if (tetris.squares[i].row < 1)
            {
                continue;
            }
            else if (tetris.squares[i].row >= rowNum ||
                     squareFilled[tetris.squares[i].row][tetris.squares[i].col - 1])
            {
                canMoveDown = FALSE;
                break;
            }
        }
        if (canMoveDown)
        {
            for (i = 0; i != squarePerTetris; ++i)
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
    for (; j != colNum; ++j)
    {
        if (!squareFilled[row - 1][j])
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
    for (; i != 0; --i)
    {
        for (j = 0; j != colNum; ++j)
        {
            squareFilled[i][j] = squareFilled[i - 1][j];
            filledShape[i][j]  = filledShape[i - 1][j];
        }
    }
    for (j = 0; j != colNum; ++j)
    {
        squareFilled[0][j] = FALSE;
    }
    return TRUE;
}

void OnStart()
{
    int i, j;
    level        = 1;
    points       = 0;
    moveInterval = originalInterval;
    for (i = 0; i != rowNum; ++i)
    {
        for (j = 0; j != colNum; ++j)
        {
            squareFilled[i][j] = FALSE;
        }
    }
    nextShape = (int)(1.0 * rand() * (sizeof(shapesArray)
                / sizeof(shapesArray[0])) / (RAND_MAX + 1.0));
    nextDirection = (int)(1.0 * rand() * directionNum / (RAND_MAX + 1.0));
    GenerateTetris();
    stopped = FALSE;
    paused  = FALSE;
    InvalidateInfo();
    InvalidateScene();
    SetTimer(hWnd, timerID, moveInterval, NULL);
}

LRESULT OnCreate()
{
    srand(time(NULL));
    OnStart();
    return 0;
}

LRESULT OnDestroy()
{
    KillTimer(hWnd, timerID);
    PostQuitMessage(0);
    DeleteObject(hBackgroundBrush);
    DeleteObject(hSquareBrush);
    DeleteObject(hSceneBorderPen);
    DeleteObject(hSquareBorderPen);
    DeleteObject(hTextFont);
    DeleteObject(hBitmap);
    DeleteDC(hPaintDC);
    DeleteDC(hDC);
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
    if (stopped || paused)
    {
        return 0;
    }
    if (!MoveTetrisDown())
    {
        if (FillSquares())
        {
            int row = rowNum;
            int cleared = 0;
            while (row != 0)
            {
                if (ClearRow(row))
                {
                    ++cleared;
                    ++lines;
                    if (lines % linesToUpgrade == 0 && level < highestLevel)
                    {
                        ++level;
                        KillTimer(hWnd, timerID);
                        moveInterval = originalInterval - (level - 1) * intervalStep;
                        SetTimer(hWnd, timerID, moveInterval, NULL);
                    }
                    MoveFilledSquaresDown(row - 1);
                }
                else
                {
                    --row;
                }
            }
            if (cleared > 0)
            {
                points += level * pointsIncrease[cleared - 1];
            }
            InvalidateInfo();
            GenerateTetris();
        }
        else
        {
            stopped = TRUE;
            KillTimer(hWnd, timerID);
        }
    }
    InvalidateScene();
    return 0;
}

LRESULT OnKeyDown(WPARAM wParam)
{
    if (VK_ESCAPE == wParam)
    {
        DestroyWindow(hWnd);
    }
    else if (VK_RETURN == wParam)
    {
        OnStart();
    }
    else if (VK_SPACE == wParam)
    {
        paused = !paused;
    }
    else if (VK_LEFT == wParam)
    {
        if (MoveTetrisLeft())
        {
            InvalidateScene();
        }
    }
    else if (VK_RIGHT == wParam)
    {
        if (MoveTetrisRight())
        {
            InvalidateScene();
        }
    }
    else if (VK_DOWN == wParam)
    {
        if (MoveTetrisToBottom())
        {
            InvalidateScene();
        }
    }
    else if (VK_UP == wParam)
    {
        if (RotateTetrisClockwize())
        {
            InvalidateScene();
        }
    }
    return 0;
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

    wcex.cbSize        = sizeof(WNDCLASSEX);
    wcex.style         = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc   = WndProc;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = 0;
    wcex.hInstance     = hInstance;
    wcex.hIcon         = NULL;
    wcex.hCursor       = NULL;
    wcex.hbrBackground = CreateSolidBrush(backgroundColor);
    wcex.lpszMenuName  = NULL;
    wcex.lpszClassName = pWindowClass;
    wcex.hIconSm       = NULL;

    return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;
    hWnd  = CreateWindow(pWindowClass, pTitle, WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, width, height,
            NULL, NULL, hInstance, NULL);
    if (NULL == hWnd)
    {
        return FALSE;
    }

    SetTimer(hWnd, timerID, moveInterval, NULL);

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

