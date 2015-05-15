#include "polygonitem.h"

#include <QtWidgets>
#include "p2dengine/general/p2dparams.h"

PolygonItem::PolygonItem(QColor color, QVector<QPointF> points)
{
    this->color = color;
    setZValue(0);

    setFlags(ItemIsSelectable | ItemIsMovable);
    setAcceptHoverEvents(true);


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
}

PolygonItem::~PolygonItem()
{
    delete p2DPolygonObject;
    delete transform;
    delete aabb;
}

QRectF PolygonItem::boundingRect() const
{
    int minx = aabb->lowerBound.x;
    int miny = aabb->lowerBound.y;
    int maxx = aabb->upperBound.x;
    int maxy = aabb->upperBound.y;
    return QRectF(minx, miny, maxx-minx, maxy-miny);
}

QPainterPath PolygonItem::shape() const
{    
    return path;
}

void PolygonItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    int count = p2DPolygonObject->GetVertexCount();

    QColor c = (option->state & QStyle::State_MouseOver) ? QColor(color.red(),color.green(),color.blue(),70) : this->color;

    if (count > 1) {
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
