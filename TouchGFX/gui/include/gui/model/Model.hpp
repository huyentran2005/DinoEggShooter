#ifndef MODEL_HPP
#define MODEL_HPP

#include <stdint.h>


class ModelListener;

class Model
{
public:
    Model();

    void bind(ModelListener* listener)
    {
        modelListener = listener;
    }

    void tick();
    void setFinalScore(uint16_t score);
    uint16_t getFinalScore();

private:
    uint16_t finalScore;
protected:
    ModelListener* modelListener;
};

#endif // MODEL_HPP
