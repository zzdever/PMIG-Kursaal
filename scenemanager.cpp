#include "scenemanager.h"

#include "utils.h"

SceneManager::SceneManager()
{
    isDrawing = false;
    isEngineRunning = true;

    InitP2DEngine();

    /*
    QColor color(50,50,50);
    item = new Chip(color, 0, 0);
    item->setPos(QPointF(0, 0));
    addItem(item);
    */
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
    if(isEngineRunning && this->isActive())
        scene->Step(timeStep, velocityIterations, positionIterations);


#define DEBUG 0
#if DEBUG
    int i=0;
    for(P2DBody* bodyList = scene->GetBodyList();
        bodyList; bodyList = bodyList->GetNext(), i++)
    {
        const P2DTransform& xf = bodyList->GetTransform();
        qDebug()<<"body "<<i
               <<" position "<<CoordinateInterface::MapToScene(bodyList->GetPosition())
              <<" angle "<<bodyList->GetAngle();

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
#endif



    // Refresh the viewport
    update();

    return;
}

void SceneManager::toggleEngine()
{
    isEngineRunning = ! isEngineRunning;
}

void SceneManager::InitP2DEngine()
{
    // Prepare for simulation. Typically we use a time step of 1/60 of a
    // second (60Hz) and 10 iterations. This provides a high quality simulation
    // in most game scenarios.
    timeStep = 1.0f / 60.0f;
    velocityIterations = 6;
    positionIterations = 2;


    // Define the gravity vector.
    P2DVec2 gravity(0.0f, 10.0f);
    // Construct a world object, which will hold and simulate the rigid bodies.
    scene = new P2DScene(gravity);

    LoadGround();

    /*
    // Define the ground body.
    P2DBodyDef groundBodyDef;
    groundBodyDef.position.Set(0.0f, 0.0f);
    // Call the body factory which allocates memory for the ground body
    // from a pool and creates the ground box shape (also from a pool).
    // The body is also added to the world.
    P2DBody* groundBody = scene->CreateBody(&groundBodyDef);

    // Define the ground box shape.
    P2DPolygonObject groundBox;
    groundBox.SetARect(10.0f, 1.0f);

    // Add the ground fixture to the ground body.
    groundBody->CreateFixture(&groundBox, 0.0f);
    */



    /*
    // add a bunch of particles
    const int len = 7, step = len*2+5;
    qDebug()<< "len" << len;
    for(int i=-10*step;i<10*step;i+=step)
        for(int j=-10*step;j<10*step;j+=step)
    {
        polyItem = new PolygonItem(QColor(10,10,240), this);
        QVector<QPointF> points;
        points.push_back(QPointF(len+i,len+j));
        points.push_back(QPointF(-len+i,len+j));
        points.push_back(QPointF(-len+i,-len+j));
        points.push_back(QPointF(len+i,-len+j));
        polyItem->BindP2DBody(scene, points, P2D_DYNAMIC_BODY);
        addItem(polyItem);
    }
    */
}

void SceneManager::ClearScene()
{
    QList<QGraphicsItem *> allItems;
    allItems = this->items();
    foreach(QGraphicsItem* item, allItems){
        //if(static_cast<PolygonItem*>(item)->GetP2DBody()->GetType() != P2D_STATIC_BODY){
        {
            removeItem(item);
            if(item) delete item;
        }
    }
    return;
}

void SceneManager::contextMenuEvent(QGraphicsSceneContextMenuEvent* e)
{
    if(itemAt(e->scenePos(), QTransform()) == NULL){
        QMenu menu;
        QAction *clearAction = menu.addAction("Clear Scene");

        QAction *selectedAction = menu.exec(e->screenPos());
        if(selectedAction == clearAction){
            ClearScene();
        } else {}
    }

    QGraphicsScene::contextMenuEvent(e);
}

void SceneManager::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Space)
        toggleEngine();
}

void SceneManager::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // We can only draw in a blank area.
    if(event->button() == Qt::LeftButton && itemAt(event->scenePos(), QTransform()) == NULL){
        isDrawing = true;
        drawingItem = new DrawingPolygonItem(QColor(qrand()%255, qrand()%255, qrand()%255), event->scenePos());
        addItem(drawingItem);
    }

    QGraphicsScene::mousePressEvent(event);
}


void SceneManager::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(isDrawing){
        drawingItem->AddPoint(event->scenePos());
    }

    QGraphicsScene::mouseMoveEvent(event);
}


