/// @file
/// This file implements the panels
///
#include "panels.h"

#include <QAction>
#include <QtEvents>
#include <QFrame>
#include <QMainWindow>
#include <QMenu>
#include <QPainter>
#include <QImage>
#include <QColor>
#include <QDialog>
#include <QGridLayout>
#include <QSpinBox>
#include <QLabel>
#include <QPainterPath>
#include <QPushButton>
#include <QHBoxLayout>
#include <QBitmap>
#include <QFontMetrics>
#include <QtDebug>

#define PANEL_IMPLEMENTED

/// @param [in] panelName Name for the panel
/// @param [in] parent Parent widget
PanelFrame::PanelFrame(const QString &panelName, QWidget *parent)
    : QFrame(parent)
{
    Q_UNUSED(panelName);
//////////////FONT/////////
    QFont font = this->font();
    font.setPointSize(8);
    setFont(font);

}


void PanelFrame::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), Qt::white);

#ifdef PANEL_IMPLEMENTED
    p.setRenderHint(QPainter::Antialiasing, false);

    QSize sz = size();
    QString text = QString::fromLatin1("sz: %1x%2\n\n"
                                       "Sorry, currently panel\n"
                                       "is NOT implemented")
                    .arg(sz.width()).arg(sz.height());

    QRect r = fontMetrics().boundingRect(rect(), Qt::AlignLeft|Qt::AlignTop, text);
    r.adjust(-2, -2, 1, 1);
    p.translate(4, 4);
    QColor bg = Qt::cyan;
    bg.setAlpha(60);
    p.setBrush(bg);
    p.setPen(Qt::black);
    p.drawRect(r);
///////////DRAW//TEXT////////
    p.drawText(rect(), Qt::AlignLeft|Qt::AlignTop, text);
#endif // PANEL_IMPLEMENTED
}


/// @param [in] panelName Name for the panel
/// @param [in] parent Parent widget
/// @param [in] flags Window style
Panel::Panel(const QString &panelName, QWidget *parent, Qt::WindowFlags flags)
    : QDockWidget(parent, flags)
{
    setObjectName(panelName);
    setWindowTitle(objectName());

    QFrame *frame = new PanelFrame(panelName, this);
    frame->setFrameStyle(QFrame::Box | QFrame::Sunken);

    setWidget(frame);

    windowWidgetAction = toggleViewAction();

}


/// @param [in] e The resize event
void Panel::resizeEvent(QResizeEvent *e)
{
    QDockWidget::resizeEvent(e);
}

