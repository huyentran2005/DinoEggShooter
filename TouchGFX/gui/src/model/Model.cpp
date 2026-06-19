#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>

Model::Model() : modelListener(0)
{

}

void Model::tick()
{

}

void Model::setFinalScore(uint16_t score)
{
    finalScore = score;
}

uint16_t Model::getFinalScore()
{
    return finalScore;
}
