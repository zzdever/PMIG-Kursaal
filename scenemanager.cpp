#include "scenemanager.h"

SceneManager::SceneManager()
{
    isDrawing = false;

    QColor color(100,100,100);
    item = new Chip(color, 0, 0);
    item->setPos(QPointF(0, 0));
    addItem(item);

    tmp=0;
}


void SceneManager::Render()
{
    // Physical engine solve here
    //tmp++;
    //item->setPos(QPointF(0, tmp%100));
    update();

    return;
}

void SceneManager::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // We can only draw in a blank area.
    if(event->button() == Qt::LeftButton && itemAt(event->scenePos(), QTransform()) == nullptr){
        isDrawing = true;
        drawingItem = new DrawingPolygon(QColor(qrand()%255, qrand()%255, qrand()%255), event->scenePos());
        addItem(drawingItem);
    }

    QGraphicsScene::mousePressEvent(event);
}


void SceneManager::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(isDrawing){
        drawingItem->AddPoint(event->scenePos());
        //update();
    }

    QGraphicsScene::mouseMoveEvent(event);
}


void SceneManager::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if(isDrawing){
        isDrawing = false;
        removeItem(drawingItem);

        // TODO add to the object tree
    }

    QGraphicsScene::mouseReleaseEvent(event);
}




DrawingPolygon::DrawingPolygon(QColor color, QPointF p)
{
    points.push_back(p);
    color.setAlpha(49);
    this->color = color;

    // This should be maximized, put it on the top of everything.
    setZValue(100);

    //setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
}

void DrawingPolygon::AddPoint(QPointF p)
{
    points.push_back(p);
}

QRectF DrawingPolygon::boundingRect() const
{
    return QRectF(0, 0, 100, 100);
}

void DrawingPolygon::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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
        for (int i = 1; i < points.size(); ++i)
            path.lineTo((points.at(i)));
        painter->drawPath(path);
        painter->setPen(p);
        painter->setBrush(b);
    }
}




