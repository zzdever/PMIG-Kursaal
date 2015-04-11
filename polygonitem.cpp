#include "polygonitem.h"

#include <QtWidgets>
#include "p2dengine/general/p2dparams.h"

PolygonItem::PolygonItem(QColor color, QVector<QPointF> points)
{
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

    this->color = color;
    setZValue(0);

    setFlags(ItemIsSelectable | ItemIsMovable);
    setAcceptHoverEvents(true);
}

PolygonItem::~PolygonItem()
{
    delete p2DPolygonObject;
}

QRectF PolygonItem::boundingRect() const
{
    return QRectF(0, 0, 200, 200);
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

    QColor c = (option->state & QStyle::State_MouseOver) ? Qt::black : this->color;

    if (count > 1) {
        QPen p = painter->pen();
        QBrush b = painter->brush();

        QPen pnew(QPen(c, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        //pnew.setCosmetic(true);
        painter->setPen(pnew);
        QBrush bnew(QBrush(c, Qt::SolidPattern));
        painter->setBrush(bnew);

        painter->drawPath(path);
        painter->setPen(p);
        painter->setBrush(b);
    }
}

void PolygonItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug()<<"pressed";
    QGraphicsItem::mousePressEvent(event);
    update();
}

void PolygonItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseMoveEvent(event);
    update();
}

void PolygonItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    update();
}
