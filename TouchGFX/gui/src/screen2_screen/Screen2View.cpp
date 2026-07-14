#include <gui/screen2_screen/Screen2View.hpp>
#include <touchgfx/Color.hpp>
#include <images/BitmapDatabase.hpp>
#include <cmath>
#include <cstdio>

#include <cmsis_os.h>
extern osMessageQueueId_t myQueue01Handle;
extern "C"
{
    void PlayShootSound(void);
    void PlayStartGameSound(void);
    void PlayGameOverSound(void);
}

Screen2View::Screen2View() { }

void Screen2View::setupScreen()
{
    Screen2ViewBase::setupScreen();
    game.init();
    PlayStartGameSound();

    for(int i = 0; i < ROWS * COLS; i++)
    {
        eggSprites[i].setVisible(false);

        add(eggSprites[i]);
    }
    for(int i = 0; i < MAX_FALLING; i++)
    {
        fallingSprites[i].setVisible(false);
        add(fallingSprites[i]);
    }

    currentEgg.setBitmap(getBitmap(game.gs.currentColor));
    currentEgg.setVisible(true);
    add(currentEgg);

    nextEgg.setBitmap(getBitmap(game.gs.nextColor));
    nextEgg.setXY(50, SCREEN_H - 50);  // góc phải dưới
    nextEgg.setVisible(true);
    add(nextEgg);

    flyingBall.setVisible(false);
    add(flyingBall);


    dotPainter.setColor(
        touchgfx::Color::getColorFromRGB(255,255,100)
    );

    for(int i = 0; i < AIM_DOTS; i++)
    {
        aimDots[i].setPosition(0,0,8,8);

        aimDots[i].setRadius(3);

        aimDots[i].setPainter(dotPainter);

        aimDots[i].setVisible(true);

        add(aimDots[i]);
    }

    refreshBoard();
    updateScoreLabel();
}
void Screen2View::tearDownScreen() {
    Screen2ViewBase::tearDownScreen();
}

typedef struct
  {
      uint8_t x;
      uint8_t y;
      uint8_t button;
  } JoystickData_t;

void Screen2View::handleTickEvent()
{
    Screen2ViewBase::handleTickEvent();

    game.updateBall();
    game.updateFalling();

    // Sau 6s nếu thua chuyển sang màn hình screen3
    if (game.gs.gameOver)
    {
    	gameOverDelay++;

    	if(gameOverDelay == 10){
    		PlayGameOverSound();
    	}
    	if (gameOverDelay >= 120)
        {
        	presenter->gameOver(game.gs.score);
    	}

    	return;
    }

    // Sau 6s thêm một hàng bóng  trên lưới
    static uint32_t tickCount = 0;

    tickCount++;

    if(tickCount >= 900)
    {
        tickCount = 0;
        game.pushBoardDown();
    }

    JoystickData_t joy;

    if(osMessageQueueGet(myQueue01Handle, &joy, NULL, 0) == osOK)
    {
        float minAngle = 0.15f;
        float maxAngle = M_PI - 0.15f;

        // Trái -> trái, phải -> phải
        game.gs.aimAngle =
            maxAngle -
            ((float)joy.x / 255.0f) * (maxAngle - minAngle);

        static bool oldButton = false;

        if(joy.button &&
           !oldButton &&
           !game.gs.ball.active)
        {
            game.shootBall(game.gs.aimAngle);
            PlayShootSound();
        }

        oldButton = joy.button;
    }

    refreshBoard();

    updateScoreLabel();
}

void Screen2View::handleClickEvent(const ClickEvent& event)
{
    if (event.getType() == ClickEvent::PRESSED)
    {

        // Tọa độ điểm chạm
        float touchX = event.getX();
        float touchY = event.getY();

        // Tọa độ điểm xuất phát của bóng (phải khớp với game.shootBall)
        float startX = SCREEN_W / 2.0f - 12.0f;
        float startY = SCREEN_H - 70.0f;

        // Tính góc bắn dựa trên điểm chạm (dùng hàm atan2)
        float dx = touchX - startX;
        float dy = startY - touchY; // Đảo ngược vì trục Y của màn hình hướng xuống
        float angle = atan2f(dy, dx);

        // Gọi logic bắn
        if(angle < 0.15f) angle = 0.15f;
        if(angle > M_PI - 0.15f) angle = M_PI - 0.15f;

        game.gs.aimAngle = angle;
        game.shootBall(angle);
        PlayShootSound();
        refreshBoard();
    }
}

void Screen2View::updateScoreLabel() {
	static int lastScore = -1;

	if (lastScore == game.gs.score)
		return;

	lastScore = game.gs.score;
    Unicode::snprintf(scoreLabelBuffer, SCORELABEL_SIZE, "%d", game.gs.score);
    scoreLabel.setWildcard(scoreLabelBuffer);
    scoreLabel.invalidate();
}

