/// @file
/// This file implements all the functions
/// to draw the graphic user interface,
/// including toolbar, widget, menu, etc.
///

#include "mainwindow.h"

#define TILE_SIZE 100

Q_DECLARE_METATYPE(QDockWidget::DockWidgetFeatures)

/// @param [in] parent Parent QWidget
MainWindow::MainWindow(QWidget *parent)
    :QMainWindow(parent)
{
    setObjectName("MainWindow");
    setWindowTitle("PMIG Kursaal");
    setAutoFillBackground(true);
    setObjectName("MainWindow");

    stackedWidget = new QStackedWidget();

    playGround = new PlayGround("PlayGround", this);
    //setCentralWidget(playGround);
    sceneManager = new SceneManager();
    sceneManager->setSceneRect(-SCENE_WIDTH_HALF, -SCENE_HEIGHT_HALF,
                               SCENE_WIDTH_HALF*2, SCENE_WIDTH_HALF*2);
    playGround->setScene(sceneManager);
    stackedWidget->addWidget(playGround);

    centerScribbleArea = new ScribbleArea(this);
    //setCentralWidget(centerScribbleArea);

    centerScribbleArea->setAutoFillBackground(true);
    QPixmap bg(TILE_SIZE, TILE_SIZE);
    QPainter bgPainter(&bg);
    bgPainter.setPen(QPen(Qt::white,0));
    bgPainter.setBrush(QBrush(QColor(245,245,245)));
    bgPainter.drawRect(0, 0, TILE_SIZE/2, TILE_SIZE/2);
    bgPainter.setBrush(QBrush(Qt::white));
    bgPainter.drawRect(TILE_SIZE/2, 0, TILE_SIZE/2, TILE_SIZE/2);
    bgPainter.setBrush(QBrush(Qt::white));
    bgPainter.drawRect(0, TILE_SIZE/2, TILE_SIZE/2, TILE_SIZE/2);
    bgPainter.setBrush(QBrush(QColor(245,245,245)));
    bgPainter.drawRect(TILE_SIZE/2, TILE_SIZE/2, TILE_SIZE/2, TILE_SIZE/2);
    QPalette bgPalette;
    //bgPalette.setBrush(QPalette::Background, QBrush(bg));
    bgPalette.setBrush(QPalette::Background, QBrush(QImage(":images/bg.png")));
    centerScribbleArea->setPalette(bgPalette);
    playGround->setBackgroundBrush(QBrush(bg));

    centerScribbleArea->setFocusPolicy(Qt::WheelFocus);

    stackedWidget->addWidget(centerScribbleArea);


    setCentralWidget(stackedWidget);
    //stackedWidget->setCurrentIndex(1);

    setupToolBar();

    setupMenuBar();

    //setupWindowWidgets();
    DockOptions dockOptions = AnimatedDocks|AllowTabbedDocks;
    QMainWindow::setDockOptions(dockOptions);

    //statusBar()->showMessage(tr("Ready"));

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(RefreshScene()));
    timer->start(20);

    //timeCounter = 0;
    //videoRecTimer.start(1000*1/30, this);
}

//MainWindow::~MainWindow()
//{
//    ;
//}

/*
void MainWindow::timerEvent(QTimerEvent *e)
{
    if(e->timerId() == videoRecTimer.timerId()){
        timeCounter++;
        if(timeCounter>300){
            QPixmap pixMap = this->centralWidget()->grab();
            pixMap.save(QString("C:\\Users\\ying\\output\\%1.png").arg(timeCounter));
            //qDebug()<<QString("C:\\Users\\ying\\output\\%1.png").arg(timeCounter);
            //image->save(QString("C:\\Users\\ying\\output\\%1.png").arg(timeCounter));
        }

        videoRecTimer.start(1000*1/30, this);
    }
}
*/


