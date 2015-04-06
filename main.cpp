#include <QApplication>
#include <QSplashScreen>
#include <QThread>


#include "mainwindow.h"

#if 1
int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(resources);

    QApplication app(argc, argv);
//    QSplashScreen *splash = new QSplashScreen;
//    splash->setPixmap(QPixmap(":images/bg.png"));
//    splash->show();
    app.setOrganizationName("PMIG Project");
    app.setApplicationName("PMIG");
    MainWindow mainWin;
    //mainWin.resize(1024,576);
    mainWin.setWindowState(Qt::WindowMaximized);
    mainWin.show();
    return app.exec();
}
#endif




#if 0

/********************************************************************************
*
*
*  This program is demonstration for ellipse fitting. Program finds
*  contours and approximate it by ellipses.
*
*  Trackbar specify threshold parametr.
*
*  White lines is contours. Red lines is fitting ellipses.
*
*
*  Autor:  Denis Burenkov.
*
*
*
********************************************************************************/
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
using namespace cv;
using namespace std;

// static void help()
// {
//     cout <<
//             "\nThis program is demonstration for ellipse fitting. The program finds\n"
//             "contours and approximate it by ellipses.\n"
//             "Call:\n"
//             "./fitellipse [image_name -- Default ../data/stuff.jpg]\n" << endl;
// }

int sliderPos = 70;

Mat image;

void processImage(int, void*);

int main( int argc, char** argv )
{
    const char* filename = argc == 2 ? argv[1] : (char*)"/Users/ying/百度云同步盘/Courses/OpenCV/cell3.jpg";
    image = imread(filename, 0);
    if( image.empty() )
    {
        cout << "Couldn't open image " << filename << "\nUsage: fitellipse <image_name>\n";
        return 0;
    }

    imshow("source", image);
    namedWindow("result", 1);

    // Create toolbars. HighGUI use.
    createTrackbar( "threshold", "result", &sliderPos, 255, processImage );
    processImage(0, 0);

    // Wait for a key stroke; the same function arranges events processing
    waitKey();
    return 0;
}

// Define trackbar callback functon. This function find contours,
// draw it and approximate it by ellipses.
void processImage(int /*h*/, void*)
{
    vector<vector<Point> > contours;
    Mat bimage = image >= sliderPos;

    findContours(bimage, contours, RETR_LIST, CHAIN_APPROX_NONE);

    Mat cimage = Mat::zeros(bimage.size(), CV_8UC3);

    for(size_t i = 0; i < contours.size(); i++)
    {
        size_t count = contours[i].size();
        if( count < 6 )
            continue;

        Mat pointsf;
        Mat(contours[i]).convertTo(pointsf, CV_32F);
        RotatedRect box = fitEllipse(pointsf);

        if( MAX(box.size.width, box.size.height) > MIN(box.size.width, box.size.height)*30 )
            continue;
        drawContours(cimage, contours, (int)i, Scalar::all(255), 1, 8);

        ellipse(cimage, box, Scalar(0,0,255), 1, LINE_AA);
        ellipse(cimage, box.center, box.size*0.5f, box.angle, 0, 360, Scalar(0,255,255), 1, LINE_AA);
        Point2f vtx[4];
        box.points(vtx);
        for( int j = 0; j < 4; j++ )
            line(cimage, vtx[j], vtx[(j+1)%4], Scalar(0,255,0), 1, LINE_AA);
    }

    imshow("result", cimage);
}



#endif

#if 0
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>

using namespace cv;
using namespace std;


//floodfill()
//Fills a connected component with the given color.

static void help()
{
    cout << "\nThis program demonstrated the floodFill() function\n"
        "Call:\n"
        "./ffilldemo [image_name -- Default: fruits.jpg]\n" << endl;

    cout << "Hot keys: \n"
        "\tESC - quit the program\n"
        "\tc - switch color/grayscale mode\n"
        "\tm - switch mask mode\n"
        "\tr - restore the original image\n"
        "\ts - use null-range floodfill\n"
        "\tf - use gradient floodfill with fixed(absolute) range\n"
        "\tg - use gradient floodfill with floating(relative) range\n"
        "\t4 - use 4-connectivity mode\n"
        "\t8 - use 8-connectivity mode\n" << endl;
}

