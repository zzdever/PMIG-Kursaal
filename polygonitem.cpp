#include "polygonitem.h"

#include <QtWidgets>

#include "utils.h"

#include "p2dengine/general/p2dparams.h"


PolygonItem::PolygonItem(QColor color)
{
    this->color = color;
    setZValue(0);

    setFlags(ItemIsSelectable | ItemIsMovable);
    setAcceptHoverEvents(true);

    timer.Reset();

/*
    qDebug()<<"input size"<<points.size();
    p2DPolygonObject = new P2DPolygonObject;
    P2DVec2 pts[P2D_MAX_POLYGON_VERTICES];
    for(int i=0; i<points.size(); i++){
        pts[i] = P2DVec2(points.at(i).x(), points.at(i).y());
    }
    p2DPolygonObject->SetPoints((const P2DVec2*)pts, points.size());

    int count = p2DPolygonObject->GetVertexCount();
    qDebug()<<"output size"<<count;
    path.moveTo(QPointF(p2DPolygonObject->GetVertex(0).x,
                        p2DPolygonObject->GetVertex(0).y));
    for (int i = 1; i < count; ++i)
        path.lineTo(QPointF(p2DPolygonObject->GetVertex(i).x,
                            p2DPolygonObject->GetVertex(i).y));

    transform = new P2DTransform();
    transform->SetIdentity();
    aabb = new P2DAABB();
    p2DPolygonObject->ComputeAABB(aabb, *transform);
*/
}

PolygonItem::~PolygonItem()
{
    // delete p2DPolygonObject;
    // delete transform;
    if(aabb) delete aabb;
}

QRectF PolygonItem::boundingRect() const
{
    /*
    int minx = aabb->lowerBound.x;
    int miny = aabb->lowerBound.y;
    int maxx = aabb->upperBound.x;
    int maxy = aabb->upperBound.y;
    return QRectF(minx, miny, maxx-minx, maxy-miny);
    */

    QPointF pMin = CoordinateInterface::MapToScene(P2DVec2(aabb->lowerBound.x, aabb->lowerBound.y));
    QPointF pMax = CoordinateInterface::MapToScene(P2DVec2(aabb->upperBound.x, aabb->upperBound.y));
    return QRectF(pMin.x(), pMin.y(), pMax.x()-pMin.x(), pMax.y()-pMin.y());

}

QPainterPath PolygonItem::shape() const
{    
    return path;
}

void PolygonItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // int count = p2DPolygonObject->GetVertexCount();
    // TODO Do we need to check the count again?
    // I think this is not necessary but not sure there is no problem.

    if((int)timer.GetMilliseconds() % (500)==0){
        qDebug()<<"this position"<<this->pos();
        qDebug()<<"get position"<<CoordinateInterface::MapToScene(body->GetPosition());
    }

    QColor c = (option->state & QStyle::State_MouseOver && P2D_STATIC_BODY != body->GetType()) ?
                QColor(color.red(),color.green(),color.blue(),70) : this->color;

    this->setRotation(CoordinateInterface::RadToDeg(body->GetAngle()));
    //qDebug()<<"paint position"<<CoordinateInterface::MapToScene(body->GetPosition());
    //qDebug()<<"setpos"<<CoordinateInterface::MapToScene(P2DMul(body->GetTransform(), body->GetPosition()));
    this->setPos(CoordinateInterface::MapToScene(body->GetPosition()));

    /*
    this->setTransform(QTransform(cos(angle), sin(angle), position.x,
                                  -sin(angle), cos(angle), position.y,
                                  0,           0,          1.0), false);
                                  */

    QPen p = painter->pen();
    QBrush b = painter->brush();

    //if (count > 1) {
    {
        //QPen pnew(QPen(c, 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        QPen pnew(QPen(QColor(0,0,0), 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        //pnew.setCosmetic(true);
        painter->setPen(pnew);
        QBrush bnew(QBrush(c, Qt::SolidPattern));
        painter->setBrush(bnew);

        painter->drawPath(path);
        painter->setPen(p);
        painter->setBrush(b);

        painter->drawRect(boundingRect());
    }   
}