/// @brief Used to generate the color button on the toolbox
///
/// This action can update icon when necessary
class ColorIconAction:public QAction
{
public:
    static int actionNum; ///< Static num to track the number of objects have been created
    int id;  ///< ID number of the action
    /// Constructor
    /// @param [in] icon Icon of the action
    /// @param [in] text Label of the action
    /// @param [in] parent Parent widget.
    ColorIconAction(const QIcon &icon, const QString &text, QObject *parent)
        :QAction(icon, text, parent) {}

public slots:
    /// Update the icon of the action when color changes
    void updateColorIcon(int id, QColor color);

};

int ColorIconAction::actionNum;

/// @param [in] id Indicate which icon to update according to this id
/// @param [in] color The new color
void ColorIconAction::updateColorIcon(int id, QColor color)
{
    if(id!=this->id) return;

    QPixmap icon(25,25);
    icon.fill(color);
    this->setIcon(icon);
}

void MainWindow::setupToolBar()
{
    currentToolType = ToolType::Brush;

    toolsToolBar.insert(ToolType::Brush, new BrushToolTweak(this));
    toolsToolBar.insert(ToolType::Erase, new EraseToolTweak(this));
    toolsToolBar.insert(ToolType::Marquee, new MarqueeToolTweak(this));
    toolsToolBar.insert(ToolType::Transform, new TransformToolTweak(this));
    toolsToolBar.insert(ToolType::Lasso, new LassoToolTweak(this));
    toolsToolBar.insert(ToolType::Pen, new PenToolTweak(this));
    foreach (ToolType::toolType tmp, toolsToolBar.keys()) {
        addToolBar(toolsToolBar[tmp]);
    }
    toolsToolBar[currentToolType]->setHidden(false);


    QActionGroup *toolBoxGroup = new QActionGroup(this);
    toolBoxGroup->setExclusive(true);

    QAction *marqueeAct = new QAction(QIcon(":images/marquee-square.png"),tr("&Marquee tool (M)"),this);
    marqueeAct->setShortcut(Qt::Key_M);
    marqueeAct->setStatusTip(tr("To select an area"));
    marqueeAct->setCheckable(true);
    connect(marqueeAct,SIGNAL(toggled(bool)),this,SLOT(setToolMarquee(bool)));

    QAction *brushAct = new QAction(QIcon(":images/brush-tool.png"),tr("&Brush tool (B)"),this);
    brushAct->setShortcut(Qt::Key_B);
    brushAct->setStatusTip(tr("To brush strokes"));
    brushAct->setCheckable(true);
    brushAct->setChecked(true);
    connect(brushAct,SIGNAL(toggled(bool)),this,SLOT(setToolBrush(bool)));

    QAction *penAct = new QAction(QIcon(":images/pen-tool.png"),tr("&Pen tool (P)"),this);
    penAct->setShortcut(Qt::Key_P);
    penAct->setStatusTip(tr("To draw a path"));
    penAct->setCheckable(true);
    connect(penAct,SIGNAL(toggled(bool)),this,SLOT(setToolPen(bool)));

    QAction *eraseAct = new QAction(QIcon(":images/eraser-tool.png"),tr("&Erase tool (E)"),this);
    eraseAct->setShortcut(Qt::Key_E);
    eraseAct->setStatusTip(tr("To erase an area"));
    eraseAct->setCheckable(true);
    connect(eraseAct,SIGNAL(toggled(bool)),this,SLOT(setToolErase(bool)));

    QAction *transformAct = new QAction(QIcon(":images/transform-tool.png"),tr("&Transform tool (T)"),this);
    transformAct->setShortcut(Qt::Key_T);
    transformAct->setStatusTip(tr("Freely transform the shape of an area"));
    transformAct->setCheckable(true);
    connect(transformAct,SIGNAL(toggled(bool)),this,SLOT(setToolTransform(bool)));

    QAction *lassoAct = new QAction(QIcon(":images/lasso-tool.png"),tr("&Lasso tool (L)"),this);
    lassoAct->setShortcut(Qt::Key_L);
    lassoAct->setStatusTip(tr("To select an irregular area"));
    lassoAct->setCheckable(true);
    connect(lassoAct,SIGNAL(toggled(bool)),this,SLOT(setToolLasso(bool)));


    toolBoxGroup->addAction(marqueeAct);
    toolBoxGroup->addAction(brushAct);
    toolBoxGroup->addAction(penAct);
    toolBoxGroup->addAction(eraseAct);
    toolBoxGroup->addAction(transformAct);
    toolBoxGroup->addAction(lassoAct);


    toolBox = addToolBar(tr("Tool box"));
    addToolBar(Qt::LeftToolBarArea, toolBox);
    toolBox->setAllowedAreas(Qt::LeftToolBarArea);
    toolBox->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    toolBox->setOrientation(Qt::Vertical);
    toolBox->setFloatable(false);
    toolBox->setMovable(false);
    //toolBox->setFocusPolicy(Qt::WheelFocus);


    toolBox->addAction(marqueeAct);
    toolBox->addAction(lassoAct);
    toolBox->addAction(brushAct);
    toolBox->addAction(penAct);
    toolBox->addAction(eraseAct);
    toolBox->addAction(transformAct);

    toolBox->addSeparator();

    QSignalMapper *signalMapper = new QSignalMapper(this);

    QPixmap fgColorIcon(25,25);
    fgColorIcon.fill(QColor(Qt::black));
    ColorIconAction* fgColorAct=new ColorIconAction(QIcon(fgColorIcon), tr("Frontground Color"), this);
    fgColorAct->id=fgColorAct->actionNum++;
    fgColorAct->setStatusTip(tr("To change the frontground color"));
    connect(fgColorAct, SIGNAL(triggered()), signalMapper, SLOT(map()));
    signalMapper->setMapping(fgColorAct,fgColorAct->id);
    connect(this, &MainWindow::updateColorIcon, fgColorAct, &ColorIconAction::updateColorIcon);
    toolBox->addAction(fgColorAct);

    QPixmap bgColorIcon(25,25);
    bgColorIcon.fill(QColor(Qt::white));
    ColorIconAction* bgColorAct=new ColorIconAction(QIcon(bgColorIcon), tr("Background Color"), this);
    bgColorAct->id=bgColorAct->actionNum++;
    bgColorAct->setStatusTip(tr("To change the background color"));
    connect(bgColorAct, SIGNAL(triggered()), signalMapper, SLOT(map()));
    signalMapper->setMapping(bgColorAct,bgColorAct->id);
    connect(this, &MainWindow::updateColorIcon, bgColorAct, &ColorIconAction::updateColorIcon);
    toolBox->addAction(bgColorAct);

    connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(setColor(int)));

}

