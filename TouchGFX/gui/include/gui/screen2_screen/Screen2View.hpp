#ifndef SCREEN2VIEW_HPP
#define SCREEN2VIEW_HPP

#include <gui_generated/screen2_screen/Screen2ViewBase.hpp>
#include <gui/screen2_screen/Screen2Presenter.hpp>
#include <gui/common/BubbleGame.hpp>
#include <touchgfx/widgets/Box.hpp>
#include <touchgfx/widgets/canvas/Line.hpp>
#include <touchgfx/widgets/canvas/PainterRGB565.hpp>
#include <touchgfx/widgets/Image.hpp>

class Screen2View : public Screen2ViewBase
{
public:
    Screen2View();
    virtual ~Screen2View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();
private:
    BubbleGame game;
    int gameOverDelay = 0;
    static const uint16_t BUBBLE_COLORS[6];
    Image eggSprites[ROWS * COLS];
    Image currentEgg;
    touchgfx::Line aimLine;
    touchgfx::PainterRGB565 aimLinePainter;
    touchgfx::Image nextEgg;
    touchgfx::Image flyingBall;
    touchgfx::Bitmap getBitmap(BubbleColor color);
    void refreshBoard();
    void updateScoreLabel();
    void handleClickEvent(const ClickEvent& event);
};

#endif
