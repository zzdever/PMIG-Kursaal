#ifndef SCRIBBLEAREA_H
#define SCRIBBLEAREA_H

#include <QColor>
#include <QImage>
#include <QPoint>
#include <QWidget>
#include <QList>
#include <QCursor>
#include <QVector>

#include <cv.h>
#include <highgui.h>
#include <opencv2/imgproc/imgproc.hpp>

#include "toolbox.h"
#include "shared/hoverpoints.h"


/// QWidget in the center to scribble
class ScribbleArea : public QWidget
{
    Q_OBJECT

public:
    /// Constructor
    ScribbleArea(QWidget *parent = 0);

    /// @brief Query whether the file is modified
    /// @return Whether modified
    /// @retval true Modified
    /// @retval false Not modified or saved
    bool isModified() const { return modified; }
    /// @brief Set the modification state of the file
    /// @param [in] value True or false for modified or not modified
    void setModified(bool value) {modified = value; return; }

    /// @brief Set the new tool type
    void setToolType(ToolType::toolType type);
    /// @brief Set the new foreground color
    /// @param [in] color The new foreground color
    void setFgColor(QColor color) {fgColor=color;}
    /// @brief Set the new background color
    /// @param [in] colot The new background color
    void setBgColor(QColor color) {bgColor=color;}
    /// @brief Establish a selection area includes all
    void selectAll(void);

    /// Apply tool effects, need two positions' information
    void ApplyToolFunction(QPoint lastPoint, QPoint currentPoint);
    /// Apply tool effects, need one position's information
    void ApplyToolFunction(QPoint currentPoint);
    /// Apply tool effects, no positions' information needed
    void ApplyToolFunction();

    /// @brief Open an image
    bool openImage(const QString &fileName);
    /// @brief Save an image
    bool saveImage(const QString &fileName, const char *fileFormat);
    /// @brief Resize an image
    /// @remarks Not used
    void resizeImage(QImage *image, const QSize &newSize);
    /// @brief Notification from mainwindow to implement rotate operation
    void setTransformSelectionState(void);

public slots:
    /// @brief Print the image
    void print();
    /// @brief To update the display QImage when changes made
    void updateDisplay(int changedImageNum);
    /// @brief To update cursor if necessary
    void updateCursor();
    /// @brief To make a selection when pen is active
    void makeSelection(void);
    /// @brief Stroke the path when pen is active
    void strokeSelectedArea(void);
    /// @brief Fill the area useing foreground color when pen is active
    void fillSelectedArea(void);
    /// @brief Apply black and white filter
    void blackAndWhite(void);
    /// @brief Apply gaussian blur filter
    void gaussianBlur(void);
    /// @brief Apply canny edge detect filter
    void cannyEdge(void);
    /// @brief Apply erode filter
    void erodeFilter(void);
    /// @brief Apply dilate filter
    void dilateFilter(void);
    /// @brief Apply grab the object filter
    void grabcutFilter(void);


protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void enterEvent(QEvent * event);
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    //    void mouseDoubleClickEvent(QMouseEvent *event);

private:
    int totalImageNum;   ///< Total number of images
    int currentImageNum;    ///< The index of the current image under process
    QList<QImage> imageStackDisplay;  ///< Maintain a stack of QImage to display
    QPoint imageCentralPoint;   ///< The central point of scribble area

    ToolType::toolType toolType;   ///< Current tool type
    const int toolIndicationAlpha;  ///< Handler transparency of pen and marquee tool
    QPolygonF marqueeHandlerControl;  ///< Points for marquee tool handler
    HoverPoints *marqueeHandler;    ///< Four handlers of marquee tool
    QPolygonF lassoHandlerControl;  ///< Points for lasso tool handler
    HoverPoints *lassoHandler;   ///< Handlers of lasso tool
    //QPolygonF penHandlerControl;
    //HoverPoints *penHandler;
    /// @note Anchor Point not used, should be used if the rotate implemented completely
    HoverPoints *anchorPoint;

    BrushToolFunction *brushToolFunction;  ///< Used to get brush tool parameters
    EraseToolFunction *eraseToolFunction;  ///< Used to get erase parameters
    PenToolFunction *penToolFunction;   ///< Used to get pen parameters
    QMenu* penMenu;   ///< Context menu of pen

    QList<IplImage*> imageStackEdit;   ///< Stack of IlpImage* of iamge actually working on
//    QList<Mat> imageStack;
    //QImage CVMatToQImage(const cv::Mat& imgMat);
    /// @brief Convert a Iplimage to QImage to display
    QImage IplImage2QImage(const IplImage *iplImage, double mini, double maxi);


    static const uchar TwoPointsSelection=0;   ///< Indicate selection uses two points mode
    static const uchar PolygonSelection=1;  ///< Indicate selection uses polygon mode
    uchar selectionType;    ///< Record the selection mode
    QPoint firstPoint;   ///< The Point when mouse is pressed
    QPoint lastPoint;   ///< The Point where the mouse at the last time

    ///@remarks Ellipse selection also uses this
    CvPoint vertexLeftTop;      ///< Two points to denote a rectangle area @see vertexRightBottom
    CvPoint vertexRightBottom;     ///< @see vertexLeftTop
    QVector<CvPoint> irregularSelectionPoints;  ///< Vector of all the points of the irregular selection
    long int irregularSelectionPointNum;  ///< Total bumber of points of the irregular selection, not used
    QColor fgColor;   ///< Foregroud color
    QColor bgColor;   ///< Backgroud color

    bool modified;  ///< Whether the file has been modified
    bool somethingSelected;  ///< Whether some area is selected by either marquee, lasso or pen
    bool isMouseMoving;   ///< Whether the mouse is pressed and moving
    bool isMousePressed;   ///< Whether the mouse is pressed

private slots:
    void updateMarqueeHandlerControlPoints(QPolygonF);  ///< Update the marquee handlers when resize the selection

//    void updatePenHandlerControlPoints(QPolygonF points) {
//        penHandlerControl=points;
//        penHandler->setPoints(points);
//    }

private:

    bool originalImageSaved;   ///< Whether the original image is saved

    cv::Mat tmpImage;   ///< Temopary image Mat
    cv::Mat originalImage;  ///< The original image
    cv::Mat partImage;  ///< The Mat of selected area
    cv::Mat partMask;   ///< The mask for selected area
    cv::Mat mask;       ///< A mask

    /// @brief Convert Iplimage to Mat
    void Ipl2Mat();
    /// @brief Convert Mat to Iplimage
    void Mat2Ipl();
    /// @brief Draw a line
    void drawLineTo(QPoint lastPoint, QPoint currentPoint);
    /// @brief Fill the area using backgroung color
    void deleteSelectedArea(void);
    /// @brief Draw the mask
    void drawMask(int value=100);
    /// @brief Readjust the rectangle
    void readjustRect();
    /// @brief Drag and move the selected area
    void dragMoveSelectedArea();
    /// @brief Rotate the selected area, driver
    void rotateSelectedArea(QPoint firstPoint, QPoint lastPoint);
    /// @brief Rotate the selected area, actually do
    IplImage* rotateImage2(IplImage* img, double angle);


};

#endif
