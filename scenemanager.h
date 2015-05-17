#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QTimer>
#include <QFile>
#include <QPainter>

#include "playground.h"
#include "polygonitem.h"

#include "p2dengine/general/p2dmath.h"
#include "p2dengine/general/p2dtimer.h"
#include "p2dengine/scene/p2dscenemanager.h"
#include "p2dengine/scene/p2dbody.h"
#include "p2dengine/scene/p2dfixture.h"
#include "p2dengine/general/p2dparams.h"


class DrawingPolygonItem;

class SceneManager : public QGraphicsScene{
public:
    SceneManager();
    ~SceneManager();

    void contextMenuEvent(QGraphicsSceneContextMenuEvent* e);
    void keyPressEvent(QKeyEvent* e);

    void Render();
    void toggleEngine();


private:
    QGraphicsItem *item;

    bool isDrawing;
    DrawingPolygonItem *drawingItem;
    PolygonItem* polyItem;

    bool isEngineRunning;


private: /*Related to p2dengine*/
    P2DScene* scene;
    P2DBody* body;

    float32 timeStep;
    int32 velocityIterations;
    int32 positionIterations;

    void InitP2DEngine();

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