void PolygonItem::BindP2DBody(P2DScene* scene, QVector<QPointF> points, P2DBodyType bodyType)
{
    // Define the polygon shape for our dynamic body.
    P2DPolygonObject polygonObject;
    P2DVec2 pts[P2D_MAX_POLYGON_VERTICES];
    int32 count = points.size();
    assert(count<P2D_MAX_POLYGON_VERTICES);

    // Compute the centroid of the points in scene coordinate.
    // Note there is a little complexity here. We need to SetPoints first to sort the
    // points in the order of the convex hull. Otherwise the area may be negative if
    // the points are in the opposite order of the convex hull.
    for(int i=0; i<count; i++){
        pts[i] = P2DVec2((float32)points.at(i).x(), (float32)points.at(i).y());
    }
    polygonObject.SetPoints(pts, count);
    // Note here centroid is in scene coordinate.
    P2DVec2 centroid = polygonObject.GetCentroid(polygonObject.GetVertices(), polygonObject.GetVertexCount());
    this->setPos(QPointF(centroid.x, centroid.y));
qDebug()<<"centroid"<<centroid.x<<" "<<centroid.y;

qDebug()<<"input size"<<count;
    // Now we set the points. The polygon is draw in its local coordinate.
    for(int i=0; i<count; i++){
        pts[i] = CoordinateInterface::MapToEngine(points.at(i) - QPointF(centroid.x, centroid.y));
    }
    polygonObject.SetPoints(pts, count);
    count = polygonObject.GetVertexCount();
qDebug()<<"output size"<<count;

    // Precompute the AABB
    P2DTransform transform;
    transform.SetIdentity();
    //transform.Set(centroid, 0);
    aabb = new P2DAABB();
    polygonObject.ComputeAABB(aabb, transform);
qDebug()<<"width"<<(-aabb->lowerBound.x+aabb->upperBound.x);
qDebug()<<"height"<<(-aabb->lowerBound.y+aabb->upperBound.y);
qDebug()<<"bounding area"<<(aabb->lowerBound.x - aabb->upperBound.x)*(aabb->lowerBound.y-aabb->upperBound.y);

    // Generate the path.
    // Note this is in local coordinate, with origin at centroid.
    path.moveTo(CoordinateInterface::MapToScene(polygonObject.GetVertex(0)));
    for (int i = 1; i < count; ++i)
        path.lineTo(CoordinateInterface::MapToScene(polygonObject.GetVertex(i)));



    // Define the dynamic body. We set its position and call the body factory.
    P2DBodyDef bodyDef;
    bodyDef.type = bodyType;
    P2DVec2 c = CoordinateInterface::MapToEngine(QPointF(centroid.x, centroid.y));
    bodyDef.position.Set(c.x, c.y);
    body = scene->CreateBody(&bodyDef);


    // Define the dynamic body fixture.
    P2DFixtureDef fixtureDef;
    fixtureDef.shape = &polygonObject;
    // Set the box density to be non-zero, so it will be dynamic.
    if(bodyType == P2DBodyType::P2D_DYNAMIC_BODY)
        fixtureDef.density = 1.0f;
    // Override the default friction.
    fixtureDef.friction = 0.9f;

    // Add the shape to the body.
    body->CreateFixture(&fixtureDef);


/*
    qDebug()<<"get position 1"<<CoordinateInterface::MapToScene(this->GetP2DBody()->GetPosition());
    P2DTransform xf;
    xf.Set(P2DVec2(1,1),0);

    xf = P2DMul(xf, body->GetTransform());
    body->SetTransform(xf.position, 0);
    qDebug()<<"get position 2"<<CoordinateInterface::MapToScene(this->GetP2DBody()->GetPosition());
    */
}

void PolygonItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(P2D_STATIC_BODY != body->GetType())
        body->SetActive(false);

    QGraphicsItem::mousePressEvent(event);
    update();
}

void PolygonItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(P2D_DYNAMIC_BODY != body->GetType()) {
        event->accept();
        return;
    }

    // Note event->pos() returns the mouse cursor position in item coordinates.
    body->SetTransform(CoordinateInterface::MapToEngine(QGraphicsItem::mapToScene(event->pos())), body->GetAngle());

    //qDebug()<<"should be at"<<CoordinateInterface::MapToEngine(event->pos()).x<<CoordinateInterface::MapToEngine(event->pos()).y;
    //qDebug()<<"actual at"<<body->GetPosition().x<<body->GetPosition().y;

    /*
    P2DTransform xf;
    xf.Set(CoordinateInterface::MapToEngine(event->pos())
           - CoordinateInterface::MapToEngine(event->lastPos()), 0);
    xf = P2DMul(xf, body->GetTransform());
    body->SetTransform(xf.position, xf.rotation.GetAngle());
    */

    QGraphicsItem::mouseMoveEvent(event);
    update();
}

void PolygonItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    body->SetActive(true);

    QGraphicsItem::mouseReleaseEvent(event);
    update();
}

void PolygonItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    QAction *changeColorAction = menu.addAction("Change Color");
    menu.addSeparator();
    QAction *liquifyAction = menu.addAction("Liquify");

    QAction *selectedAction = menu.exec(event->screenPos());
    if(selectedAction == changeColorAction){
        QColorDialog cDialog;
        cDialog.exec();
        this->color = cDialog.selectedColor();
    } else if(selectedAction == liquifyAction){
        qDebug()<<"liquify";
    }
}
