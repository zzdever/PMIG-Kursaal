#ifndef POLYGONITEM_H
#define POLYGONITEM_H

#include <QColor>
#include <QGraphicsItem>

#include "p2dengine/objects/p2dpolygonobject.h"

class PolygonItem : public QGraphicsItem
{
public:
    PolygonItem(QColor color, QVector<QPointF> points);
    ~PolygonItem();

    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    P2DPolygonObject *p2DPolygonObject;
    P2DTransform *transform;
    P2DAABB *aabb;

    int x;
    int y;
    QColor color;
    QPainterPath path;
};

#endif // POLYGONITEM_H
