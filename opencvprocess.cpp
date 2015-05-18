/// @file
/// This file contains all the functions
/// that actually do the processing work,
/// powered by opencv library.
///

#include <QMouseEvent>
#include <QWidget>
#include <QPainter>
#include <QBitmap>

#include "scribblearea.h"

/// @param fileName The name of the file to open
/// @return Whether open successfully
/// @retval true Scceed to open
/// @retval false Fail to open
bool ScribbleArea::openImage(const QString &fileName)
{
    IplImage *img = cvLoadImage(fileName.toStdString().c_str());
    if(img)
    {
        if(imageStackEdit.size()>0)
            imageStackEdit.pop_back();
        imageStackEdit.append(img);

        if(0==totalImageNum){
            totalImageNum++;
            currentImageNum = totalImageNum-1;
        }

        updateDisplay(currentImageNum);
        return true;
    } else {
        qDebug()<<"Unable to load image "<<fileName;
        return false;
    }

//    if(totalImageNum == 1)
//    {
//        QMessageBox *tmpMessage = new QMessageBox(this);
//        tmpMessage->setText("Sorry, currently only one layer is supported.");
//        tmpMessage->show();
//        return false;
//    }

//    IplImage *img = cvLoadImage(fileName.toStdString().c_str());
//    if(img)
//    {
//        imageStackEdit.append(img);

//        totalImageNum++;
//        currentImageNum = totalImageNum-1;
//        updateDisplay(currentImageNum);
//        return true;
//    }
//    else
//    {
//        qDebug()<<"Unable to load image "<<fileName;
//        return false;
//    }

//    QSize newSize = loadedImage.size().expandedTo(size());
//    resizeImage(&loadedImage, newSize);
//    image = loadedImage;
//    modified = false;
//    update();
//    return true;

}

/// @param fileName Save in what name
/// @param fileFormat Save in which format
/// @return Whether save successfully
/// @retval true Scceed to save
/// @retval false Fail to save
bool ScribbleArea::saveImage(const QString &fileName, const char *fileFormat)
{
    Q_UNUSED(fileFormat);
    if(currentImageNum>=0){
        cv::Mat image(imageStackEdit[currentImageNum]);
        cv::imwrite((fileName).toStdString(),image);
    }
    return true;
}


/// @param [in] lastPoint The point where the mouse is last time
/// @param [in] currentPoint The point where the mouse now is
void ScribbleArea::ApplyToolFunction(QPoint lastPoint, QPoint currentPoint)
{
    switch (toolType) {
    case ToolType::Brush:
    case ToolType::Erase:
        drawLineTo(lastPoint, currentPoint);
        break;
    case ToolType::Transform:{
        //Pay attetion that vertexLeftTop may be not LeftTop
        //and vertexRightBottom may be not RightBottom
        if(lastPoint.x()>vertexLeftTop.x+5 && lastPoint.x()<vertexRightBottom.x-5
                && lastPoint.y()>vertexLeftTop.y+5 && lastPoint.y()<vertexRightBottom.y-5){
            qDebug()<<"In, move, form ("<<lastPoint.x()<<lastPoint.y()<<") to ("<<currentPoint.x()<<currentPoint.y()<<")";
            dragMoveSelectedArea();
            for(QVector<CvPoint>::iterator iter=irregularSelectionPoints.begin();iter!=irregularSelectionPoints.end();iter++){
                (*iter).x+=currentPoint.x()-lastPoint.x();
                (*iter).y+=currentPoint.y()-lastPoint.y();
            }
        }
        else if(lastPoint.x()<vertexLeftTop.x-5 || lastPoint.x()>vertexRightBottom.x+5
                || lastPoint.y()<vertexLeftTop.y-5 || lastPoint.y()>vertexRightBottom.y+5){
            QPoint lfirstPoint;
            lfirstPoint.setX(firstPoint.x()-(imageCentralPoint.x()-imageStackDisplay[currentImageNum].width()/2));
            lfirstPoint.setY(firstPoint.y()-(imageCentralPoint.y()-imageStackDisplay[currentImageNum].height()/2));
            rotateSelectedArea(lfirstPoint,lastPoint);
//            qDebug()<<"Out, rotate, form ("<<lastPoint.x()<<lastPoint.y()<<") to ("<<currentPoint.x()<<currentPoint.y()<<")";
        }
        else{
//            qDebug()<<"On, transform, form ("<<lastPoint.x()<<lastPoint.y()<<") to ("<<currentPoint.x()<<currentPoint.y()<<")";
            dragMoveSelectedArea();
            for(QVector<CvPoint>::iterator iter=irregularSelectionPoints.begin();iter!=irregularSelectionPoints.end();iter++){
                (*iter).x+=currentPoint.x()-lastPoint.x();
                (*iter).y+=currentPoint.y()-lastPoint.y();
            }
        }

        setTransformSelectionState();

        break;
    }
    default:
        break;
    }

    //ALL CHANGE MADE TO IMAGE MUST CALL THIS TO DISPALY
    updateDisplay(currentImageNum);
}

