#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QTimer>

#include "playground.h"
#include "polygonitem.h"

class DrawingPolygonItem;

class SceneManager : public QGraphicsScene{
public:
    SceneManager();
    void Render();

private:
    QGraphicsItem *item;

    bool isDrawing;
    DrawingPolygonItem *drawingItem;
    PolygonItem *polyItem;

    int tmp;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

};



#include <QColor>
#include <QGraphicsItem>
#include <QtWidgets>

class DrawingPolygonItem : public QGraphicsItem
{
public:
    DrawingPolygonItem(QColor color, QPointF p);
    void AddPoint(QPointF p);

    QColor GetColor(){
        color.setAlpha(255);
        return color;
    }

    QVector<QPointF> GetPoints(){
        return points;
    }

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);


private:
    QColor color;
    QVector<QPointF> points;

    qreal minx, maxx, miny, maxy;
};




#endif // SCENEMANAGER_H
