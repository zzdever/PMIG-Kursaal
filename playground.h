#ifndef PLAYGROUND_H
#define PLAYGROUND_H

#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QTouchEvent>


/*
class GraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    GraphicsView(PlayGround *v) : QGraphicsView(), view(v) {
        setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        viewport()->setAttribute(Qt::WA_AcceptTouchEvents);
    }

protected:

    //bool viewportEvent(QEvent *event);

};
*/

class MyGraphicsScene : public QGraphicsScene{
  protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
};



class PlayGround : public QGraphicsView
{
    Q_OBJECT
public:
    explicit PlayGround(const QString &name, QWidget *parent = 0);

protected:
#ifndef QT_NO_WHEELEVENT
    void wheelEvent(QWheelEvent *);
#endif
    //void mouseMoveEvent(QMouseEvent *event);
    //void mousePressEvent(QMouseEvent *event);

public slots:
    void zoomIn(int level = 1);
    void zoomOut(int level = 1);

private slots:
    void resetView();
    void setupMatrix();
    void print();

private:
    int zoom=250;
};



#include <QColor>
#include <QGraphicsItem>

class Chip : public QGraphicsItem
{
public:
    Chip(const QColor &color, int x, int y);

    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    int x;
    int y;
    QColor color;
    QVector<QPointF> stuff;
};


#endif // PLAYGROUND_H
