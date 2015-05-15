#include "polygonitem.h"

#include <QtWidgets>
#include "p2dengine/general/p2dparams.h"

namespace Coordinate {

QPointF MapToEngine(QPointF pIn)
{//qDebug()<<"MapToEngine input"<<pIn;
    qreal x = (pIn.x()-(-SCENE_WIDTH_HALF)) / (SCENE_WIDTH_HALF*2)
            * (ENGINE_SCENE_MAXIMUM_HALF*2)
            + (-ENGINE_SCENE_MAXIMUM_HALF);

    qreal y = (pIn.y()-(-SCENE_HEIGHT_HALF)) / (SCENE_HEIGHT_HALF*2)
            * (ENGINE_SCENE_MAXIMUM_HALF*2)
            + (-ENGINE_SCENE_MAXIMUM_HALF);

    QPointF pOut(x,y);

    return pOut;
}

QPointF MapToScene(QPointF pIn)
{//qDebug()<<"MapToScene input"<<pIn;
    qreal x = (pIn.x()-(-ENGINE_SCENE_MAXIMUM_HALF)) / (ENGINE_SCENE_MAXIMUM_HALF*2)
            * (SCENE_WIDTH_HALF*2)
            + (-SCENE_WIDTH_HALF);

    qreal y = (pIn.y()-(-ENGINE_SCENE_MAXIMUM_HALF)) / (ENGINE_SCENE_MAXIMUM_HALF*2)
            * (SCENE_HEIGHT_HALF*2)
            + (-SCENE_HEIGHT_HALF);

    QPointF pOut(x,y);

    return pOut;
}
}



PolygonItem::PolygonItem(QColor color)
{
    this->color = color;
    setZValue(0);

    setFlags(ItemIsSelectable | ItemIsMovable);
    setAcceptHoverEvents(true);

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

    QPointF pMin = Coordinate::MapToScene(QPointF(aabb->lowerBound.x, aabb->lowerBound.y));
    QPointF pMax = Coordinate::MapToScene(QPointF(aabb->upperBound.x, aabb->upperBound.y));
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

    //int count = p2DPolygonObject->GetVertexCount();
    // TODO Do we need to check the count again?
    // I think this is not necessary but not sure there is no problem.

    QColor c = (option->state & QStyle::State_MouseOver) ? QColor(color.red(),color.green(),color.blue(),70) : this->color;

    //if (count > 1) {
    if (1) {
        QPen p = painter->pen();
        QBrush b = painter->brush();

        QPen pnew(QPen(c, 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        //pnew.setCosmetic(true);
        painter->setPen(pnew);
        QBrush bnew(QBrush(c, Qt::SolidPattern));
        painter->setBrush(bnew);

        painter->drawPath(path);
        painter->setPen(p);
        painter->setBrush(b);
    }

    painter->drawRect(boundingRect());
}

void PolygonItem::BindP2DBody(P2DScene* scene, QVector<QPointF> points)
{
    // Define the polygon shape for our dynamic body.
    P2DPolygonObject polygonObject;
    P2DVec2 pts[P2D_MAX_POLYGON_VERTICES];
    int32 count = points.size();

    // Compute the centroid of the points.
    for(int i=0; i<count; i++){
        pts[i] = P2DVec2((float32)points.at(i).x(), (float32)points.at(i).y());
    }
    P2DVec2 centroid = polygonObject.GetCentroid(pts, count);

qDebug()<<"input size"<<count;
    QPointF pTmp;
    for(int i=0; i<count; i++){
        pTmp = Coordinate::MapToEngine(points.at(i) - QPointF(centroid.x, centroid.y));
        //pts[i] = P2DVec2((float32)points.at(i).x(), (float32)points.at(i).y());
        pts[i] = P2DVec2((float32)pTmp.x(), (float32)pTmp.y());
    }
    polygonObject.SetPoints(pts, count);
    count = polygonObject.GetVertexCount();
qDebug()<<"output size"<<count;

    // Precompute the AABB
    P2DTransform transform;
    transform.SetIdentity();
    aabb = new P2DAABB();
    polygonObject.ComputeAABB(aabb, transform);
qDebug()<<"width"<<(aabb->lowerBound.x - aabb->upperBound.x);
qDebug()<<"height"<<(aabb->lowerBound.y-aabb->upperBound.y);
qDebug()<<"bounding area"<<(aabb->lowerBound.x - aabb->upperBound.x)*(aabb->lowerBound.y-aabb->upperBound.y);

    // Generate the path.
    // Note this is in local coordinate, with origin at centroid.
    path.moveTo(Coordinate::MapToScene(QPointF(polygonObject.GetVertex(0).x,
                        polygonObject.GetVertex(0).y)));
    for (int i = 1; i < count; ++i)
        path.lineTo(Coordinate::MapToScene(QPointF(polygonObject.GetVertex(i).x,
                            polygonObject.GetVertex(i).y)));



    // Define the dynamic body. We set its position and call the body factory.
    P2DBodyDef bodyDef;
    bodyDef.type = P2D_DYNAMIC_BODY;
    bodyDef.position.Set(centroid.x, centroid.y);
    body = scene->CreateBody(&bodyDef);


    // Define the dynamic body fixture.
    P2DFixtureDef fixtureDef;
    fixtureDef.shape = &polygonObject;
    // Set the box density to be non-zero, so it will be dynamic.
    fixtureDef.density = 1.0f;
    // Override the default friction.
    fixtureDef.friction = 0.3f;

    // Add the shape to the body.
    body->CreateFixture(&fixtureDef);
}

void PolygonItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
    update();
}

void PolygonItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug()<<this->sceneTransform();
    QGraphicsItem::mouseMoveEvent(event);
    update();
}

void PolygonItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
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