/// @param [in] id Indicate which color to update
void MainWindow::setColor(int id)
{
    QColorDialog dialog(this);
    dialog.exec();
    //if(dialog.) return;
    QColor color=dialog.selectedColor();
    if(id==0)
        centerScribbleArea->setFgColor(color);
    if(id==1)
        centerScribbleArea->setBgColor(color);

    emit updateColorIcon(id, color);
    //update();
}


/// @param [in] newToolType The type of new tool
void MainWindow::switchToolsToolBar(ToolType::toolType newToolType)
{
    if(!toolsToolBar.contains(newToolType))
        return;

    toolsToolBar[currentToolType]->setHidden(true);
    toolsToolBar[newToolType]->setHidden(false);
    currentToolType=newToolType;
    centerScribbleArea->setToolType(currentToolType);
}

void MainWindow::setupMenuBar()
{
    QAction *openAct;
    QList<QAction *> saveAsActs;
    QAction *printAct;
    QAction *exitAct;

    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    connect(openAct, SIGNAL(triggered()), this, SLOT(openFile()));

    QList<QByteArray> supportedImageFormats;
    supportedImageFormats.append(QByteArray("jpg"));
    supportedImageFormats.append(QByteArray("bmp"));
    supportedImageFormats.append(QByteArray("png"));
    supportedImageFormats.append(QByteArray("tiff"));
    foreach (QByteArray format, supportedImageFormats) {
        QString text = tr("%1...").arg(QString(format).toUpper());

        QAction *action = new QAction(text, this);
        action->setData(format);
        connect(action, SIGNAL(triggered()), this, SLOT(saveFile()));
        saveAsActs.append(action);
    }

    printAct = new QAction(tr("&Print..."), this);
    printAct->setShortcut(QKeySequence::Print);
    connect(printAct, SIGNAL(triggered()), centerScribbleArea, SLOT(print()));

    exitAct = new QAction(tr("E&xit texture editor"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    //connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(SwitchToKurssal()));


    QMenu *saveAsMenu = new QMenu(tr("&Save As"), this);
    foreach (QAction *action, saveAsActs)
        saveAsMenu->addAction(action);

    fileMenu = new QMenu(tr("&File"), this);
    menuBar()->addMenu(fileMenu);
    fileMenu->addAction(openAct);
    fileMenu->addMenu(saveAsMenu);
    fileMenu->addAction(printAct);
    fileMenu->addSeparator();
    QAction *layoutAct = fileMenu->addAction(tr("Save layout..."));
    connect(layoutAct, SIGNAL(triggered()), this, SLOT(saveLayout()));
    layoutAct = fileMenu->addAction(tr("Load layout..."));
    connect(layoutAct, SIGNAL(triggered()), this, SLOT(loadLayout()));
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);


    filterMenu = new QMenu(tr("&Filter"), this);
    menuBar()->addMenu(filterMenu);

    QAction *filterAct = new QAction(tr("Black and White"), this);
    connect(filterAct, SIGNAL(triggered()), centerScribbleArea, SLOT(blackAndWhite()));
    filterMenu->addAction(filterAct);
    //Gaussian Blur
    filterAct = new QAction(tr("Gaussian Blur"), this);
    connect(filterAct, SIGNAL(triggered()), centerScribbleArea, SLOT(gaussianBlur()));
    filterMenu->addAction(filterAct);
    //Canny Edge Detector
    filterAct = new QAction(tr("Canny Edge Detector"), this);
    connect(filterAct, SIGNAL(triggered()), centerScribbleArea, SLOT(cannyEdge()));
    filterMenu->addAction(filterAct);
    //Erode
    filterAct = new QAction(tr("Erode"), this);
    connect(filterAct, SIGNAL(triggered()), centerScribbleArea, SLOT(erodeFilter()));
    filterMenu->addAction(filterAct);
    //Dilate
    filterAct = new QAction(tr("Dilate"), this);
    connect(filterAct, SIGNAL(triggered()), centerScribbleArea, SLOT(dilateFilter()));
    filterMenu->addAction(filterAct);
    //Grabcut
    filterAct = new QAction(tr("Grab Cut"), this);
    connect(filterAct, SIGNAL(triggered()), centerScribbleArea, SLOT(grabcutFilter()));
    filterMenu->addAction(filterAct);



    loadSceneMenu = new QMenu(tr("&Scene"), this);
    menuBar()->addMenu(loadSceneMenu);

    QAction *loadAct = new QAction(tr("Load Ground"), this);
    connect(loadAct, &QAction::triggered, sceneManager, &SceneManager::LoadGround);
    loadSceneMenu->addAction(loadAct);

    loadAct = new QAction(tr("Load Bouncing Ball"), this);
    connect(loadAct, &QAction::triggered, sceneManager, &SceneManager::LoadBouncingBall);
    loadSceneMenu->addAction(loadAct);

    loadAct = new QAction(tr("Load Domino"), this);
    connect(loadAct, &QAction::triggered, sceneManager, &SceneManager::LoadDomino);
    loadSceneMenu->addAction(loadAct);

    loadAct = new QAction(tr("Load Slide"), this);
    connect(loadAct, &QAction::triggered, sceneManager, &SceneManager::LoadSlide);
    loadSceneMenu->addAction(loadAct);

    loadAct = new QAction(tr("Load Lever"), this);
    connect(loadAct, &QAction::triggered, sceneManager, &SceneManager::LoadLever);
    loadSceneMenu->addAction(loadAct);

    loadAct = new QAction(tr("Load Many Many"), this);
    connect(loadAct, &QAction::triggered, sceneManager, &SceneManager::LoadManyMany);
    loadSceneMenu->addAction(loadAct);



    windowWidgetMenu = menuBar()->addMenu(tr("&Window"));


    aboutMenu = new QMenu(tr("&About"), this);
    QAction *aboutAct = new QAction(tr("About PMIG"), this);
    aboutMenu->addAction(aboutAct);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    QAction *aboutQtAct = new QAction(tr("About QT"), this);
    aboutMenu->addAction(aboutQtAct);
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    menuBar()->addMenu(aboutMenu);
}


void MainWindow::saveLayout()
{
    QString fileName
        = QFileDialog::getSaveFileName(this, tr("Save layout"));
    if (fileName.isEmpty())
        return;
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
        QString msg = tr("Failed to open %1\n%2")
                        .arg(fileName)
                        .arg(file.errorString());
        QMessageBox::warning(this, tr("Error"), msg);
        return;
    }

    QByteArray geo_data = saveGeometry();
    QByteArray layout_data = saveState();

    bool ok = file.putChar((uchar)geo_data.size());
    if (ok)
        ok = file.write(geo_data) == geo_data.size();
    if (ok)
        ok = file.write(layout_data) == layout_data.size();

    if (!ok) {
        QString msg = tr("Error writing to %1\n%2")
                        .arg(fileName)
                        .arg(file.errorString());
        QMessageBox::warning(this, tr("Error"), msg);
        return;
    }
}

