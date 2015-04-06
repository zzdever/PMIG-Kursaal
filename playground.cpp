#include "playground.h"

#ifndef QT_NO_PRINTER
#include <QPrinter>
#include <QPrintDialog>
#endif
#ifndef QT_NO_OPENGL
#include <QtOpenGL>
#else
#include <QtWidgets>
#endif
#include <qmath.h>


//#ifndef QT_NO_WHEELEVENT
void GraphicsView::wheelEvent(QWheelEvent *e)
{
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    if(e->delta() > 0) {
        // Zoom in
        view->zoomIn(6);
    } else {
        // Zooming out
        view->zoomOut(6);
    }

    // Don't call superclass handler here
    // as wheel is normally used for moving scrollbars
    // QGraphicsView::wheelEvent(e);
}
//#endif

/*
bool GraphicsView::viewportEvent(QEvent *event)
 {
     switch (event->type()) {
     case QEvent::TouchBegin:
     case QEvent::TouchUpdate:
     case QEvent::TouchEnd:
     {
         QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
         QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();
         if (touchPoints.count() == 2) {
             // determine scale factor
             const QTouchEvent::TouchPoint &touchPoint0 = touchPoints.first();
             const QTouchEvent::TouchPoint &touchPoint1 = touchPoints.last();
             qDebug()<<touchPoint0.pos()<<touchPoint0.startPos();

             QVector2D vec1 = QVector2D(touchPoint0.pos() - touchPoint0.startPos());
             QVector2D vec2 = QVector2D(touchPoint1.pos() - touchPoint1.startPos());
             //qDebug()<<vec1.x()<<vec1.y()<<","<<vec2.x()<<vec2.y();
             if((vec1.x()*vec2.x() + vec1.y()*vec2.y()) >= 0)
                 return true;

             qreal currentScaleFactor =
                     QLineF(touchPoint0.pos(), touchPoint1.pos()).length()
                     / QLineF(touchPoint0.startPos(), touchPoint1.startPos()).length();
             if (touchEvent->touchPointStates() & Qt::TouchPointReleased) {
                 // if one of the fingers is released, remember the current scale
                 // factor so that adding another finger later will continue zooming
                 // by adding new scale factor to the existing remembered value.
                 totalScaleFactor *= currentScaleFactor;
                 currentScaleFactor = 1;
             }
             if(totalScaleFactor * currentScaleFactor < 0.1)
                 break;
             setTransform(QTransform().scale(totalScaleFactor * currentScaleFactor,
                                             totalScaleFactor * currentScaleFactor));
         }
         return true;
     }
     default:
         break;
     }
     return QGraphicsView::viewportEvent(event);
 }
*/

