#include "scenemanager.h"



SceneManager::SceneManager()
{
    isDrawing = false;

    QColor color(100,100,100);
    item = new Chip(color, 0, 0);
    item->setPos(QPointF(0, 0));
    addItem(item);

    InitP2DEngine();
}

SceneManager::~SceneManager()
{
    if(item) delete item;
    if(drawingItem) delete drawingItem;
    if(polyItem) delete polyItem;
    if(scene) delete scene;
}


void SceneManager::Render()
{
    // Instruct the scene to perform a single step of simulation.
    // It is generally best to keep the time step and iterations fixed.
    scene->Step(timeStep, velocityIterations, positionIterations);

    // Now print the position and angle of the body.
    P2DVec2 position = body->GetPosition();
    float32 angle = body->GetAngle();

    //qDebug()<< scene->GetBodyCount();

    for(P2DBody* bodyList = scene->GetBodyList();
        bodyList; bodyList = bodyList->GetNext())
    {
        const P2DTransform& xf = bodyList->GetTransform();

        for(P2DFixture* fixture = bodyList->GetFixtureList();
            fixture; fixture = fixture->GetNext())
        {
            P2DPolygonObject* poly = (P2DPolygonObject*)fixture->GetShape();
            int vertexCount = poly->m_count;
            assert(vertexCount <= P2D_MAX_POLYGON_VERTICES);
            P2DVec2 vertices[P2D_MAX_POLYGON_VERTICES];
            for (int i = 0; i < vertexCount; ++i){
                vertices[i] = P2DMul(xf, poly->m_vertices[i]);
            }
            //DrawSolidPolygon(vertices, vertexCount, color);
        }
    }

    //qDebug()<<position.x<<position.y<<angle;
    //printf("%4.2f %4.2f %4.2f\n", position.x, position.y, angle);

    // Refresh the viewport
    update();

    return;
}

void SceneManager::InitP2DEngine()
{
    // Define the gravity vector.
    P2DVec2 gravity(0.0f, -10.0f);
    // Construct a world object, which will hold and simulate the rigid bodies.
    scene = new P2DScene(gravity);

    // Define the ground body.
    P2DBodyDef groundBodyDef;
    groundBodyDef.position.Set(0.0f, -10.0f);
    // Call the body factory which allocates memory for the ground body
    // from a pool and creates the ground box shape (also from a pool).
    // The body is also added to the world.
    P2DBody* groundBody = scene->CreateBody(&groundBodyDef);

    // Define the ground box shape.
    P2DPolygonObject groundBox;
    groundBox.SetARect(50.0f, 10.0f);

    // Add the ground fixture to the ground body.
    groundBody->CreateFixture(&groundBox, 0.0f);

    // Define the dynamic body. We set its position and call the body factory.
    P2DBodyDef bodyDef;
    bodyDef.type = P2D_DYNAMIC_BODY;
    bodyDef.position.Set(0.0f, 4.0f);
    body = scene->CreateBody(&bodyDef);

    // Define another box shape for our dynamic body.
    P2DPolygonObject dynamicBox;
    dynamicBox.SetARect(1.0f, 1.0f);

    // Define the dynamic body fixture.
    P2DFixtureDef fixtureDef;
    fixtureDef.shape = &dynamicBox;
    // Set the box density to be non-zero, so it will be dynamic.
    fixtureDef.density = 1.0f;
    // Override the default friction.
    fixtureDef.friction = 0.3f;

    // Add the shape to the body.
    body->CreateFixture(&fixtureDef);


    // Prepare for simulation. Typically we use a time step of 1/60 of a
    // second (60Hz) and 10 iterations. This provides a high quality simulation
    // in most game scenarios.
    timeStep = 1.0f / 60.0f;
    velocityIterations = 6;
    positionIterations = 2;

}


void SceneManager::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // We can only draw in a blank area.
    if(event->button() == Qt::LeftButton && itemAt(event->scenePos(), QTransform()) == nullptr){
        isDrawing = true;
        drawingItem = new DrawingPolygonItem(QColor(qrand()%255, qrand()%255, qrand()%255), event->scenePos());
        addItem(drawingItem);
    }

    QGraphicsScene::mousePressEvent(event);
}


void SceneManager::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
//qDebug()<<"engine position"<<Coordinate::MapToEngine(event->scenePos());
    if(isDrawing){
        drawingItem->AddPoint(event->scenePos());
    }

    QGraphicsScene::mouseMoveEvent(event);
}


void SceneManager::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if(isDrawing){
        // TODO add to the object tree
        polyItem = new PolygonItem(drawingItem->GetColor());
        polyItem->BindP2DBody(scene, drawingItem->GetPoints());
        //polyItem->setPos(event->scenePos());
        addItem(polyItem);

        isDrawing = false;
        removeItem(drawingItem);

    }

    QGraphicsScene::mouseReleaseEvent(event);
}




DrawingPolygonItem::DrawingPolygonItem(QColor color, QPointF p)
{
    points.push_back(p);
    minx = p.x();
    maxx = p.x();
    miny = p.y();
    maxy = p.y();
    color.setAlpha(49);
    this->color = color;

    // This should be maximized, put it on the top of everything.
    setZValue(100);

    //setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
}



void DrawingPolygonItem::AddPoint(QPointF p)
{
    //p = Coordinate::MapToEngine(p);

    int size = points.size();
    // we do not allow the second point is too close to the first one
    if(size==1 && QVector2D(p - points.at(size - 1)).length()<ENGINE_SCENE_MINIMUM_HALF){
        return;
    }

    bool pop = false;
    if(size >= 2){
        // the last vector
        QVector2D v0 = QVector2D(points.at(size - 1) - points.at(size - 2));
        // the current vector
        QVector2D v1 = QVector2D(p - points.at(size - 1));

        qreal crossProduct = v0.x()*v1.y() - v1.x()*v0.y();
        qreal sinV = crossProduct/(v0.length()*v1.length());
        if(qAbs(sinV) < sin(0.1)/*in radian*/ || v1.length()<ENGINE_SCENE_MINIMUM_HALF)
            pop = true;
    }

    if(pop){
        points.pop_back();
    }

    points.push_back(p);
    if(p.x()<minx) minx = p.x();
    if(p.x()>maxx) maxx = p.x();
    if(p.y()<miny) miny = p.y();
    if(p.y()>maxy) maxy = p.y();
}

QRectF DrawingPolygonItem::boundingRect() const
{
    //QPointF pMin = Coordinate::MapToScene(QPointF(minx, miny));
    //QPointF pMax = Coordinate::MapToScene(QPointF(maxx, maxy));
    //return QRectF(pMin.x(), pMin.y(), pMax.x()-pMin.x(), pMax.y()-pMin.y());
    return QRectF(minx, miny, maxx-minx, maxy-miny);
}

void DrawingPolygonItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // draw the forming polygon
    // size is not very large, <100 for a not crazily complex polygon
    if (points.size() > 1) {
        QPen p = painter->pen();
        QBrush b = painter->brush();

        QPen pnew(QPen(Qt::black, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        pnew.setCosmetic(true);
        painter->setPen(pnew);
        QBrush bnew(QBrush(color, Qt::SolidPattern));
        painter->setBrush(bnew);

        QPainterPath path;
        path.moveTo((points.first()));
        for (int i = 1; i < points.size(); ++i){
            path.lineTo((points.at(i)));
        }
        painter->drawPath(path);
        painter->setPen(p);
        painter->setBrush(b);
    }

    painter->drawRect(boundingRect());
}




