#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QTimer>

#include "playground.h"

class DrawingPolygon;

class SceneManager : public QGraphicsScene{
public:
    SceneManager();
    void Render();

private:
    QGraphicsItem *item;

    bool isDrawing;
    DrawingPolygon *drawingItem;

    int tmp;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

};



#include <QColor>
#include <QGraphicsItem>
#include <QtWidgets>

class DrawingPolygon : public QGraphicsItem
{
public:
    DrawingPolygon(QColor color, QPointF p);
    void AddPoint(QPointF p);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);


private:
    QColor color;
    QVector<QPointF> points;
};




#endif // SCENEMANAGER_H
