#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QTimer>

#include "playground.h"

class SceneManager : public QGraphicsScene{
public:
    SceneManager();
    void Render();

private:
    QGraphicsItem *item;

    int tmp;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

};


#endif // SCENEMANAGER_H
