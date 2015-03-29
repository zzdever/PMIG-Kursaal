/// @file
/// This file implements all the tools.
///
#include <QSpinBox>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QWidget>
#include <QDebug>

#include "toolbox.h"

/// @param [in] title Title for the tool
/// @param [in] parent Parent widget
ToolTweak::ToolTweak(const QString &title, QWidget *parent)
    :QToolBar(title, parent)
{
    this->setAllowedAreas(Qt::TopToolBarArea);
    this->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    this->setMovable(true);
    this->setHidden(true);
    QLabel *name = new QLabel(title,this);
    name->setFixedHeight(35);
    this->addWidget(name);
    this->addSeparator();
}

//+++++++++++++Brush+Tool+++++++++++++++++++++++++++++++++++++
int BrushToolBase::brushSize=2;
int BrushToolBase::lineType;
bool BrushToolBase::antiAliasing;

/// @param [in] parent Parent widget
BrushToolTweak::BrushToolTweak(QWidget *parent)
    :ToolTweak("BRUSH TOOL", parent)
{
    QSpinBox *sizeSpinBox = new QSpinBox(this);
    sizeSpinBox->setRange(1,100);
    sizeSpinBox->setValue(2);
    this->addWidget(new QLabel("size: ",this));
    this->addWidget(sizeSpinBox);
    connect(sizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setBrushSize(int)));

    this->addSeparator();

    QCheckBox *antiAliasingCheckBox = new QCheckBox(this);
    antiAliasingCheckBox->setText("Anti-Aliasing");
    this->addWidget(antiAliasingCheckBox);
    connect(antiAliasingCheckBox,SIGNAL(toggled(bool)),this, SLOT(setAntiAliasing(bool)));
}

/// @param [in] parent Parent widget
BrushToolFunction::BrushToolFunction(QWidget *parent)
    :QObject(parent)
{
    ;
}


//+++++++++++ERASE+TOOL+++++++++++++++++++++++++++++++++++++++
int EraseToolBase::eraseSize=10;
int EraseToolBase::eraseShape;

/// @param [in] parent Parent widget
EraseToolTweak::EraseToolTweak(QWidget *parent)
    :ToolTweak("ERASE TOOL", parent)
{
    QSpinBox *sizeSpinBox = new QSpinBox(this);
    sizeSpinBox->setRange(1,100);
    sizeSpinBox->setValue(10);
    this->addWidget(new QLabel("size: ",this));
    this->addWidget(sizeSpinBox);
    connect(sizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setEraseSize(int)));
}

/// @param [in] parent Parent widget
EraseToolFunction::EraseToolFunction(QWidget *parent)
    :QObject(parent)
{
    ;
}

//+++++++++++Marquee+TOOL+++++++++++++++++++++++++++++++++++++++
int MarqueeToolBase::selectionType;

/// @param [in] parent Parent widget
MarqueeToolTweak::MarqueeToolTweak(QWidget *parent)
    :ToolTweak("MARQUEE TOOL", parent)
{
    QComboBox *selectionTypeBox = new QComboBox(this);
    selectionTypeBox->addItem("Rectangle");
    //selectionTypeBox->addItem("Circle");
    this->addWidget(selectionTypeBox);

    connect(selectionTypeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setSelectionType(int)));
}

/// @param [in] parent Parent widget
MarqueeToolFunction::MarqueeToolFunction(QWidget *parent)
    :QObject(parent)
{
    ;
}

//+++++++++++Transform+TOOL+++++++++++++++++++++++++++++++++++++++
/// @param [in] parent Parent widget
TransformToolTweak::TransformToolTweak(QWidget *parent)
    :ToolTweak("TRANSFORM TOOL", parent)
{
    //temporary
    this->addSeparator();
    QLabel *note = new QLabel("Sorry, currently size free transformation is NOT supported",this);
    this->addWidget(note);
    //temporary
}

/// @param [in] parent Parent widget
TransformToolFunction::TransformToolFunction(QWidget *parent)
    :QObject(parent)
{
    ;
}


//+++++++++++Lasso+TOOL+++++++++++++++++++++++++++++++++++++++
/// @param [in] parent Parent widget
bool LassoToolBase::magnetic=false;

LassoToolTweak::LassoToolTweak(QWidget *parent)
    :ToolTweak("LASSO TOOL", parent)
{
    QCheckBox *magneticCheckBox = new QCheckBox(this);
    magneticCheckBox->setText("Magnetic");
    this->addWidget(magneticCheckBox);
    connect(magneticCheckBox,SIGNAL(toggled(bool)),this, SLOT(setMagnetic(bool)));
}

/// @param [in] parent Parent widget
LassoToolFunction::LassoToolFunction(QWidget *parent)
    :QObject(parent)
{
    ;
}


//+++++++++++Pen+TOOL+++++++++++++++++++++++++++++++++++++++
/// @param [in] parent Parent widget
PenToolTweak::PenToolTweak(QWidget *parent)
    :ToolTweak("PEN TOOL", parent)
{
    ;
}

/// @param [in] parent Parent widget
PenToolFunction::PenToolFunction(QWidget *parent)
    :QObject(parent)
{
    penHandler = new HoverPoints(parent, HoverPoints::CircleShape);
    penHandler->setConnectionType(HoverPoints::LineConnection);
    penHandler->setCloseType(HoverPoints::Close);
    penHandler->setEditable(false);
    penHandler->setPointSize(QSize(10, 10));
    penHandler->setShapeBrush(QBrush(QColor(0, 0, 0, 255)));
    penHandler->setShapePen(QPen(QColor(0, 0, 0, 150)));
    penHandler->setConnectionPen(QPen(QColor(0, 0, 0, 150)));
    connect(penHandler, SIGNAL(pointsChanged(QPolygonF)),this, SLOT(updatePenHandlerControlPoints(QPolygonF)));

    penHandlerControl.clear();

    penMenu = new QMenu;
    QAction *action = new QAction("Clear pen Points", penMenu);
    connect(action, SIGNAL(triggered()), this, SLOT(clearPoints()));
    penMenu->addAction(action);
    penMenu->addSeparator();
    action = new QAction("Make selection", penMenu);
    connect(action, SIGNAL(triggered()),this,SLOT(makeSelection_()));
    penMenu->addAction(action);
    action = new QAction("Stroke path", penMenu);
    connect(action, SIGNAL(triggered()),this,SLOT(strokePath_()));
    penMenu->addAction(action);
    action = new QAction("Fill shape", penMenu);
    connect(action, SIGNAL(triggered()),this, SLOT(fillPath_()));
    penMenu->addAction(action);
}
