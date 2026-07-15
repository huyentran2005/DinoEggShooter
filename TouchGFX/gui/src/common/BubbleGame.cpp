#include <gui/common/BubbleGame.hpp>
#include <cstring>
extern "C"
{
    void PlayBreakSound(void);
}
BubbleGame::BubbleGame() {
    gs.rng = 123;
}

void BubbleGame::seedRandom(uint32_t seed) {
    if (seed == 0) seed = 1;
    gs.rng = seed;
}

void BubbleGame::init() {
    memset(gs.cells, 0, sizeof(gs.cells));
    gs.topRowOdd = true;
    for (int r = 0; r < 5; r++) {
    	int maxC = getRowSize(r);
        for (int c = 0; c < maxC; c++) {
            gs.cells[r][c].color = (BubbleColor)(rng8() % NUM_COLORS);
            gs.cells[r][c].alive = true;
        }
    }
    for(int i = 0; i < MAX_FALLING; i++)
    {
        gs.falling[i].active = false;
    }

    gs.currentColor = (BubbleColor)(rng8() % NUM_COLORS);
    gs.nextColor    = (BubbleColor)(rng8() % NUM_COLORS);
    gs.aimAngle     = M_PI / 2.0f;
    gs.score        = 0;
    gs.gameOver     = false;
    gs.ball.active  = false;
}

//Random màu
uint8_t BubbleGame::rng8() {
    gs.rng ^= gs.rng << 7;
    gs.rng ^= gs.rng >> 5;
    gs.rng ^= gs.rng << 3;
    return gs.rng;
}

// Chuyển từ (row, col) => (pixel x, pixel y)
// Đệm 2 bên lề trái phải
// Hàng lẻ sẽ bị lệch nửa ô so với hàng chẵn
void BubbleGame::cellToPixel(int row, int col, int& px, int& py) {
	const int SIDE_MARGIN = 13;
	const int TOP_MARGIN = 23;

	int offsetX = isShortRow(row) ? R_PIX : 0;
    px = SIDE_MARGIN + offsetX + col * (R_PIX * 2) + R_PIX;
    py = TOP_MARGIN + row * (int)(R_PIX * 1.732f) + R_PIX;
}


// Ngược lại so với hàm cellToPixel()
// Dùng để chuyển đội tọa độ của bóng sau
// khi bắn lên lưới sẽ nằm ở (row, col) nào
bool BubbleGame::pixelToCell(int px, int py, int& row, int& col) {
	const int SIDE_MARGIN = 13;
	const int TOP_MARGIN = 23;

    row = (int)roundf((py - TOP_MARGIN - R_PIX) / (R_PIX * 1.72f));

    if (row < 0) row = 0;
    if (row >= ROWS) row = ROWS - 1;

    int offsetX = isShortRow(row) ? R_PIX : 0;
    col = (int)roundf((px - SIDE_MARGIN - R_PIX - offsetX) / (float)(R_PIX * 2));

    int maxCol = getRowSize(row);
    if (col < 0) col = 0;
    if (col >= maxCol) col = maxCol - 1;
    return true;
}

bool BubbleGame::isShortRow(int row)
{
    bool shortRow = ((row & 1) != 0);

    if(gs.topRowOdd)
        shortRow = !shortRow;

    return shortRow;
}

int BubbleGame::getRowSize(int row)
{
    return isShortRow(row) ? (COLS - 1) : COLS;
}

//Điều khiển hướng bắn bóng
void BubbleGame::shootBall(float angle) {
    if (gs.ball.active || gs.gameOver) return;

    // Giới hạn góc bắn để không bắn quá ngang
    if (angle < 0.15f) angle = 0.15f;
    if (angle > M_PI - 0.15f) angle = M_PI - 0.15f;

    const float SPEED = 7.0f;
    gs.ball.x  = SCREEN_W / 2.0f - 12.0f;
    gs.ball.y  = SCREEN_H - 70.0f;
    gs.ball.vx = cosf(angle) * SPEED;
    gs.ball.vy = -sinf(angle) * SPEED;
    gs.ball.color  = gs.currentColor;
    gs.ball.active = true;
}


void BubbleGame::pushBoardDown()
{
    // kiểm tra thua
    for(int c = 0; c < COLS; c++)
    {
        if(gs.cells[ROWS-1][c].alive)
        {
            gs.gameOver = true;
            return;
        }
    }

    // dịch xuống
    for(int r = ROWS - 1; r > 0; r--)
    {
    	int maxPrev = getRowSize(r - 1);

        for(int c = 0; c < COLS; c++)
        {
            if(c < maxPrev)
                gs.cells[r][c] = gs.cells[r - 1][c];
            else
                gs.cells[r][c].alive = false;
        }
    }
    gs.topRowOdd = !gs.topRowOdd;
    // sinh hàng mới
    int maxTop = getRowSize(0);

    for(int c = 0; c < COLS; c++)
    {
        if(c < maxTop)
        {
            gs.cells[0][c].alive = true;
            gs.cells[0][c].color =
                (BubbleColor)(rng8() % NUM_COLORS);
        }
        else
        {
            gs.cells[0][c].alive = false;
        }
    }

//    Thua neu cham gioi han
    for (int c = 0; c < COLS; c++)
        {
            if (gs.cells[ROWS - 2][c].alive)
            {
                gs.gameOver = true;
                return;
            }
    }

}