/// @param [in] currentPoint The point the mouse now is
void ScribbleArea::ApplyToolFunction(QPoint currentPoint)
{
    switch(toolType){
    case ToolType::Erase:
        vertexLeftTop.x=currentPoint.rx() - eraseToolFunction->getEraseSize()/2;
        vertexLeftTop.y=currentPoint.ry() - eraseToolFunction->getEraseSize()/2;
        vertexRightBottom.x=vertexLeftTop.x + eraseToolFunction->getEraseSize();
        vertexRightBottom.y=vertexLeftTop.y + eraseToolFunction->getEraseSize();
        ApplyToolFunction();
        break;
    default:
        break;
    }

    //ALL CHANGE MADE TO IMAGE MUST CALL THIS TO DISPALY
    //emit
    updateDisplay(currentImageNum);
}

void ScribbleArea::ApplyToolFunction()
{
    switch (toolType) {

    default:
        break;
    }

    updateDisplay(currentImageNum);
}

/// @param [in] lastPoint The first point of a line
/// @param [in] currentPoinr The second point of a line
void ScribbleArea::drawLineTo(QPoint lastPoint, QPoint currentPoint)
{
    //Mat image = Mat::zeros(height, width, CV_8UC3);
    //Mat image(imageStack[currentImageNum]);

    cv::Point pt1, pt2;

    pt1.x=lastPoint.x();
    pt1.y=lastPoint.y();
    pt2.x=currentPoint.x();
    pt2.y=currentPoint.y();

    QColor color;
    int size;
    int lineType; // change it to 8 to see non-antialiased graphics
    switch(toolType)
    {
    case ToolType::Brush:{
        color=fgColor;
        size=brushToolFunction->getBrushSize();
        if(brushToolFunction->getAntiAliasing())
            lineType = CV_AA;
        else
            lineType = 8;
        break;
    }
    case ToolType::Erase:{
        color=bgColor;
        size=eraseToolFunction->getEraseSize();
        lineType = CV_AA;
        break;
    }
    default:{
        lineType = CV_AA;
        color=fgColor;
        size=5;
        break;
    }
    }


    //line( image, pt1, pt2,  Scalar(icolor&255, (icolor>>8)&255, (icolor>>16)&255), rng.uniform(1,10), lineType );
    cvLine(imageStackEdit[currentImageNum], pt1, pt2,
           cvScalar(color.blue(),color.green(), color.red(), color.alpha()), size, lineType);
    /// cvScalar(b, g, r, a);
    return;
}

void ScribbleArea::deleteSelectedArea()
{
    if(totalImageNum<=0) return;

    if(somethingSelected == false) {return;}

    switch (selectionType){
    case TwoPointsSelection:
        cvRectangle(imageStackEdit[currentImageNum], vertexLeftTop, vertexRightBottom, CV_RGB(255,255,255), -1);
        break;
    case PolygonSelection:
        foreach(CvPoint tmp, irregularSelectionPoints){
            qDebug()<<"("<<tmp.x<<","<<tmp.y<<")";
        }

        break;
    }

    updateDisplay(currentImageNum);
}

void ScribbleArea::strokeSelectedArea(void)
{
    if(totalImageNum<=0) return;

    if(somethingSelected == false) return;


    int size=3;
    int lineType=CV_AA;

    for(int i=0;i<irregularSelectionPoints.length()-1;i++){
        cvLine(imageStackEdit[currentImageNum], irregularSelectionPoints[i], irregularSelectionPoints[i+1],
               cvScalar(fgColor.blue(),fgColor.green(), fgColor.red(), fgColor.alpha()), size, lineType);
    }
    cvLine(imageStackEdit[currentImageNum], irregularSelectionPoints[irregularSelectionPoints.length()-1],
            irregularSelectionPoints[0],cvScalar(fgColor.blue(),fgColor.green(), fgColor.red(), fgColor.alpha()), size, lineType);

    updateDisplay(currentImageNum);
}

