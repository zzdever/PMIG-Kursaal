#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QHash>
#include <QWidget>
#include <QAction>
#include <QLayout>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QTextEdit>
#include <QFile>
#include <QDataStream>
#include <QFileDialog>
#include <QMessageBox>
#include <QSignalMapper>
#include <QApplication>
#include <QPainter>
#include <QMouseEvent>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QDockWidget>
#include <QImageWriter>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>
#include <QBitmap>
#include <QtEvents>
#include <QFrame>
#include <QMainWindow>
#include <QObject>
#include <QTimer>
#include <QDebug>
#include <QStackedWidget>

#include <cv.h>
#include <highgui.h>

#include "scribblearea.h"
#include "panels.h"
#include "toolbox.h"
#include "playground.h"
#include "scenemanager.h"
#include "params.h"
#include "polygonitem.h"

class ToolBar;
QT_FORWARD_DECLARE_CLASS(QMenu)
QT_FORWARD_DECLARE_CLASS(QSignalMapper)

/// Class to create the main window
class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    ScribbleArea *centerScribbleArea; ///< Area in the center
    QHash<ToolType::toolType, ToolTweak*> toolsToolBar;  ///< A list of all of toolbars of every tool
    ToolType::toolType currentToolType;  ///< Record the type of current selected tool
    QToolBar *toolBox;  ///< The toolbox bar on the left

    QMenu *fileMenu;  ///< Menu File
    QMenu *filterMenu;  ///<  Menu Filter
    QMenu *windowWidgetMenu;  ///< Menu Window
    QMenu *aboutMenu;   ///< Menu about
    QMenu *loadSceneMenu;  ///< Menu File

    QStackedWidget *stackedWidget;
    SceneManager *sceneManager;
    PlayGround *playGround;
    PolygonItem *texturingItem;

    QTimer *timer;
    //QBasicTimer videoRecTimer;
    //int timeCounter;

public:
    /// Constructor
    MainWindow(QWidget *parent = 0);
    //~MainWindow();

    MainWindow* getInstance() {return this;}

    //void timerEvent(QTimerEvent *e);

protected:
    /// Rewrite to handle save or discard before quit
    void closeEvent(QCloseEvent *event);
    //void keyPressEvent(QKeyEvent *event);
    //void paintEvent(QPaintEvent *);


private:
    /// Create toolbar on the top and on the left(toolbox)
    void setupToolBar();
    /// Create menus
    void setupMenuBar();
    /// Create widgets on the right
    void setupWindowWidgets();
    /// Switch to a new toolbar on the box when the tool changes
    void switchToolsToolBar(ToolType::toolType newToolType);

private slots:
    /// Open a file
    void openFile();
    /// Initiate a save file action and maintain whether the file has been modified
    void saveFile();
    /// Quiery whether to save the file
    bool maybeSave();
    /// Ask for format and save the file
    bool saveWrite(const QByteArray);

    /// Save the current window layout
    void saveLayout();
    /// Load a saved window layout
    void loadLayout();

    /// Show about information
    void about();


    /// Set tool to marquee
    void setToolMarquee(bool toggle){
        if(toggle) switchToolsToolBar(ToolType::Marquee);
    }
    /// Set tool to brush
    void setToolBrush(bool toggle){
        if(toggle) switchToolsToolBar(ToolType::Brush);
    }
    /// Set tool to pen
    void setToolPen(bool toggle){
        if(toggle) switchToolsToolBar(ToolType::Pen);
    }
    /// Set tool to erase
    void setToolErase(bool toggle){
        if(toggle) switchToolsToolBar(ToolType::Erase);
    }
    /// Set tool to transform
    void setToolTransform(bool toggle){
        if(toggle) {switchToolsToolBar(ToolType::Transform);
            centerScribbleArea->setTransformSelectionState();
        }
    }
    /// Set tool to lasso
    void setToolLasso(bool toggle){
        if(toggle) switchToolsToolBar(ToolType::Lasso);
    }

    /// Ask and set the foreground and background color
    void setColor(int);

    void RefreshScene(){
        sceneManager->Render();
        playGround->updateView();
    }

    void SwitchToKurssal();

public:
    void SwitchToTextureEditing(PolygonItem *item);


signals:
    /// Update color icons on the toolbox bar
    void updateColorIcon(int, QColor);
};


#endif
