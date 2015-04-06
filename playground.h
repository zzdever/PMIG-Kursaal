#ifndef PLAYGROUND_H
#define PLAYGROUND_H

#include <QGraphicsView>
#include <QTouchEvent>

#include "scenemanager.h"

#define ZOOM_STEP 0.3
#define ZOOM_PARAM 0.7
#define TRANS_STEP 3
#define TRANS_PARAM 0.7

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


class PlayGround : public QGraphicsView
{
    Q_OBJECT
public:
    explicit PlayGround(const QString &name, QWidget *parent = 0);
    void updateView(){
        setupMatrix();
    }

protected:
#ifndef QT_NO_WHEELEVENT
    void wheelEvent(QWheelEvent *);
#endif
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    //void paintEvent(QPaintEvent *event);

private slots:
    void resetView();
    void setupMatrix();
    void print();

private:
    double zoomAmount, toZoomAmount;
    QPointF toTranslation;
    QPoint lastMousePressPos;
    bool isDragging;
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