void SceneManager::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if(isDrawing){
        // TODO add to the object tree
        if(drawingItem->GetPoints().size() >= 3){
            polyItem = new PolygonItem(drawingItem->GetColor(), this);
            polyItem->BindP2DBody(scene, drawingItem->GetPoints());
            addItem(polyItem);
        }

        isDrawing = false;
        removeItem(drawingItem);
        delete drawingItem;
    }

    QGraphicsScene::mouseReleaseEvent(event);
}


void SceneManager::LoadGround(void)
{
    ClearScene();
    // Add the ground body.
    polyItem = new PolygonItem(QColor(21,25,123), this);
    QVector<QPointF> points;
    points.push_back(QPointF(-SCENE_WIDTH_HALF*3, SCENE_HEIGHT_HALF*4/5));
    points.push_back(QPointF(SCENE_WIDTH_HALF*3, SCENE_HEIGHT_HALF*4/5));
    points.push_back(QPointF(SCENE_WIDTH_HALF*3, SCENE_HEIGHT_HALF*5/5));
    points.push_back(QPointF(-SCENE_WIDTH_HALF*3, SCENE_HEIGHT_HALF*5/5));
    polyItem->BindP2DBody(scene, points, P2D_STATIC_BODY);
    addItem(polyItem);
}

void SceneManager::LoadBouncingBall(void)
{
    LoadGround();

    // Add balls.
    for(int j=0; j<10; j++){
        polyItem = new PolygonItem(QColor(qrand()%255, qrand()%255, qrand()%255), this);
        QVector<QPointF> points;
        const int segments = 20;
        for(int i=0; i < segments; i++){
            points.push_back(QPointF(50*cos(360*i/segments) + (j-5)*120, 50*sin(360*i/segments) - 200));
        }
        polyItem->BindP2DBody(scene, points, P2D_DYNAMIC_BODY, 1.0*j/10);
        addItem(polyItem);
    }

}

void SceneManager::LoadDomino(void)
{
    LoadGround();

    // Add dominoes.
    const int width = 40, height = 300;
    int xOffset = -SCENE_WIDTH_HALF*3;
    for(int i=0; i<50; i++){
        xOffset += width*3;
        polyItem = new PolygonItem(QColor(qrand()%255, qrand()%255, qrand()%255), this);
        QVector<QPointF> points;
        points.push_back(QPointF(width/2 + xOffset, SCENE_HEIGHT_HALF*4/5));
        points.push_back(QPointF(width/2 + xOffset, SCENE_HEIGHT_HALF*4/5 - height));
        points.push_back(QPointF(-width/2 + xOffset, SCENE_HEIGHT_HALF*4/5 - height));
        points.push_back(QPointF(-width/2 + xOffset, SCENE_HEIGHT_HALF*4/5));
        polyItem->BindP2DBody(scene, points, P2D_DYNAMIC_BODY,0.5,0.1);
        addItem(polyItem);
    }
}

void SceneManager::LoadSlide(void)
{
    LoadGround();

    // Add a box.
    polyItem = new PolygonItem(QColor(qrand()%255, qrand()%255, qrand()%255), this);
    QVector<QPointF> points;
    QRect rec(0,0, 100, 100);
    points.push_back(rec.bottomLeft());
    points.push_back(rec.bottomRight());
    points.push_back(rec.topRight());
    points.push_back(rec.topLeft());
    polyItem->BindP2DBody(scene, points, P2D_DYNAMIC_BODY,0.1,0.5);
    polyItem->Rotate(30);
    polyItem->Translate(QPointF(-300, -300));
    addItem(polyItem);


    //QVector<QRect> rects;
    rec = (QRect(-300,0, 600, 30));
    QVector<double> rotations;
    rotations.push_back(30);
    rotations.push_back(-30);
    rotations.push_back(30);

    QVector<float> frictions;
    frictions.push_back(0.1);
    frictions.push_back(0.5);
    frictions.push_back(0.9);

    QVector<QPointF> translations;
    translations.push_back(QPointF(-150,0));
    translations.push_back(QPointF(150,300));
    translations.push_back(QPointF(-150,600));

    for(int i=0; i<rotations.size(); i++){
        polyItem = new PolygonItem(QColor(qrand()%255, qrand()%255, qrand()%255), this);
        QVector<QPointF> points;
        //rec = rects.at(i);
        points.push_back(rec.bottomLeft());
        points.push_back(rec.bottomRight());
        points.push_back(rec.topRight());
        points.push_back(rec.topLeft());
        polyItem->BindP2DBody(scene, points, P2D_STATIC_BODY, 0.1, frictions.at(i));
        polyItem->Rotate(rotations.at(i));
        polyItem->Translate(translations.at(i));
        addItem(polyItem);
    }

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