PlayGround::PlayGround(const QString &name, QWidget *parent)
    : QFrame(parent)
{
    Q_UNUSED(name);
    setFrameShape(NoFrame);
    graphicsView = new GraphicsView(this);
    graphicsView->setRenderHint(QPainter::Antialiasing, true);
    graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    graphicsView->setOptimizationFlags(QGraphicsView::DontSavePainterState);
    graphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    QVBoxLayout *topLayout = new QVBoxLayout;
    topLayout->addWidget(graphicsView);
    setLayout(topLayout);

    /*
    int size = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
    QSize iconSize(size, size);

    QToolButton *zoomInIcon = new QToolButton;
    zoomInIcon->setAutoRepeat(true);
    zoomInIcon->setAutoRepeatInterval(33);
    zoomInIcon->setAutoRepeatDelay(0);
    zoomInIcon->setIcon(QPixmap(":/zoomin.png"));
    zoomInIcon->setIconSize(iconSize);
    QToolButton *zoomOutIcon = new QToolButton;
    zoomOutIcon->setAutoRepeat(true);
    zoomOutIcon->setAutoRepeatInterval(33);
    zoomOutIcon->setAutoRepeatDelay(0);
    zoomOutIcon->setIcon(QPixmap(":/zoomout.png"));
    zoomOutIcon->setIconSize(iconSize);
    zoomSlider = new QSlider;
    zoomSlider->setMinimum(0);
    zoomSlider->setMaximum(500);
    zoomSlider->setValue(250);
    zoomSlider->setTickPosition(QSlider::TicksRight);

    // Zoom slider layout
    QVBoxLayout *zoomSliderLayout = new QVBoxLayout;
    zoomSliderLayout->addWidget(zoomInIcon);
    zoomSliderLayout->addWidget(zoomSlider);
    zoomSliderLayout->addWidget(zoomOutIcon);

    QToolButton *rotateLeftIcon = new QToolButton;
    rotateLeftIcon->setIcon(QPixmap(":/rotateleft.png"));
    rotateLeftIcon->setIconSize(iconSize);
    QToolButton *rotateRightIcon = new QToolButton;
    rotateRightIcon->setIcon(QPixmap(":/rotateright.png"));
    rotateRightIcon->setIconSize(iconSize);
    rotateSlider = new QSlider;
    rotateSlider->setOrientation(Qt::Horizontal);
    rotateSlider->setMinimum(-360);
    rotateSlider->setMaximum(360);
    rotateSlider->setValue(0);
    rotateSlider->setTickPosition(QSlider::TicksBelow);

    // Rotate slider layout
    QHBoxLayout *rotateSliderLayout = new QHBoxLayout;
    rotateSliderLayout->addWidget(rotateLeftIcon);
    rotateSliderLayout->addWidget(rotateSlider);
    rotateSliderLayout->addWidget(rotateRightIcon);

    resetButton = new QToolButton;
    resetButton->setText(tr("0"));
    resetButton->setEnabled(false);

    // Label layout
    QHBoxLayout *labelLayout = new QHBoxLayout;
    label = new QLabel(name);
    label2 = new QLabel(tr("Pointer Mode"));
    selectModeButton = new QToolButton;
    selectModeButton->setText(tr("Select"));
    selectModeButton->setCheckable(true);
    selectModeButton->setChecked(true);
    dragModeButton = new QToolButton;
    dragModeButton->setText(tr("Drag"));
    dragModeButton->setCheckable(true);
    dragModeButton->setChecked(false);
    antialiasButton = new QToolButton;
    antialiasButton->setText(tr("Antialiasing"));
    antialiasButton->setCheckable(true);
    antialiasButton->setChecked(false);
    openGlButton = new QToolButton;
    openGlButton->setText(tr("OpenGL"));
    openGlButton->setCheckable(true);
#ifndef QT_NO_OPENGL
    openGlButton->setEnabled(QGLFormat::hasOpenGL());
#else
    openGlButton->setEnabled(false);
#endif
    printButton = new QToolButton;
    printButton->setIcon(QIcon(QPixmap(":/fileprint.png")));

    QButtonGroup *pointerModeGroup = new QButtonGroup;
    pointerModeGroup->setExclusive(true);
    pointerModeGroup->addButton(selectModeButton);
    pointerModeGroup->addButton(dragModeButton);

    labelLayout->addWidget(label);
    labelLayout->addStretch();
    labelLayout->addWidget(label2);
    labelLayout->addWidget(selectModeButton);
    labelLayout->addWidget(dragModeButton);
    labelLayout->addStretch();
    labelLayout->addWidget(antialiasButton);
    labelLayout->addWidget(openGlButton);
    labelLayout->addWidget(printButton);

    QGridLayout *topLayout = new QGridLayout;
    topLayout->addLayout(labelLayout, 0, 0);
    topLayout->addWidget(graphicsView, 1, 0);
    topLayout->addLayout(zoomSliderLayout, 1, 1);
    topLayout->addLayout(rotateSliderLayout, 2, 0);
    topLayout->addWidget(resetButton, 2, 1);
    setLayout(topLayout);
    */

    //connect(resetButton, SIGNAL(clicked()), this, SLOT(resetView()));
    //connect(zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(setupMatrix()));
    //connect(rotateSlider, SIGNAL(valueChanged(int)), this, SLOT(setupMatrix()));

    connect(graphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)),
            this, SLOT(setResetButtonEnabled()));
    connect(graphicsView->horizontalScrollBar(), SIGNAL(valueChanged(int)),
            this, SLOT(setResetButtonEnabled()));
    //connect(selectModeButton, SIGNAL(toggled(bool)), this, SLOT(togglePointerMode()));
    //connect(dragModeButton, SIGNAL(toggled(bool)), this, SLOT(togglePointerMode()));
    //connect(antialiasButton, SIGNAL(toggled(bool)), this, SLOT(toggleAntialiasing()));
    //connect(openGlButton, SIGNAL(toggled(bool)), this, SLOT(toggleOpenGL()));
    //connect(rotateLeftIcon, SIGNAL(clicked()), this, SLOT(rotateLeft()));
    //connect(rotateRightIcon, SIGNAL(clicked()), this, SLOT(rotateRight()));
    //connect(zoomInIcon, SIGNAL(clicked()), this, SLOT(zoomIn()));
    //connect(zoomOutIcon, SIGNAL(clicked()), this, SLOT(zoomOut()));
    //connect(printButton, SIGNAL(clicked()), this, SLOT(print()));

    setupMatrix();
    toggleOpenGL();
    toggleAntialiasing();
    togglePointerMode();
}

