#ifndef POLYGONITEM_H
#define POLYGONITEM_H

#include <QColor>
#include <QGraphicsItem>

#include "params.h"

#include "p2dengine/objects/p2dpolygonobject.h"
#include "p2dengine/scene/p2dbody.h"
#include "p2dengine/scene/p2dscenemanager.h"
#include "p2dengine/general/p2dtimer.h"

class PolygonItem : public QGraphicsItem
{
public:
    PolygonItem(QColor color, QGraphicsScene *parent);
    ~PolygonItem();

    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);

public: /*Related to p2dengine*/
    void BindP2DBody(P2DScene *scene, QVector<QPointF> points, P2DBodyType bodyType = P2D_DYNAMIC_BODY);
    P2DBody* GetP2DBody(){return body;}
    void SetTexture(QImage& tex) {
        texture = (tex.copy(0,0,tex.width(),tex.height()));
    }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

private:
    QGraphicsScene *parentScene;

    int x;
    int y;
    QColor color;
    QPainterPath path;

    QImage texture;
    bool useTexture;

private: /*Related to p2dengine*/
    P2DBody* body;
    P2DAABB *aabb;


private: /*timing*/
    P2DTimer timer;

};

#endif // POLYGONITEM_H