// Đổi color sang ảnh Image tương ứng trong assets
Bitmap Screen2View::getBitmap(BubbleColor color)
{
    switch(color)
    {
    case BC_RED:
        return Bitmap(BITMAP_EGG_RED_ID);

    case BC_BLUE:
        return Bitmap(BITMAP_EGG_BLUE_ID);

    case BC_GREEN:
        return Bitmap(BITMAP_EGG_GREEN_ID);

    case BC_YELLOW:
        return Bitmap(BITMAP_EGG_YELLOW_ID);

    case BC_PINK:
        return Bitmap(BITMAP_EGG_PINK_ID);

    case BC_PURPLE:
        return Bitmap(BITMAP_EGG_PURPLE_ID);

    default:
        return Bitmap(BITMAP_EGG_RED_ID);
    }
}

void Screen2View::refreshBoard()
{
	currentEgg.setBitmap(
	    getBitmap(game.gs.currentColor)
	);
	currentEgg.setXY(
	    SCREEN_W/2 - 12,
	    SCREEN_H - 60
	);
	currentEgg.setVisible(!game.gs.ball.active);
	currentEgg.invalidate();


	nextEgg.setBitmap(getBitmap(game.gs.nextColor));
	nextEgg.setXY(50, SCREEN_H - 50);
	nextEgg.invalidate();

	// Vẽ đường ngắm hướng bắn
//	float startX = SCREEN_W / 2.0f;
//	float startY = SCREEN_H - 50.0f;
//	float lineLength = 100.0f;
//
//	float endX = startX + cosf(game.gs.aimAngle) * lineLength;
//	float endY = startY - sinf(game.gs.aimAngle) * lineLength;
//
//	aimLine.setStart(startX, startY);
//	aimLine.setEnd(endX, endY);
//	aimLine.setLineWidth(3.0f);
//	aimLine.setLineEndingStyle(Line::ROUND_CAP_ENDING);
//	aimLinePainter.setColor(Color::getColorFromRGB(255, 255, 100));
//	aimLine.setPainter(aimLinePainter);
//	aimLine.invalidate();
//	aimLine.setVisible(!game.gs.ball.active);
//	aimLine.invalidate();

	float startX = SCREEN_W / 2.0f;
	float startY = SCREEN_H - 50.0f;

	for(int i = 0; i < AIM_DOTS; i++)
	{
	    float dist = 15.0f + i * 15.0f;

	    int x =
	        startX +
	        cosf(game.gs.aimAngle) * dist;

	    int y =
	        startY -
	        sinf(game.gs.aimAngle) * dist;

	    aimDots[i].setXY(x - 4, y - 4);

	    aimDots[i].setVisible(
	        !game.gs.ball.active
	    );

	    aimDots[i].invalidate();
	}


    int idx = 0;
    if (game.gs.ball.active)
        {

            flyingBall.setBitmap(getBitmap(game.gs.ball.color));
            flyingBall.setXY((int)game.gs.ball.x - 12, (int)game.gs.ball.y - 12);
            flyingBall.setVisible(true);
        }
        else
        {
            flyingBall.setVisible(false);
        }
        flyingBall.invalidate();

    for(int r = 0; r < ROWS; r++)
    {
    	int maxC = game.getRowSize(r);
        for(int c = 0; c < maxC; c++)
        {
        	// ô trống bỏ qua
            if(!game.gs.cells[r][c].alive)
                continue;
            // Nếu có trứng đổi (row, col) => (pixel x, pixel y)
            int px, py;
            game.cellToPixel(r, c, px, py);

            eggSprites[idx].setBitmap(
                getBitmap(game.gs.cells[r][c].color)
            );

            eggSprites[idx].setXY(
                px - 12,
                py - 12
            );

            eggSprites[idx].setVisible(true);

            idx++;
        }
    }
    for(int i = 0; i < MAX_FALLING; i++)
    {
        if(game.gs.falling[i].active)
        {
            fallingSprites[i].setBitmap(
                getBitmap(game.gs.falling[i].color));

            fallingSprites[i].setXY(
                (int)game.gs.falling[i].x - 12,
                (int)game.gs.falling[i].y - 12);

            fallingSprites[i].setVisible(true);
        }
        else
        {
            fallingSprites[i].setVisible(false);
        }

        fallingSprites[i].invalidate();
    }

    while(idx < ROWS * COLS)
    {
        eggSprites[idx].setVisible(false);
        idx++;
    }
    updateScoreLabel();
    invalidate();
}