QGraphicsView *PlayGround::view() const
{
    return static_cast<QGraphicsView *>(graphicsView);
}

void PlayGround::resetView()
{
    //zoomSlider->setValue(250);
    //rotateSlider->setValue(0);
    setupMatrix();
    graphicsView->ensureVisible(QRectF(0, 0, 0, 0));

    //resetButton->setEnabled(false);
}

void PlayGround::setResetButtonEnabled()
{
    //resetButton->setEnabled(true);
}

void PlayGround::setupMatrix()
{
    qreal scale = qPow(qreal(2), (zoom - 250) / qreal(50));
    //qreal scale = qPow(qreal(2), (zoomSlider->value() - 250) / qreal(50));

    QMatrix matrix;
    matrix.scale(scale, scale);
    //matrix.rotate(rotateSlider->value());

    graphicsView->setMatrix(matrix);
    //setResetButtonEnabled();
}

void PlayGround::togglePointerMode()
{
    /*
    graphicsView->setDragMode(selectModeButton->isChecked()
                              ? QGraphicsView::RubberBandDrag
                              : QGraphicsView::ScrollHandDrag);
                              */
    graphicsView->setInteractive(GraphicsView::ScrollHandDrag);
}

void PlayGround::toggleOpenGL()
{
#ifndef QT_NO_OPENGL
    graphicsView->setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
    //graphicsView->setViewport(openGlButton->isChecked() ? new QGLWidget(QGLFormat(QGL::SampleBuffers)) : new QWidget);
#endif
}

void PlayGround::toggleAntialiasing()
{
    graphicsView->setRenderHint(QPainter::Antialiasing, true);
    //graphicsView->setRenderHint(QPainter::Antialiasing, antialiasButton->isChecked());
}

void PlayGround::print()
{
#if !defined(QT_NO_PRINTER) && !defined(QT_NO_PRINTDIALOG)
    QPrinter printer;
    QPrintDialog dialog(&printer, this);
    if (dialog.exec() == QDialog::Accepted) {
        QPainter painter(&printer);
        graphicsView->render(&painter);
    }
#endif
}

void PlayGround::zoomIn(int level)
{
    zoom += 10;
    setupMatrix();
    //zoomSlider->setValue(zoomSlider->value() + level);
}

void PlayGround::zoomOut(int level)
{
    zoom -= 10;
    setupMatrix();
    //zoomSlider->setValue(zoomSlider->value() - level);
}

void PlayGround::rotateLeft()
{
    //rotateSlider->setValue(rotateSlider->value() - 10);
}

void PlayGround::rotateRight()
{
    //rotateSlider->setValue(rotateSlider->value() + 10);
}







#include <QtWidgets>

Chip::Chip(const QColor &color, int x, int y)
{
    this->x = x;
    this->y = y;
    this->color = color;
    setZValue((x + y) % 2);

    setFlags(ItemIsSelectable | ItemIsMovable);
    setAcceptHoverEvents(true);
}