void ScribbleArea::fillSelectedArea(void)
{
    if(totalImageNum<=0) return;

    if(somethingSelected == false) return;

    Ipl2Mat();
    drawMask();
    tmpImage.setTo(cv::Scalar(fgColor.blue(),fgColor.green(),fgColor.red()));
    Mat2Ipl();

    updateDisplay(currentImageNum);
}




void ScribbleArea::blackAndWhite(void){
    if(totalImageNum<=0) return;

    if(somethingSelected == false) return;

    Ipl2Mat();
    drawMask();
    cv::cvtColor(tmpImage,tmpImage,CV_BGR2GRAY);
    cv::cvtColor(tmpImage,tmpImage,CV_GRAY2BGR);
    Mat2Ipl();

    updateDisplay(currentImageNum);
}

void ScribbleArea::gaussianBlur(void){
    if(totalImageNum<=0) return;

    Ipl2Mat();
    drawMask();

    QDialog getSizeDialog(this);
    getSizeDialog.setWindowTitle("Gaussian Blur");

    QGridLayout *dialogLayout = new QGridLayout(&getSizeDialog);
    dialogLayout->addWidget(new QLabel(tr("Input size: "), &getSizeDialog),0, 0);
    QSpinBox *sizeInputBox = new QSpinBox;
    sizeInputBox->setValue(5);    /// @note Value step need to be set
    sizeInputBox->setRange(0,100);   /// @note Range need to be set
    dialogLayout->addWidget(sizeInputBox,0,1);

    QPushButton *okButton = new QPushButton(tr("Ok"), &getSizeDialog);
    QPushButton *cancelButton = new QPushButton(tr("Cancel"), &getSizeDialog);
    connect(okButton, SIGNAL(clicked()), &getSizeDialog, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), &getSizeDialog, SLOT(reject()));
    okButton->setDefault(true);
    dialogLayout->addWidget(cancelButton);
    dialogLayout->addWidget(okButton);

    if(!getSizeDialog.exec()) return;

    int size=sizeInputBox->value();

    cv::GaussianBlur( tmpImage, tmpImage, cv::Size( size*2+1, size*2+1 ), 0, 0 );
    Mat2Ipl();

    updateDisplay(currentImageNum);
}

void ScribbleArea::cannyEdge(void){
    if(totalImageNum<=0) return;

    Ipl2Mat();
    drawMask();

    QDialog getSizeDialog(this);
    getSizeDialog.setWindowTitle("Canny Edge");

    QGridLayout *dialogLayout = new QGridLayout(&getSizeDialog);
    dialogLayout->addWidget(new QLabel(tr("Input threshold: "), &getSizeDialog),0, 0);
    QSpinBox *sizeInputBox = new QSpinBox;
    sizeInputBox->setValue(50);
    sizeInputBox->setRange(0,100);  /// @note Range need to be set
    dialogLayout->addWidget(sizeInputBox,0,1);

    QPushButton *okButton = new QPushButton(tr("Ok"), &getSizeDialog);
    QPushButton *cancelButton = new QPushButton(tr("Cancel"), &getSizeDialog);
    connect(okButton, SIGNAL(clicked()), &getSizeDialog, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), &getSizeDialog, SLOT(reject()));
    okButton->setDefault(true);
    dialogLayout->addWidget(cancelButton);
    dialogLayout->addWidget(okButton);

    if(!getSizeDialog.exec()) return;

    int threshold=sizeInputBox->value();

    cv::Canny(tmpImage,tmpImage,threshold,threshold*3);
    cv::cvtColor(tmpImage,tmpImage,CV_GRAY2BGR);
//    qDebug()<<tmpImage.channels();
    Mat2Ipl();

    updateDisplay(currentImageNum);
}