void MainWindow::loadLayout()
{
    QString fileName
        = QFileDialog::getOpenFileName(this, tr("Load layout"));
    if (fileName.isEmpty())
        return;
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        QString msg = tr("Failed to open %1\n%2")
                        .arg(fileName)
                        .arg(file.errorString());
        QMessageBox::warning(this, tr("Error"), msg);
        return;
    }

    uchar geo_size;
    QByteArray geo_data;
    QByteArray layout_data;

    bool ok = file.getChar((char*)&geo_size);
    if (ok) {
        geo_data = file.read(geo_size);
        ok = geo_data.size() == geo_size;
    }
    if (ok) {
        layout_data = file.readAll();
        ok = layout_data.size() > 0;
    }

    if (ok)
        ok = restoreGeometry(geo_data);
    if (ok)
        ok = restoreState(layout_data);

    if (!ok) {
        QString msg = tr("Error reading %1")
                        .arg(fileName);
        QMessageBox::warning(this, tr("Error"), msg);
        return;
    }
}


void MainWindow::setupWindowWidgets()
{
    qRegisterMetaType<QDockWidget::DockWidgetFeatures>();

    static const struct Set {
        const char * name;
        uint flags;
        Qt::DockWidgetArea area;
    } sets [] = {
        { "Info", 0, Qt::RightDockWidgetArea },
        { "ColorSwatch", 0, Qt::RightDockWidgetArea },
        { "Graph", 0, Qt::RightDockWidgetArea },
        { "Curve", 0, Qt::RightDockWidgetArea },
        { "Character", 0, Qt::RightDockWidgetArea },
        { "Layer", 0, Qt::RightDockWidgetArea }
    };
    const int setCount = sizeof(sets) / sizeof(Set);

    QList<Panel*> windowWidgetsList;
    for (int i = 0; i < setCount; ++i) {
        Panel *panel = new Panel(tr(sets[i].name), this, Qt::WindowFlags(sets[i].flags));
        panel->setAllowedAreas(Qt::RightDockWidgetArea);
        windowWidgetsList.append(panel);
        addDockWidget(sets[i].area, panel);
        windowWidgetMenu->addAction(panel->windowWidgetAction);
    }

    QMainWindow::setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    QMainWindow::setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    QMainWindow::setTabPosition(Qt::RightDockWidgetArea, QTabWidget::North);
    tabifyDockWidget(windowWidgetsList.at(0), windowWidgetsList.at(1));
    windowWidgetsList.at(0)->raise();
    tabifyDockWidget(windowWidgetsList.at(2), windowWidgetsList.at(3));
    tabifyDockWidget(windowWidgetsList.at(3), windowWidgetsList.at(4));
    windowWidgetsList.at(2)->raise();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
    /*
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
    */
}