QRectF Chip::boundingRect() const
{
    return QRectF(0, 0, 110, 70);
}

QPainterPath Chip::shape() const
{
    QPainterPath path;
    path.addRect(14, 14, 82, 42);
    return path;
}

void Chip::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

    QColor fillColor = (option->state & QStyle::State_Selected) ? color.dark(150) : color;
    if (option->state & QStyle::State_MouseOver)
        fillColor = fillColor.light(125);

    const qreal lod = option->levelOfDetailFromTransform(painter->worldTransform());
    if (lod < 0.2) {
        if (lod < 0.125) {
            painter->fillRect(QRectF(0, 0, 110, 70), fillColor);
            return;
        }

        QBrush b = painter->brush();
        painter->setBrush(fillColor);
        painter->drawRect(13, 13, 97, 57);
        painter->setBrush(b);
        return;
    }

    QPen oldPen = painter->pen();
    QPen pen = oldPen;
    int width = 2;
    if (option->state & QStyle::State_Selected)
        width += 2;

    pen.setWidth(width);
    QBrush b = painter->brush();
    painter->setBrush(QBrush(fillColor.dark(option->state & QStyle::State_Sunken ? 120 : 100)));

    painter->drawRect(QRect(14, 14, 79, 39));
    painter->setBrush(b);

    if (lod >= 1) {
        painter->setPen(QPen(Qt::gray, 1));
        painter->drawLine(15, 54, 94, 54);
        painter->drawLine(94, 53, 94, 15);
        painter->setPen(QPen(Qt::black, 0));
    }

    // Draw text
    if (lod >= 2) {
        QFont font("Times", 10);
        font.setStyleStrategy(QFont::ForceOutline);
        painter->setFont(font);
        painter->save();
        painter->scale(0.1, 0.1);
        painter->drawText(170, 180, QString("Model: VSC-2000 (Very Small Chip) at %1x%2").arg(x).arg(y));
        painter->drawText(170, 200, QString("Serial number: DLWR-WEER-123L-ZZ33-SDSJ"));
        painter->drawText(170, 220, QString("Manufacturer: Chip Manufacturer"));
        painter->restore();
    }

    // Draw lines
    QVarLengthArray<QLineF, 36> lines;
    if (lod >= 0.5) {
        for (int i = 0; i <= 10; i += (lod > 0.5 ? 1 : 2)) {
            lines.append(QLineF(18 + 7 * i, 13, 18 + 7 * i, 5));
            lines.append(QLineF(18 + 7 * i, 54, 18 + 7 * i, 62));
        }
        for (int i = 0; i <= 6; i += (lod > 0.5 ? 1 : 2)) {
            lines.append(QLineF(5, 18 + i * 5, 13, 18 + i * 5));
            lines.append(QLineF(94, 18 + i * 5, 102, 18 + i * 5));
        }
    }
    if (lod >= 0.4) {
        const QLineF lineData[] = {
            QLineF(25, 35, 35, 35),
            QLineF(35, 30, 35, 40),
            QLineF(35, 30, 45, 35),
            QLineF(35, 40, 45, 35),
            QLineF(45, 30, 45, 40),
            QLineF(45, 35, 55, 35)
        };
        lines.append(lineData, 6);
    }
    painter->drawLines(lines.data(), lines.size());

    // Draw red ink
    if (stuff.size() > 1) {
        QPen p = painter->pen();
        painter->setPen(QPen(Qt::red, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->setBrush(Qt::NoBrush);
        QPainterPath path;
        path.moveTo(stuff.first());
        for (int i = 1; i < stuff.size(); ++i)
            path.lineTo(stuff.at(i));
        painter->drawPath(path);
        painter->setPen(p);
    }
}

void Chip::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
    update();
}

void Chip::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->modifiers() & Qt::ShiftModifier) {
        stuff << event->pos();
        update();
        return;
    }
    QGraphicsItem::mouseMoveEvent(event);
}

void Chip::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    update();
}


