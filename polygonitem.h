#ifndef POLYGONITEM_H
#define POLYGONITEM_H

#include <QColor>
#include <QGraphicsItem>

#include "p2dengine/objects/p2dpolygonobject.h"
#include "p2dengine/scene/p2dbody.h"
#include "p2dengine/scene/p2dscenemanager.h"

class PolygonItem : public QGraphicsItem
{
public:
    PolygonItem(QColor color);
    ~PolygonItem();

    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);

public: /*Related to p2dengine*/
    void BindP2DBody(P2DScene *scene, QVector<QPointF> points);
    P2DBody* GetP2DBody(){return body;}

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

private:
    //P2DPolygonObject *p2DPolygonObject;
    //P2DTransform *transform;
    //

    int x;
    int y;
    QColor color;
    QPainterPath path;

private: /*Related to p2dengine*/
    P2DBody* body;
    P2DAABB *aabb;
};

#endif // POLYGONITEM_H