void ScribbleArea::erodeFilter(void){
    if(totalImageNum<=0) return;

    Ipl2Mat();
    drawMask();

    QDialog getSizeDialog(this);
    getSizeDialog.setWindowTitle("Eroder Filter");

    QGridLayout *dialogLayout = new QGridLayout(&getSizeDialog);
    dialogLayout->addWidget(new QLabel(tr("Input size: "), &getSizeDialog),0, 0);
    QSpinBox *sizeInputBox = new QSpinBox;
    sizeInputBox->setValue(1);
    sizeInputBox->setRange(0,100);  /// @note Range need to be set
    dialogLayout->addWidget(sizeInputBox,0,1);

    QPushButton *okButton = new QPushButton(tr("Ok"), &getSizeDialog);
    QPushButton *cancelButton = new QPushButton(tr("Cancel"), &getSizeDialog);
    connect(okButton, SIGNAL(clicked()), &getSizeDialog, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), &getSizeDialog, SLOT(reject()));
    okButton->setDefault(true);
    dialogLayout->addWidget(cancelButton);
    dialogLayout->addWidget(okButton);

    if(!getSizeDialog.exec()) return;

    int size=sizeInputBox->value();


    cv::Mat element = cv::getStructuringElement( cv::MORPH_RECT,
                                                 cv::Size( 2*size + 1, 2*size+1 ),
                                                 cv::Point( size, size ) );
    cv::erode(tmpImage,tmpImage,element);
    Mat2Ipl();

    updateDisplay(currentImageNum);
}

void ScribbleArea::dilateFilter(void){
    if(totalImageNum<=0) return;

    Ipl2Mat();
    drawMask();

    QDialog getSizeDialog(this);
    getSizeDialog.setWindowTitle("Dilate Filter");

    QGridLayout *dialogLayout = new QGridLayout(&getSizeDialog);
    dialogLayout->addWidget(new QLabel(tr("Input size: "), &getSizeDialog),0, 0);
    QSpinBox *sizeInputBox = new QSpinBox;
    sizeInputBox->setValue(1);
    sizeInputBox->setRange(0,100);  /// @note Range need to be set
    dialogLayout->addWidget(sizeInputBox,0,1);

    QPushButton *okButton = new QPushButton(tr("Ok"), &getSizeDialog);
    QPushButton *cancelButton = new QPushButton(tr("Cancel"), &getSizeDialog);
    connect(okButton, SIGNAL(clicked()), &getSizeDialog, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), &getSizeDialog, SLOT(reject()));
    okButton->setDefault(true);
    dialogLayout->addWidget(cancelButton);
    dialogLayout->addWidget(okButton);

    if(!getSizeDialog.exec()) return;

    int size=sizeInputBox->value();


    cv::Mat element = cv::getStructuringElement( cv::MORPH_RECT,
                                                 cv::Size( 2*size + 1, 2*size+1 ),
                                                 cv::Point( size, size ) );
    cv::dilate(tmpImage,tmpImage,element);
    Mat2Ipl();

    updateDisplay(currentImageNum);
}

void ScribbleArea::grabcutFilter(void){
    if(totalImageNum<=0) return;

    Ipl2Mat();
    drawMask(cv::GC_PR_FGD);

    cv::Mat bkgModel,fgrModel;

    cv::grabCut(tmpImage,  // input image
                mask, // segmentation result
                cv::Rect(),bkgModel,fgrModel,3,cv::GC_INIT_WITH_MASK);
    mask=mask&1;

    cv::Mat tmp=cv::Mat(imageStackEdit[currentImageNum]);
    tmp.setTo(cv::Scalar(bgColor.blue(),bgColor.green(),bgColor.red()));
    IplImage tmpIpl=tmp;
    cvConvertImage(&tmpIpl,imageStackEdit[currentImageNum]);

    Mat2Ipl();

    updateDisplay(currentImageNum);
}

void ScribbleArea::Ipl2Mat(){
    cv::Mat tmp=cv::Mat(imageStackEdit[currentImageNum]);
    tmpImage=tmp.clone();
}

/// @param [in] The value of the mask
void ScribbleArea::drawMask(int value){
    mask=cv::Mat(imageStackEdit[currentImageNum]->height,imageStackEdit[currentImageNum]->width, CV_8U);
    mask.setTo(0);
    if(somethingSelected == false){
        mask.setTo(value);
    }
    else{
        if(selectionType==TwoPointsSelection){
            readjustRect();
            cv::rectangle(mask,cv::Point(vertexLeftTop),cv::Point(vertexRightBottom),value,-1);
        }
        else{
//            foreach(CvPoint tmp, irregularSelectionPoints){
//                qDebug()<<"("<<tmp.x<<","<<tmp.y<<")";
//            }

            cv::Point *points = new cv::Point[irregularSelectionPoints.length()];
            for(int i=0;i<irregularSelectionPoints.length();i++){
                points[i]=irregularSelectionPoints[i];
            }
            const cv::Point* ppt[1] = { points };
            int npt[] = { irregularSelectionPoints.length() };

            cv::fillPoly( mask,
                      ppt,
                      npt,
                      1,
                      value);

            delete[] points;


            /*
            cv::Point points[1][irregularSelectionPoints.length()];
            for(int i=0;i<irregularSelectionPoints.length();i++){
                points[0][i]=irregularSelectionPoints[i];
            }
            const cv::Point* ppt[1] = { points[0] };
            int npt[] = { irregularSelectionPoints.length() };

            cv::fillPoly( mask,
                      ppt,
                      npt,
                      1,
                      value);
                      */
        }
    }
}

