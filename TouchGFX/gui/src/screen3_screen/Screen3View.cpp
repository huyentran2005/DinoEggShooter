#include <gui/screen3_screen/Screen3View.hpp>

Screen3View::Screen3View()
{
}

void Screen3View::setupScreen()
{
    Screen3ViewBase::setupScreen();

    uint16_t s = presenter->getFinalScore();

    Unicode::snprintf(
            scoreBuffer,
            SCORE_SIZE,
            "%d",
            s);
    score.setWildcard(scoreBuffer);
    score.invalidate();
}

void Screen3View::tearDownScreen()
{
    Screen3ViewBase::tearDownScreen();
}