//void MainWindow::keyPressEvent(QKeyEvent *event)
//{
//    if(event->matches(QKeySequence::SelectAll))
//        centerScribbleArea->selectAll();
//}

void MainWindow::openFile()
{
    //if (maybeSave())
    {
        QString fileName = QFileDialog::getOpenFileName(this,
                                   tr("Open File"), QDir::currentPath());
        if (!fileName.isEmpty())
        {
            centerScribbleArea->openImage(fileName);
        }
    }
}


/// @param [in] fileFormat Indicate in which file format to save
bool MainWindow::saveWrite(const QByteArray fileFormat)
{
    QString initialPath = QDir::currentPath() + "/untitled." + fileFormat;

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),
                               initialPath,
                               tr("%1 Files (*.%2);;All Files (*)")
                               .arg(QString::fromLatin1(fileFormat.toUpper()))
                               .arg(QString::fromLatin1(fileFormat)));

    if (fileName.isEmpty()) {
        return false;
    } else {
        return centerScribbleArea->saveImage(fileName, fileFormat.constData());
    }
}

void MainWindow::saveFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    QByteArray fileFormat = action->data().toByteArray();
    if(MainWindow::saveWrite(fileFormat)) centerScribbleArea->setModified(false);
}

bool MainWindow::maybeSave()
{
    if (centerScribbleArea->isModified()) {
       QMessageBox::StandardButton ret;
       ret = QMessageBox::warning(this, tr("Modified"),
                          tr("The image has been modified.\n"
                             "Do you want to save your changes?"),
                          QMessageBox::Save | QMessageBox::Discard
              | QMessageBox::Cancel);
        if (ret == QMessageBox::Save) {
            return saveWrite("png");
        } else if (ret == QMessageBox::Cancel) {
            return false;
        }
    }
    return true;
}



void MainWindow::about()
{
    QMessageBox::about(this, tr("About PMIG"),
            tr("<p><b>PMIG</b> is a simple image processing tool.</p>"
               "<p>If you are curious about the name, "
               "it's a tricky reverse of the powerful open software GIMP.</p>"));
}


//void MainWindow::setCurrentFile(const QString &fileName)
//{
//    curFile = fileName;
//    //textEdit->document()->setModified(false);
//    setWindowModified(false);

//    QString shownName = curFile;
//    if (curFile.isEmpty())
//        shownName = "untitled";
//    setWindowFilePath(shownName);
//}

//QString MainWindow::strippedName(const QString &fullFileName)
//{
//    return QFileInfo(fullFileName).fileName();
//}


void MainWindow::SwitchToTextureEditing(PolygonItem *item)
{
    texturingItem = item;
    stackedWidget->setCurrentIndex(1);
}

void MainWindow::SwitchToKurssal()
{
    texturingItem->SetTexture(centerScribbleArea->GetQImage());
    stackedWidget->setCurrentIndex(0);
}