void ScribbleArea::Mat2Ipl(){
    cv::Mat tmp=cv::Mat(imageStackEdit[currentImageNum]);
    tmpImage.copyTo(tmp,mask);
    IplImage tmpIpl=tmp;
    cvConvertImage(&tmpIpl,imageStackEdit[currentImageNum]);
}

void ScribbleArea::dragMoveSelectedArea()
{
    qDebug()<<"drag";
    if(isMousePressed==true){ // display move
        drawMask();
        if(originalImageSaved==false){
            qDebug()<<"set originalImageSaved";
            Ipl2Mat();
            partImage=cv::Mat(tmpImage,cv::Rect(vertexLeftTop.x,vertexLeftTop.y,
                                                vertexRightBottom.x-vertexLeftTop.x,
                                                vertexRightBottom.y-vertexLeftTop.y)).clone();
            originalImage=tmpImage.clone();
            tmpImage.setTo(cv::Scalar(bgColor.blue(),bgColor.green(),bgColor.red()));
            tmpImage.copyTo(originalImage,mask);

            originalImageSaved=true;
        }
        cv::Mat tmp(tmpImage.rows,tmpImage.cols,tmpImage.type());
        cv::Rect rect(vertexLeftTop.x,vertexLeftTop.y,
                      vertexRightBottom.x-vertexLeftTop.x,
                      vertexRightBottom.y-vertexLeftTop.y);
        cv::Rect rectPart(0,0,rect.width,rect.height);
        if(rect.x<0){
            rect.width+=rect.x;
            rectPart.width=rect.width;
            rectPart.x=-rect.x;
            rect.x=0;
        }
        if(rect.y<0){
            rect.height+=rect.y;
            rectPart.height=rect.height;
            rectPart.y=-rect.y;
            rect.y=0;
        }
        if(rect.x+rect.width>imageStackEdit[currentImageNum]->width-1){
            rect.width=imageStackEdit[currentImageNum]->width-1-rect.x;
            rectPart.width=rect.width;
        }
        if(rect.y+rect.height>imageStackEdit[currentImageNum]->height-1){
            rect.height=imageStackEdit[currentImageNum]->height-1-rect.y;
            rectPart.height=rect.height;
        }
        cv::Mat(partImage,rectPart).copyTo(cv::Mat(tmp,rect));
        tmpImage=originalImage.clone();
        tmp.copyTo(tmpImage,mask);
        IplImage tmpIpl=tmpImage;
        cvConvertImage(&tmpIpl,imageStackEdit[currentImageNum]);
    }
    else  //actual move
        originalImageSaved=false;
    updateDisplay(currentImageNum);
}

