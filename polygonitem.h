#ifndef POLYGONITEM_H
#define POLYGONITEM_H

#include <QColor>
#include <QGraphicsItem>

#include "p2dengine/objects/p2dpolygonobject.h"

class PolygonItem : public QGraphicsItem
{
public:
    PolygonItem(const QColor &color, QVector<QPointF> points);

    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    P2DPolygonObject *p2DPolygonObject;

    int x;
    int y;
    QColor color;
    QVector<QPointF> stuff;
};

#endif // POLYGONITEM_H
