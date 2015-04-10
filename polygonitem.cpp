#include "polygonitem.h"

#include <QtWidgets>

PolygonItem::PolygonItem(const QColor &color, QVector<QPointF> points)
{
    p2DPolygonObject = new P2DPolygonObject;
    P2DVec2* pts[P2D_MAX_POLYGON_VERTICES];
    for(int i=0; i<points.size(); i++){
        pts[i] = new P2DVec2(points.at(i).x(), points.at(i).y());
    }
    p2DPolygonObject->SetPoints(pts, points.size());

    this->color = color;
    setZValue(0);

    setFlags(ItemIsSelectable | ItemIsMovable);
    setAcceptHoverEvents(true);
}

QRectF PolygonItem::boundingRect() const
{
    return QRectF(0, 0, 110, 70);
}

QPainterPath PolygonItem::shape() const
{
    QPainterPath path;
    path.addRect(14, 14, 82, 42);
    return path;
}

void PolygonItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

    int count = p2DPolygonObject->GetVertexCount();

    if (count > 1) {
        QPen p = painter->pen();
        QBrush b = painter->brush();

        QPen pnew(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        //pnew.setCosmetic(true);
        painter->setPen(pnew);
        QBrush bnew(QBrush(color.lighter(70), Qt::SolidPattern));
        painter->setBrush(bnew);

        QPainterPath path;

        path.moveTo((points.first()));
        for (int i = 1; i < points.size(); ++i)
            path.lineTo((points.at(i)));
        painter->drawPath(path);
        painter->setPen(p);
        painter->setBrush(b);
    }
}

void PolygonItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
    update();
}

void PolygonItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->modifiers() & Qt::ShiftModifier) {
        stuff << event->pos();
        update();
        return;
    }
    QGraphicsItem::mouseMoveEvent(event);
}

void PolygonItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    update();
}