Mat image0, image, gray, mask;
int ffillMode = 1;
int loDiff = 20, upDiff = 20;
int connectivity = 4;
int isColor = true;
bool useMask = false;
int newMaskVal = 255;

static void onMouse( int event, int x, int y, int, void* )
{
    if( event != CV_EVENT_LBUTTONDOWN )
        return;

    Point seed = Point(x,y);
    int lo = ffillMode == 0 ? 0 : loDiff;
    int up = ffillMode == 0 ? 0 : upDiff;
    int flags = connectivity + (newMaskVal << 8) +
        (ffillMode == 1 ? CV_FLOODFILL_FIXED_RANGE : 0);
    int b = (unsigned)theRNG() & 255;
    int g = (unsigned)theRNG() & 255;
    int r = (unsigned)theRNG() & 255;
    Rect ccomp;

    Scalar newVal = isColor ? Scalar(b, g, r) : Scalar(r*0.299 + g*0.587 + b*0.114);
    Mat dst = isColor ? image : gray;
    int area;

    if( useMask )
    {
        threshold(mask, mask, 1, 128, CV_THRESH_BINARY);
        area = floodFill(dst, mask, seed, newVal, &ccomp, Scalar(lo, lo, lo),
            Scalar(up, up, up), flags);
        imshow( "mask", mask );
    }
    else
    {
        area = floodFill(dst, seed, newVal, &ccomp, Scalar(lo, lo, lo),
            Scalar(up, up, up), flags);
    }

    imshow("image", dst);
    cout << area << " pixels were repainted\n";
}


int main( )
{
    char* filename="/Users/ying/Italy4.jpg";
    image0 = imread(filename, 1);

    if( image0.empty() )
    {
        cout << "Image empty. Usage: ffilldemo <image_name>\n";
        return 0;
    }
    help();
    image0.copyTo(image);
    cvtColor(image0, gray, CV_BGR2GRAY);
    mask.create(image0.rows+2, image0.cols+2, CV_8UC1);

    namedWindow( "image", 0 );
    createTrackbar( "lo_diff", "image", &loDiff, 255, 0 );
    createTrackbar( "up_diff", "image", &upDiff, 255, 0 );

    setMouseCallback( "image", onMouse, 0 );

    for(;;)
    {
        imshow("image", isColor ? image : gray);

        int c = waitKey(0);
        if( (c & 255) == 27 )
        {
            cout << "Exiting ...\n";
            break;
        }
        switch( (char)c )
        {
        case 'c':
            if( isColor )
            {
                cout << "Grayscale mode is set\n";
                cvtColor(image0, gray, CV_BGR2GRAY);
                mask = Scalar::all(0);
                isColor = false;
            }
            else
            {
                cout << "Color mode is set\n";
                image0.copyTo(image);
                mask = Scalar::all(0);
                isColor = true;
            }
            break;
        case 'm':
            if( useMask )
            {
                destroyWindow( "mask" );
                useMask = false;
            }
            else
            {
                namedWindow( "mask", 0 );
                mask = Scalar::all(0);
                imshow("mask", mask);
                useMask = true;
            }
            break;
        case 'r':
            cout << "Original image is restored\n";
            image0.copyTo(image);
            cvtColor(image, gray, CV_BGR2GRAY);
            mask = Scalar::all(0);
            break;
        case 's':
            cout << "Simple floodfill mode is set\n";
            ffillMode = 0;
            break;
        case 'f':
            cout << "Fixed Range floodfill mode is set\n";
            ffillMode = 1;
            break;
        case 'g':
            cout << "Gradient (floating range) floodfill mode is set\n";
            ffillMode = 2;
            break;
        case '4':
            cout << "4-connectivity mode is set\n";
            connectivity = 4;
            break;
        case '8':
            cout << "8-connectivity mode is set\n";
            connectivity = 8;
            break;
        }
    }

    return 0;
}

#endif
