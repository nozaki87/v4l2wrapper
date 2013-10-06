#include <stdio.h>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#if !defined(WIN32) && !defined(__APPLE__)
# include "v4l2c.h"
#endif

static int check_break (void)
{
    unsigned char c;
    c = cvWaitKey (3);
    if (c == '\x20') c = cv::waitKey(0);
    if (c == '\x1b') return 1;
    else return 0;
}

#define VIDEO_WIDTH  1920
#define VIDEO_HEIGHT 1080

int main(int argc, char ** argv) 
{
    cv::Mat inputimage;
#if defined(WIN32) || defined(__APPLE__)
    cv::VideoCapture VC;
#else
    V4L2C VC;
#endif

    VC.open(0);
    VC.set(CV_CAP_PROP_FRAME_WIDTH, VIDEO_WIDTH);
    VC.set(CV_CAP_PROP_FRAME_HEIGHT, VIDEO_HEIGHT);
    VC.set(CV_CAP_PROP_FPS, 30);
    VC.mV4L2WImgParam.pix_fmt = V4L2_PIX_FMT_MJPEG;

    while (1) {
        VC.read(inputimage);
        cv::imshow("Input Image", inputimage);
        if (check_break()) break;
    }

    VC.release();

    return 0;
}

