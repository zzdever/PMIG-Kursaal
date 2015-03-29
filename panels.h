#ifndef PANELS_H
#define PANELS_H

#include <QDockWidget>
#include <QFrame>

QT_FORWARD_DECLARE_CLASS(QAction)
QT_FORWARD_DECLARE_CLASS(QActionGroup)
QT_FORWARD_DECLARE_CLASS(QMenu)

/// QDockWidget for panels
class Panel : public QDockWidget
{
    Q_OBJECT

public:
    /// Constructor
    explicit Panel(const QString &panelName, QWidget *parent = 0, Qt::WindowFlags flags = 0);


    QMenu* menu;  ///< Menu for every panel
    QAction *windowWidgetAction; ///< Action for show the panel or not

protected:
    /// Rewrite to deal with panel resize event
    virtual void resizeEvent(QResizeEvent *e);


};

/// Inside frame for panels
class PanelFrame : public QFrame
{
    Q_OBJECT
public:
    /// Constructor
    PanelFrame(const QString &panelName, QWidget *parent);


public slots:

protected:
    /// Rewrite to deal with panel paint event
    void paintEvent(QPaintEvent *);
};

#endif // PANELS_H
