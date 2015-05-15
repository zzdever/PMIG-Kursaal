#include "utils.h"

P2DVec2 CoordinateInterface::MapToEngine(QPointF pIn)
{//qDebug()<<"MapToEngine input"<<pIn;
    float32 x = (pIn.x()-(-SCENE_WIDTH_HALF)) / (SCENE_WIDTH_HALF*2)
            * (ENGINE_SCENE_MAXIMUM_HALF*2)
            + (-ENGINE_SCENE_MAXIMUM_HALF);

    float32 y = (pIn.y()-(-SCENE_HEIGHT_HALF)) / (SCENE_HEIGHT_HALF*2)
            * (ENGINE_SCENE_MAXIMUM_HALF*2)
            + (-ENGINE_SCENE_MAXIMUM_HALF);

    P2DVec2 pOut(x,y);

    return pOut;
}

QPointF CoordinateInterface::MapToScene(P2DVec2 pIn)
{//qDebug()<<"MapToScene input"<<pIn;
    qreal x = (pIn.x-(-ENGINE_SCENE_MAXIMUM_HALF)) / (ENGINE_SCENE_MAXIMUM_HALF*2)
            * (SCENE_WIDTH_HALF*2)
            + (-SCENE_WIDTH_HALF);

    qreal y = (pIn.y-(-ENGINE_SCENE_MAXIMUM_HALF)) / (ENGINE_SCENE_MAXIMUM_HALF*2)
            * (SCENE_HEIGHT_HALF*2)
            + (-SCENE_HEIGHT_HALF);

    QPointF pOut(x,y);

    return pOut;
}
