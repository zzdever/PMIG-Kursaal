#include "scenemanager.h"

SceneManager::SceneManager()
{
    isDrawing = false;

    QColor color(100,100,100);
    item = new Chip(color, 0, 0);
    item->setPos(QPointF(0, 0));
    addItem(item);
}


void SceneManager::Render()
{
    // Physical engine solve here
    update();

    return;
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
    if(isDrawing){
        drawingItem->AddPoint(event->scenePos());
    }

    QGraphicsScene::mouseMoveEvent(event);
}


void SceneManager::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if(isDrawing){
        // TODO add to the object tree
        polyItem = new PolygonItem(drawingItem->GetColor(), drawingItem->GetPoints());
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
    int size = points.size();
    // we do not allow the second point is too close to the first one
    if(size==1 && QVector2D(p - points.at(size - 1)).length()<FLT_EPSILON)
        return;

    bool pop = false;
    if(size >= 2){
        // the last vector
        QVector2D v0 = QVector2D(points.at(size - 1) - points.at(size - 2));
        // the current vector
        QVector2D v1 = QVector2D(p - points.at(size - 1));

        qreal crossProduct = v0.x()*v1.y() - v1.x()*v0.y();
        qreal sinV = crossProduct/(v0.length()*v1.length());
        if(qAbs(sinV) < sin(0.05)/*in radian*/ || v1.length()<FLT_EPSILON)
            pop = true;
    }

    if(pop)
        points.pop_back();

    points.push_back(p);
    if(p.x()<minx) minx = p.x();
    if(p.x()>maxx) maxx = p.x();
    if(p.y()<miny) miny = p.y();
    if(p.y()>maxy) maxy = p.y();
}

QRectF DrawingPolygonItem::boundingRect() const
{
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

    painter->drawRect(QRectF(minx, miny, maxx-minx, maxy-miny));
}