// Cập nhật trạng thái bóng bắn
void BubbleGame::updateBall() {
    const int SIDE_MARGIN = 13;
    if (!gs.ball.active) return;

    gs.ball.x += gs.ball.vx;
    gs.ball.y += gs.ball.vy;

    // Nảy tường
    if (gs.ball.x < R_PIX + SIDE_MARGIN) {
        gs.ball.x = R_PIX + SIDE_MARGIN;
        gs.ball.vx = -gs.ball.vx;
    }
    if (gs.ball.x > SCREEN_W - R_PIX - SIDE_MARGIN) {
        gs.ball.x = SCREEN_W - R_PIX - SIDE_MARGIN;
        gs.ball.vx = -gs.ball.vx;
    }

    bool hit = false;

    // 1. Kiểm tra va chạm với trần
    if (gs.ball.y <= R_PIX + 23) { // 23 là TOP_MARGIN
        hit = true;
    }

    // 2. Kiểm tra va chạm với các bóng khác
    if (!hit) {
        for (int r = 0; r < ROWS; r++) {
            int maxC = getRowSize(r);
            for (int c = 0; c < maxC; c++) {
                if (!gs.cells[r][c].alive) continue;

                int px, py;
                cellToPixel(r, c, px, py);
                float dx = gs.ball.x - px;
                float dy = gs.ball.y - py;
                float distSq = dx * dx + dy * dy;

                // Khoảng cách va chạm: (R+R)^2 = (2*12)^2 = 576
                // Dùng 1.8f * R_PIX để bóng "lún" vào một chút mới tính là chạm, tránh chạm sượt
                if (distSq < (R_PIX * 1.8f) * (R_PIX * 1.8f)) {
                    hit = true;
                    break;
                }
            }
            if (hit) break;
        }
    }

    if (hit) {
        // Tìm ô TRỐNG gần nhất với vị trí hiện tại của quả bóng đang bay
        int bestR = -1, bestC = -1;
        float minDist = 999999.0f;

        for (int r = 0; r < ROWS; r++) {
            int maxC = getRowSize(r);
            for (int c = 0; c < maxC; c++) {
                if (gs.cells[r][c].alive) continue; // Ô đã có bóng thì bỏ qua

                int px, py;
                cellToPixel(r, c, px, py);
                float dx = gs.ball.x - px;
                float dy = gs.ball.y - py;
                float dSq = dx * dx + dy * dy;

                if (dSq < minDist) {
                    minDist = dSq;
                    bestR = r;
                    bestC = c;
                }
            }
        }

        if (bestR != -1) {
            placeBall(bestR, bestC);
        } else {
            gs.ball.active = false; // Trường hợp lỗi không tìm thấy ô
        }
    }

    // Kiểm tra bóng bay ra ngoài màn hình dưới (nếu có lỗi)
    if (gs.ball.y > SCREEN_H) gs.ball.active = false;
}

// Đặt bóng bắn dính trên lưới và xử lý nổ
void BubbleGame::placeBall(int row, int col) {
    if (row < 0 || row >= ROWS || col < 0 || col >= COLS) return;

    gs.cells[row][col].color = gs.ball.color;
    gs.cells[row][col].alive = true;
    gs.ball.active = false;

    if (row >= ROWS - 2) {
    	gs.gameOver = true;
        return;
    }

    int matchR[ROWS * COLS], matchC[ROWS * COLS];
    int count = floodFill(row, col, gs.ball.color, matchR, matchC);

    if (count >= 3) {
        for (int i = 0; i < count; i++) {
        	spawnFallingBubble(matchR[i], matchC[i]);

        	gs.cells[matchR[i]][matchC[i]].alive = false;
        }
        gs.score += count * 10;
        PlayBreakSound();
        dropFloating();
    }

    gs.currentColor = gs.nextColor;
    gs.nextColor    = (BubbleColor)(rng8() % NUM_COLORS);
}

