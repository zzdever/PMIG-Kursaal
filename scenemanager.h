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
    void Render();


private:
    QGraphicsItem *item;

    bool isDrawing;
    DrawingPolygonItem *drawingItem;
    PolygonItem *polyItem;


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









/*


class Tiles : public Test
{
public:
    enum
    {
        e_count = 20
    };

    Tiles()
    {
        m_fixtureCount = 0;
        b2Timer timer;

        {
            float32 a = 0.5f;
            b2BodyDef bd;
            bd.position.y = -a;
            b2Body* ground = m_world->CreateBody(&bd);

#if 1
            int32 N = 200;
            int32 M = 10;
            b2Vec2 position;
            position.y = 0.0f;
            for (int32 j = 0; j < M; ++j)
            {
                position.x = -N * a;
                for (int32 i = 0; i < N; ++i)
                {
                    b2PolygonShape shape;
                    shape.SetAsBox(a, a, position, 0.0f);
                    ground->CreateFixture(&shape, 0.0f);
                    ++m_fixtureCount;
                    position.x += 2.0f * a;
                }
                position.y -= 2.0f * a;
            }
#else
            int32 N = 200;
            int32 M = 10;
            b2Vec2 position;
            position.x = -N * a;
            for (int32 i = 0; i < N; ++i)
            {
                position.y = 0.0f;
                for (int32 j = 0; j < M; ++j)
                {
                    b2PolygonShape shape;
                    shape.SetAsBox(a, a, position, 0.0f);
                    ground->CreateFixture(&shape, 0.0f);
                    position.y -= 2.0f * a;
                }
                position.x += 2.0f * a;
            }
#endif
        }

        {
            float32 a = 0.5f;
            b2PolygonShape shape;
            shape.SetAsBox(a, a);

            b2Vec2 x(-7.0f, 0.75f);
            b2Vec2 y;
            b2Vec2 deltaX(0.5625f, 1.25f);
            b2Vec2 deltaY(1.125f, 0.0f);

            for (int32 i = 0; i < e_count; ++i)
            {
                y = x;

                for (int32 j = i; j < e_count; ++j)
                {
                    b2BodyDef bd;
                    bd.type = b2_dynamicBody;
                    bd.position = y;

                    //if (i == 0 && j == 0)
                    //{
                    //	bd.allowSleep = false;
                    //}
                    //else
                    //{
                    //	bd.allowSleep = true;
                    //}

                    b2Body* body = m_world->CreateBody(&bd);
                    body->CreateFixture(&shape, 5.0f);
                    ++m_fixtureCount;
                    y += deltaY;
                }

                x += deltaX;
            }
        }

        m_createTime = timer.GetMilliseconds();
    }

    void Step(Settings* settings)
    {
        const b2ContactManager& cm = m_world->GetContactManager();
        int32 height = cm.m_broadPhase.GetTreeHeight();
        int32 leafCount = cm.m_broadPhase.GetProxyCount();
        int32 minimumNodeCount = 2 * leafCount - 1;
        float32 minimumHeight = ceilf(logf(float32(minimumNodeCount)) / logf(2.0f));
        g_debugDraw.DrawString(5, m_textLine, "dynamic tree height = %d, min = %d", height, int32(minimumHeight));
        m_textLine += DRAW_STRING_NEW_LINE;

        Test::Step(settings);

        g_debugDraw.DrawString(5, m_textLine, "create time = %6.2f ms, fixture count = %d",
            m_createTime, m_fixtureCount);
        m_textLine += DRAW_STRING_NEW_LINE;

        //b2DynamicTree* tree = &m_world->m_contactManager.m_broadPhase.m_tree;

        //if (m_stepCount == 400)
        //{
        //	tree->RebuildBottomUp();
        //}
    }

    static Test* Create()
    {
        return new Tiles;
    }

    int32 m_fixtureCount;
    float32 m_createTime;
};


*/













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
