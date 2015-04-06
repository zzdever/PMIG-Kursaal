#include "scenemanager.h"

SceneManager::SceneManager()
{
    QColor color(100,100,100);
    item = new Chip(color, 0, 0);
    item->setPos(QPointF(0, 0));
    addItem(item);

    /*
    item = new Chip(color, 0, 0);
    item->setPos(QPointF(5000, 0));
    addItem(item);

    item = new Chip(color, 0, 0);
    item->setPos(QPointF(-5000, 0));
    addItem(item);

    item = new Chip(color, 0, 0);
    item->setPos(QPointF(0, 5000));
    addItem(item);

    item = new Chip(color, 0, 0);
    item->setPos(QPointF(0, -5000));
    addItem(item);
    */

    tmp=0;
}


void SceneManager::Render()
{
    // Physical engine solve here
    //tmp++;
    //item->setPos(QPointF(0, tmp%100));
    return;


    QImage image("/Users/ying/Desktop/aa.png");

    // Populate scene
    int xx = 0;
    int nitems = 0;
    for (int i = -5000; i < 5000; i += 110) {
        ++xx;
        int yy = 0;
        for (int j = -7000; j < 7000; j += 70) {
            ++yy;
            qreal x = (i + 11000) / 22000.0;
            qreal y = (j + 7000) / 14000.0;

            QColor color(image.pixel(int(image.width() * x), int(image.height() * y)));
            QGraphicsItem *item = new Chip(color, xx, yy);
            item->setPos(QPointF(i, j));
            addItem(item);

            ++nitems;
        }
    }
}

void SceneManager::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug()<<"MyGraphicsScene"<<event->pos();
    qDebug()<<"MyGraphicsScene scene"<<event->scenePos();
    qDebug()<<"MyGraphicsScene screen"<<event->screenPos();

    QGraphicsScene::mousePressEvent(event);
}

