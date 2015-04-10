#include "scenemanager.h"

SceneManager::SceneManager()
{
    isDrawing = false;

    QColor color(100,100,100);
    item = new Chip(color, 0, 0);
    item->setPos(QPointF(0, 0));
    addItem(item);

    /*
    item = new Chip(color, 0, 0);
    item->setPos(QPointF(5000, 0));
    addItem(item);

    item = new Chip(color, 0, 0);
    item->setPos(QPointF(-5000, 0));
    addItem(item);

    item = new Chip(color, 0, 0);
    item->setPos(QPointF(0, 5000));
    addItem(item);

    item = new Chip(color, 0, 0);
    item->setPos(QPointF(0, -5000));
    addItem(item);
    */

    tmp=0;
}


void SceneManager::Render()
{
    // Physical engine solve here
    //tmp++;
    //item->setPos(QPointF(0, tmp%100));
    return;


    QImage image("/Users/ying/Desktop/aa.png");

    // Populate scene
    int xx = 0;
    int nitems = 0;
    for (int i = -5000; i < 5000; i += 110) {
        ++xx;
        int yy = 0;
        for (int j = -7000; j < 7000; j += 70) {
            ++yy;
            qreal x = (i + 11000) / 22000.0;
            qreal y = (j + 7000) / 14000.0;

            QColor color(image.pixel(int(image.width() * x), int(image.height() * y)));
            QGraphicsItem *item = new Chip(color, xx, yy);
            item->setPos(QPointF(i, j));
            addItem(item);

            ++nitems;
        }
    }
}

void SceneManager::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // We can only draw in a blank area.
    if(itemAt(event->scenePos(), QTransform()) == nullptr){
        isDrawing = true;
        drawingItem = new DrawingPolygon(QColor(qrand()%255, qrand()%255, qrand()%255), event->scenePos());
        //drawingItem->setPos(event->scenePos());
        addItem(drawingItem);
    }

    QGraphicsScene::mousePressEvent(event);
}


void SceneManager::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(isDrawing){
        drawingItem->AddPoint(event->scenePos());
        update();
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
    this->color = color;

    // This should be maximized, put it on the top of everything.
    setZValue(100);
}

void DrawingPolygon::AddPoint(QPointF p)
{
    points.push_back(p);
}

QRectF DrawingPolygon::boundingRect() const
{
    return QRectF(0, 0, 0, 0);
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
        painter->setPen(QPen(Qt::black, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->setBrush(QBrush(color, Qt::Dense7Pattern));
        QPainterPath path;
        path.moveTo(points.first());
        for (int i = 1; i < points.size(); ++i)
            path.lineTo(points.at(i));
        painter->drawPath(path);
        painter->setPen(p);
        painter->setBrush(b);
    }
}




