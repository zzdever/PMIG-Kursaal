#ifndef PLAYGROUND_H
#define PLAYGROUND_H

#include <QFrame>
#include <QGraphicsView>
#include <QTouchEvent>

QT_BEGIN_NAMESPACE
class QLabel;
class QSlider;
class QToolButton;
QT_END_NAMESPACE

class PlayGround;

class GraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    GraphicsView(PlayGround *v) : QGraphicsView(), view(v) {
        setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

        viewport()->setAttribute(Qt::WA_AcceptTouchEvents);
    }

protected:
#ifndef QT_NO_WHEELEVENT
    void wheelEvent(QWheelEvent *);
#endif
    //bool viewportEvent(QEvent *event);


private:
    PlayGround *view;

    double totalScaleFactor = 1;
};

class PlayGround : public QFrame
{
    Q_OBJECT
public:
    explicit PlayGround(const QString &name, QWidget *parent = 0);

    QGraphicsView *view() const;

public slots:
    void zoomIn(int level = 1);
    void zoomOut(int level = 1);

private slots:
    void resetView();
    void setResetButtonEnabled();
    void setupMatrix();
    void togglePointerMode();
    void toggleOpenGL();
    void toggleAntialiasing();
    void print();
    void rotateLeft();
    void rotateRight();

private:
    GraphicsView *graphicsView;
    QLabel *label;
    QLabel *label2;
    QToolButton *selectModeButton;
    QToolButton *dragModeButton;
    QToolButton *openGlButton;
    QToolButton *antialiasButton;
    QToolButton *printButton;
    QToolButton *resetButton;
    QSlider *zoomSlider;
    QSlider *rotateSlider;

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