/// @param [in] lastPoint The point where the mouse is last time
/// @param [in] currentPoint The point where the mouse now is
void ScribbleArea::rotateSelectedArea(QPoint firstPoint,QPoint lastPoint)
{
    qDebug()<<"rotate";
    if(isMousePressed==true){ // display move
        if(originalImageSaved==false){
            qDebug()<<"set originalImageSaved";
            Ipl2Mat();
            drawMask();
            partImage=cv::Mat(tmpImage,cv::Rect(vertexLeftTop.x,vertexLeftTop.y,
                                                vertexRightBottom.x-vertexLeftTop.x,
                                                vertexRightBottom.y-vertexLeftTop.y)).clone();
            partMask=cv::Mat(mask,cv::Rect(vertexLeftTop.x,vertexLeftTop.y,
                                                vertexRightBottom.x-vertexLeftTop.x,
                                                vertexRightBottom.y-vertexLeftTop.y)).clone();
//            cv::imshow("partmask",partMask);
//            cv::waitKey();
            originalImage=tmpImage.clone();
            tmpImage.setTo(cv::Scalar(bgColor.blue(),bgColor.green(),bgColor.red()));
            tmpImage.copyTo(originalImage,mask);

            originalImageSaved=true;
        }
        int midX=(vertexLeftTop.x+vertexRightBottom.x)/2;
        int midY=(vertexLeftTop.y+vertexRightBottom.y)/2;
//        qDebug()<<midX<<" "<<midY;
//        qDebug()<<firstPoint<<lastPoint;
        double cosValue=(double)((firstPoint.x()-midX)*(lastPoint.x()-midX)+
                                 (firstPoint.y()-midY)*(lastPoint.y()-midY))/
                        (double)(sqrt((firstPoint.x()-midX)*(firstPoint.x()-midX)+
                                      (firstPoint.y()-midY)*(firstPoint.y()-midY))*
                                 sqrt((lastPoint.x()-midX)*(lastPoint.x()-midX)+
                                      (lastPoint.y()-midY)*(lastPoint.y()-midY)));
        if(cosValue>-1.0&&cosValue<1.0){
            double angle=acos(cosValue);
            if(lastPoint.y()>((double)(firstPoint.y()-midY)/(double)(firstPoint.x()-midX)*
                    (lastPoint.x()-midX)+midY))
                angle=-angle;
            angle=-angle;
            qDebug()<<angle;

            IplImage partImageIpl=partImage;
            IplImage partMaskIpl=partMask;
            cv::Mat partImageRotate=cv::Mat(rotateImage2(&partImageIpl,angle));
            cv::Mat partMaskRotate=cv::Mat(rotateImage2(&partMaskIpl,angle));

            cv::Rect rect(midX-partImageRotate.cols/2,
                          midY-partImageRotate.rows/2,
                          partImageRotate.cols,
                          partImageRotate.rows);
            cv::Rect rectPart(0,0,rect.width,rect.height);

            if(rect.x<0){
                rect.width+=rect.x;
                rectPart.width=rect.width;
                rectPart.x=-rect.x;
                rect.x=0;
            }
            if(rect.y<0){
                rect.height+=rect.y;
                rectPart.height=rect.height;
                rectPart.y=-rect.y;
                rect.y=0;
            }
            if(rect.x+rect.width>imageStackEdit[currentImageNum]->width-1){
                rect.width=imageStackEdit[currentImageNum]->width-1-rect.x;
                rectPart.width=rect.width;
            }
            if(rect.y+rect.height>imageStackEdit[currentImageNum]->height-1){
                rect.height=imageStackEdit[currentImageNum]->height-1-rect.y;
                rectPart.height=rect.height;
            }
            tmpImage=originalImage.clone();
            cv::Mat(partImageRotate,rectPart).copyTo(cv::Mat(tmpImage,rect),cv::Mat(partMaskRotate,rectPart));
            IplImage tmpIpl=tmpImage;
            cvConvertImage(&tmpIpl,imageStackEdit[currentImageNum]);
        }

    }
    else  //actual move
        originalImageSaved=false;
    updateDisplay(currentImageNum);
}

/// @param [in] img The image to be rotated
/// @param [in] anle The angle to rotate
IplImage* ScribbleArea::rotateImage2(IplImage* img, double angle)
{
    double a = sin(angle), b = cos(angle);
    int width=img->width, height=img->height;
    int width_rotate= int(height * fabs(a) + width * fabs(b));
    int height_rotate=int(width * fabs(a) + height * fabs(b));
    IplImage* img_rotate = cvCreateImage(cvSize(width_rotate, height_rotate), img->depth, img->nChannels);
    cvZero(img_rotate);
    int tempLength = sqrt((double)width * width + (double)height *height) + 10;
    int tempX = (tempLength + 1) / 2 - width / 2;
    int tempY = (tempLength + 1) / 2 - height / 2;
    IplImage* temp = cvCreateImage(cvSize(tempLength, tempLength), img->depth, img->nChannels);
    cvZero(temp);
    cvSetImageROI(temp, cvRect(tempX, tempY, width, height));
    cvCopy(img, temp, NULL);
    cvResetImageROI(temp);
    float m[6];
    int w = temp->width;
    int h = temp->height;
    m[0] = b;
    m[1] = a;
    m[3] = -m[1];
    m[4] = m[0];
    m[2] = w * 0.5f;
    m[5] = h * 0.5f;
    CvMat M = cvMat(2, 3, CV_32F, m);
    cvGetQuadrangleSubPix(temp, img_rotate, &M);
    cvReleaseImage(&temp);
    return img_rotate;
}
