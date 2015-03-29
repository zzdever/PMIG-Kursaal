#ifndef TOOLBOX_H
#define TOOLBOX_H

#include <QObject>
#include <QHash>
#include <QToolBar>
#include <QList>
#include <QSpinBox>
#include <QDebug>

#include "shared/hoverpoints.h"

/// Enum class for tool types
class ToolType{
public:
    enum toolType{
        Brush=0,
        Erase=1,
        Marquee=2,
        Pen=3,
        Transform=4,
        Lasso=5
    };
};

/// Tool parameter tweak base class, generate toolbar
class ToolTweak:public QToolBar
{
    Q_OBJECT
public:
    /// Constructor
    ToolTweak(const QString &title, QWidget *parent);

};


//+++++++++++++Brush+Tool+++++++++++++++++++++++++++++++++++++
/// Brush tool base class, contains parameters
class BrushToolBase
{
protected:
    static int brushSize;
    static int lineType;
    static bool antiAliasing;
};

/// Brush tool parameter tweak, generate tool bar and provide parameter
class BrushToolTweak
        :public ToolTweak,
        protected BrushToolBase
{
    Q_OBJECT
public:
    /// Constructor
    BrushToolTweak(QWidget *parent);

private slots:
    void setBrushSize(int value){brushSize=value;}
    void setLineType(int value){lineType=value;}
    void setAntiAliasing(bool value){antiAliasing=value;}
};

/// Used by opencv process functions to get parameters
class BrushToolFunction
        :public QObject,
        protected BrushToolBase
{
    Q_OBJECT
public:
    /// Constructor
    BrushToolFunction(QWidget *parent);

    int getBrushSize() const {return brushSize;}
    int getLineType() const {return lineType;}
    bool getAntiAliasing() const {return antiAliasing;}

};


//+++++++++++ERASE+TOOL+++++++++++++++++++++++++++++++++++++++
/// Erase tool base class, contains parameters
class EraseToolBase
{
protected:
    static int eraseSize;
    static int eraseShape;
};

/// Erase tool parameter tweak, generate tool bar and provide parameter
class EraseToolTweak
        :public ToolTweak,
        protected EraseToolBase
{
    Q_OBJECT
public:
    /// Constructor
    EraseToolTweak(QWidget *parent);

private slots:
    void setEraseSize(int value){eraseSize=value;}
    void setEraseShape(int value){eraseShape=value;}

signals:
};

/// Used by opencv process functions to get parameters
class EraseToolFunction
        :public QObject,
        protected EraseToolBase
{
    Q_OBJECT
public:
    /// Constructor
    EraseToolFunction(QWidget *parent);

    int getEraseSize()const {return eraseSize;}
    bool getEraseShape()const {return eraseShape;}

};


//+++++++++++++Marquee+Tool+++++++++++++++++++++++++++++++++++++
/// Marquee tool base class, contains parameters
class MarqueeToolBase
{
protected:
    static int selectionType;
};

/// Marquee tool parameter tweak, generate tool bar and provide parameter
class MarqueeToolTweak
        :public ToolTweak,
        protected MarqueeToolBase
{
    Q_OBJECT
public:
    /// Constructor
    MarqueeToolTweak(QWidget *parent);

private slots:
    void setSelectionType(int value){selectionType=value;}

};

/// Used by opencv process functions to get parameters
class MarqueeToolFunction
        :public QObject,
        protected MarqueeToolBase
{
    Q_OBJECT
public:
    /// Constructor
    MarqueeToolFunction(QWidget *parent);

    int getSelectionType() const {return selectionType;}

};


//+++++++++++++Transform+Tool+++++++++++++++++++++++++++++++++++++
/// Transform tool base class, contains parameters
class TransformToolBase
{
protected:
};

/// Transform tool parameter tweak, generate tool bar and provide parameter
class TransformToolTweak
        :public ToolTweak,
        protected TransformToolBase
{
    Q_OBJECT
public:
    /// Constructor
    TransformToolTweak(QWidget *parent);

private slots:
    //void setSelectionType(int value){selectionType=value;}

};

/// Used by opencv process functions to get parameters
class TransformToolFunction
        :public QObject,
        protected TransformToolBase
{
    Q_OBJECT
public:
    /// Constructor
    TransformToolFunction(QWidget *parent);

    //int getSelectionType() const {return selectionType;}

};


//+++++++++++++Lasso+Tool+++++++++++++++++++++++++++++++++++++
/// Lasso tool base class, contains parameters
class LassoToolBase
{
protected:
    static bool magnetic;
};

/// Lasso tool parameter tweak, generate tool bar and provide parameter
class LassoToolTweak
        :public ToolTweak,
        protected LassoToolBase
{
    Q_OBJECT
public:
    /// Constructor
    LassoToolTweak(QWidget *parent);

private slots:
    void setMagnetic(bool value){magnetic=value;}

};

/// Used by opencv process functions to get parameters
class LassoToolFunction
        :public QObject,
        protected LassoToolBase
{
    Q_OBJECT
public:
    /// Constructor
    LassoToolFunction(QWidget *parent);

    bool getMagnetic() const {return magnetic;}

};


//+++++++++++++Pen+Tool+++++++++++++++++++++++++++++++++++++
/// Pen tool base class, contains parameters
class PenToolBase
{
protected:
};

/// Pen tool parameter tweak, generate tool bar and provide parameter
class PenToolTweak
        :public ToolTweak,
        protected PenToolBase
{
    Q_OBJECT
public:
    /// Constructor
    PenToolTweak(QWidget *parent);

private slots:
    //void setSelectionType(int value){selectionType=value;}

};

/// Used by opencv process functions to get parameters
class PenToolFunction
        :public QObject,
        protected PenToolBase
{
    Q_OBJECT
public:
    /// Constructor
    PenToolFunction(QWidget *parent);

    HoverPoints *penHandler;  ///< Pointer to pen tool hover points
    QPolygonF penHandlerControl;  ///< Points for the hover points
    QMenu *penMenu;  ///< Context menu for pen tool
    //int getSelectionType() const {return selectionType;}
private slots:
    /// Set new hover points
    void updatePenHandlerControlPoints(QPolygonF points) {
        penHandlerControl=points;
        penHandler->setPoints(points);
    }
    /// Make a selection area according to the hover points
    void makeSelection_() {emit makeSelection();}
    /// Stroke a path according to the hover points
    void strokePath_() {emit makeSelection(); emit strokePath(); }
    /// Fill the area according to the hover points
    void fillPath_() {emit makeSelection(); emit fillPath();}
public slots:
    /// Delete all of the hover points
    void clearPoints() {
        penHandlerControl.clear();
        penHandler->setPoints(penHandlerControl);
    }

signals:
    void makeSelection();
    void strokePath();
    void fillPath();
};
#endif // TOOLBOX_H