// === getNeighbors: trả về 6 ô lân cận theo hex grid ===
int BubbleGame::getNeighbors(int row, int col, int outR[], int outC[]) {
    int count = 0;

    // Offset hex grid: hàng chẵn và hàng lẻ khác nhau
    int dr[6], dc[6];
    if (!isShortRow(row)) {
        // Hàng chẵn
        dr[0] = -1; dc[0] = -1;
        dr[1] = -1; dc[1] =  0;
        dr[2] =  0; dc[2] = -1;
        dr[3] =  0; dc[3] =  1;
        dr[4] =  1; dc[4] = -1;
        dr[5] =  1; dc[5] =  0;
    } else {
        // Hàng lẻ
        dr[0] = -1; dc[0] =  0;
        dr[1] = -1; dc[1] =  1;
        dr[2] =  0; dc[2] = -1;
        dr[3] =  0; dc[3] =  1;
        dr[4] =  1; dc[4] =  0;
        dr[5] =  1; dc[5] =  1;
    }

    for (int i = 0; i < 6; i++) {
        int nr = row + dr[i];
        int nc = col + dc[i];
        if (nr < 0 || nr >= ROWS) continue;
        int maxC = getRowSize(nr);
        if (nc < 0 || nc >= maxC) continue;
        outR[count] = nr;
        outC[count] = nc;
        count++;
    }
    return count;
}

// === floodFill: BFS tìm tất cả bóng cùng màu liên thông ===
int BubbleGame::floodFill(int startR, int startC, BubbleColor color,
                           int resultR[], int resultC[]) {
    // Visited array
    bool visited[ROWS][COLS];
    memset(visited, 0, sizeof(visited));

    // BFS queue
    int qR[ROWS * COLS], qC[ROWS * COLS];
    int head = 0, tail = 0;
    int count = 0;

    qR[tail] = startR;
    qC[tail] = startC;
    tail++;
    visited[startR][startC] = true;

    while (head < tail) {
        int r = qR[head];
        int c = qC[head];
        head++;

        if (!gs.cells[r][c].alive) continue;
        if (gs.cells[r][c].color != color) continue;

        resultR[count] = r;
        resultC[count] = c;
        count++;

        int nr[6], nc[6];
        int n = getNeighbors(r, c, nr, nc);
        for (int i = 0; i < n; i++) {
            if (!visited[nr[i]][nc[i]]) {
                visited[nr[i]][nc[i]] = true;
                qR[tail] = nr[i];
                qC[tail] = nc[i];
                tail++;
            }
        }
    }
    return count;
}

// === dropFloating: xóa bóng không còn liên kết với hàng trên ===
//Nếu quả bóng A chạm trần -> Nó sống.
//Nếu quả bóng B chạm vào quả bóng A -> Nó cũng sống (vì nó được A giữ).
//Nếu quả bóng C không có đường nào nối ngược về hàng 0 -> Nó phải rơi.
void BubbleGame::dropFloating() {
    bool connected[ROWS][COLS];
    memset(connected, 0, sizeof(connected));

    // BFS từ hàng 0 — mọi bóng liên thông với hàng 0 là "còn treo"
    int qR[ROWS * COLS], qC[ROWS * COLS];
    int head = 0, tail = 0;

    for (int c = 0; c < COLS; c++) {
        if (gs.cells[0][c].alive && !connected[0][c]) {
            connected[0][c] = true;
            qR[tail] = 0;
            qC[tail] = c;
            tail++;
        }
    }

    while (head < tail) {
        int r = qR[head];
        int c = qC[head];
        head++;

        int nr[6], nc[6];
        int n = getNeighbors(r, c, nr, nc);
        for (int i = 0; i < n; i++) {
            int rr = nr[i], cc = nc[i];
            if (!connected[rr][cc] && gs.cells[rr][cc].alive) {
                connected[rr][cc] = true;
                qR[tail] = rr;
                qC[tail] = cc;
                tail++;
            }
        }
    }

    // Xóa tất cả bóng không connected
    for (int r = 0; r < ROWS; r++) {
    	int maxC = getRowSize(r);
        for (int c = 0; c < maxC; c++) {
            if (gs.cells[r][c].alive && !connected[r][c]) {
            	spawnFallingBubble(r, c);

            	gs.cells[r][c].alive = false;
                gs.score += 5; // bonus điểm bóng rơi
            }
        }
    }
}

void BubbleGame::spawnFallingBubble(int row, int col)
{
    for(int i = 0; i < MAX_FALLING; i++)
    {
        if(!gs.falling[i].active)
        {
            int px, py;
            cellToPixel(row, col, px, py);

            gs.falling[i].x = px;
            gs.falling[i].y = py;

            gs.falling[i].vy = 1.0f;

            gs.falling[i].color =
                gs.cells[row][col].color;

            gs.falling[i].active = true;
            break;
        }
    }
}

void BubbleGame::updateFalling()
{
    for(int i = 0; i < MAX_FALLING; i++)
    {
        if(!gs.falling[i].active)
            continue;

        gs.falling[i].vy += 0.25f; // gia tốc trọng trường

        gs.falling[i].y += gs.falling[i].vy;

        if(gs.falling[i].y > SCREEN_H + 30)
        {
            gs.falling[i].active = false;
        }
    }
}
