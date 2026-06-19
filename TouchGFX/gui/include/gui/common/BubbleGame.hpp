#ifndef BUBBLEGAME_HPP
#define BUBBLEGAME_HPP

#include <stdint.h>
#include <cmath>

constexpr int COLS      = 9;
constexpr int ROWS      = 12;
constexpr int R_PIX     = 12;        // Bán kính bong bóng
constexpr int SCREEN_W  = 240;
constexpr int SCREEN_H  = 320;
constexpr int NUM_COLORS = 6;

enum BubbleColor : uint8_t {
    BC_RED = 0, BC_BLUE, BC_GREEN, BC_YELLOW, BC_PINK, BC_PURPLE, BC_NONE = 0xFF
};

struct Bubble {
    BubbleColor color;
    bool alive;
};

struct ShootBall {
    float x, y;
    float vx, vy;
    BubbleColor color;
    bool active;
};

struct GameState {
    Bubble cells[ROWS][COLS];
    ShootBall ball;
    BubbleColor currentColor;
    BubbleColor nextColor;
    float aimAngle;
    uint16_t score;
    bool gameOver;
    uint8_t rng;
    bool topRowOdd = true;
};

class BubbleGame {
public:
    GameState gs;

    BubbleGame();

    void init();                    // Khởi tạo lưới + random bong bóng
    void shootBall(float angle);
    void updateBall();              // Cập nhật bóng đang bay
    void placeBall(int row, int col);
    void cellToPixel(int row, int col, int& px, int& py);
    void pushBoardDown();
    bool isShortRow(int row);
    int getRowSize(int row);
private:
    bool pixelToCell(int px, int py, int& row, int& col);
    int  getNeighbors(int row, int col, int outR[], int outC[]);
    int  floodFill(int startR, int startC, BubbleColor color, int resultR[], int resultC[]);
    void dropFloating();
    uint8_t rng8();
};

#endif
