#include "BubbleGame.hpp"
#include <cstring>

BubbleGame::BubbleGame() {
    gs.rng = 123;  // seed random
}

void BubbleGame::init() {
    memset(gs.cells, 0, sizeof(gs.cells));

    // Random 5 hàng đầu tiên
    for (int r = 0; r < 5; r++) {
        int maxC = (r % 2 == 0) ? COLS : COLS - 1;
        for (int c = 0; c < maxC; c++) {
            gs.cells[r][c].color = (BubbleColor)(rng8() % NUM_COLORS);
            gs.cells[r][c].alive = true;
        }
    }

    gs.currentColor = (BubbleColor)(rng8() % NUM_COLORS);
    gs.nextColor    = (BubbleColor)(rng8() % NUM_COLORS);
    gs.aimAngle     = M_PI / 2.0f;
    gs.score        = 0;
    gs.gameOver     = false;
    gs.ball.active  = false;
}

uint8_t BubbleGame::rng8() {
    gs.rng ^= gs.rng << 7;
    gs.rng ^= gs.rng >> 5;
    gs.rng ^= gs.rng << 3;
    return gs.rng;
}

// === Các hàm chuyển đổi tọa độ (hex grid) ===
void BubbleGame::cellToPixel(int row, int col, int& px, int& py) {
    int offsetX = (row % 2 == 0) ? 0 : R_PIX;
    px = offsetX + col * (R_PIX * 2) + R_PIX;
    py = row * (int)(R_PIX * 1.72f) + R_PIX;
}

bool BubbleGame::pixelToCell(int px, int py, int& row, int& col) {
    row = (int)roundf((py - R_PIX) / (R_PIX * 1.72f));
    if (row < 0) row = 0;
    if (row >= ROWS) row = ROWS - 1;

    int offsetX = (row % 2 == 0) ? 0 : R_PIX;
    col = (int)roundf((px - R_PIX - offsetX) / (float)(R_PIX * 2));

    int maxCol = (row % 2 == 0) ? COLS : COLS - 1;
    if (col < 0) col = 0;
    if (col >= maxCol) col = maxCol - 1;
    return true;
}

// === Logic bắn bóng ===
void BubbleGame::shootBall(float angle) {
    if (gs.ball.active || gs.gameOver) return;

    if (angle < 0.15f) angle = 0.15f;
    if (angle > M_PI - 0.15f) angle = M_PI - 0.15f;

    const float SPEED = 7.0f;
    gs.ball.x = SCREEN_W / 2.0f;
    gs.ball.y = SCREEN_H - 50.0f;
    gs.ball.vx = cosf(angle) * SPEED;
    gs.ball.vy = -sinf(angle) * SPEED;
    gs.ball.color = gs.currentColor;
    gs.ball.active = true;
}

void BubbleGame::updateBall() {
    if (!gs.ball.active) return;

    gs.ball.x += gs.ball.vx;
    gs.ball.y += gs.ball.vy;

    // Nảy tường trái phải
    if (gs.ball.x < R_PIX) {
        gs.ball.x = R_PIX;
        gs.ball.vx = -gs.ball.vx;
    }
    if (gs.ball.x > SCREEN_W - R_PIX) {
        gs.ball.x = SCREEN_W - R_PIX;
        gs.ball.vx = -gs.ball.vx;
    }

    // Va chạm
    bool hit = false;
    int hitR = 0, hitC = 0;

    for (int r = 0; r < ROWS && !hit; r++) {
        int maxC = (r % 2 == 0) ? COLS : COLS - 1;
        for (int c = 0; c < maxC && !hit; c++) {
            if (!gs.cells[r][c].alive) continue;
            int px, py;
            cellToPixel(r, c, px, py);
            float dist = sqrtf((gs.ball.x - px)*(gs.ball.x - px) + (gs.ball.y - py)*(gs.ball.y - py));
            if (dist < R_PIX * 1.8f) {
                hit = true;
                pixelToCell((int)gs.ball.x, (int)gs.ball.y, hitR, hitC);
            }
        }
    }

    if (gs.ball.y <= R_PIX || hit) {
        if (gs.cells[hitR][hitC].alive) {
            // Tìm ô trống lân cận
            int nr[6], nc[6];
            int n = getNeighbors(hitR, hitC, nr, nc);
            for (int i = 0; i < n; i++) {
                if (!gs.cells[nr[i]][nc[i]].alive) {
                    hitR = nr[i]; hitC = nc[i];
                    break;
                }
            }
        }
        placeBall(hitR, hitC);
    }
}

void BubbleGame::placeBall(int row, int col) {
    if (row < 0 || row >= ROWS || col < 0 || col >= COLS) return;

    gs.cells[row][col].color = gs.ball.color;
    gs.cells[row][col].alive = true;
    gs.ball.active = false;

    // Kiểm tra match 3+
    int matchR[ROWS*COLS], matchC[ROWS*COLS];
    int count = floodFill(row, col, gs.ball.color, matchR, matchC);

    if (count >= 3) {
        for (int i = 0; i < count; i++) {
            gs.cells[matchR[i]][matchC[i]].alive = false;
        }
        gs.score += count * 10;
        dropFloating();
    }

    gs.currentColor = gs.nextColor;
    gs.nextColor = (BubbleColor)(rng8() % NUM_COLORS);
}

int BubbleGame::getNeighbors(
    int row,
    int col,
    int outR[],
    int outC[]
)
{
    return 0;
}

int BubbleGame::floodFill(
    int startR,
    int startC,
    BubbleColor color,
    int resultR[],
    int resultC[]
)
{
    return 0;
}

void BubbleGame::dropFloating()
{
}
