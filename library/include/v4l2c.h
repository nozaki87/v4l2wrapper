#ifndef V4L2C_H
#define V4L2C_H

#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "v4l2w.h"

class V4L2C {
public:
    V4L2WHandler_t * mV4l2w;
    char mDevname[256];
    struct V4L2W_IMGPARAM mV4L2WImgParam;
    // int mWidth;
    // int mHeight;
    V4L2C() {
        mV4L2WImgParam.width = 640;
        mV4L2WImgParam.height = 480;
        mV4L2WImgParam.fps = 30;
        mV4L2WImgParam.img_fmt = V4L2W_IMGFORMAT_BGR24;
        mV4L2WImgParam.pix_fmt = V4L2_PIX_FMT_YUYV;
        mV4l2w = NULL;
    }
    V4L2C(int dev) {
        V4L2C();
        open(dev);
    }
    ~V4L2C() {
        //release();
    }
    void open(int dev) {
        release();
    	sprintf(mDevname, "/dev/video%d", dev);
    }
    void release() {
        if (mV4l2w) V4L2W_finalize(mV4l2w);
	mV4l2w = NULL;
    }
    void read(cv::Mat & image) {
        if (mV4l2w == NULL) {
            mV4l2w = V4L2W_init_w_param(mDevname, mV4L2WImgParam);
        }
        image = cv::Mat(mV4L2WImgParam.height, mV4L2WImgParam.width, CV_8UC3, V4L2W_capture(mV4l2w));
    }
    int output_stream_stdout() {
        if (mV4l2w == NULL) {
            mV4l2w = V4L2W_init_w_param(mDevname, mV4L2WImgParam);
        }
        V4L2W_output_stream(mV4l2w, 1);
    }
    void set(int idx, int val) {
        switch (idx) {
	case CV_CAP_PROP_FRAME_WIDTH:
	    mV4L2WImgParam.width = val;
	    break;
	case CV_CAP_PROP_FRAME_HEIGHT:
	    mV4L2WImgParam.height = val;
	    break;
	case CV_CAP_PROP_FPS:
	    mV4L2WImgParam.fps = val;
	    break;
	}
    }
    int  get(int idx) {

    }
    int grab() {

    }
    int retrieve(cv::Mat & image) {
        read(image);
    }
    void set_v4l2p(int idx, int val){
        V4L2W_setctrl(mV4l2w, idx, val);
    }
    int get_v4l2p(int idx){
        return V4L2W_getctrl(mV4l2w, idx);
    }
    void suspend_v4l2(){
        V4L2W_suspend(mV4l2w);
    }
    void resume_v4l2(){
    	V4L2W_resume(mV4l2w);
    }
};

#endif // V4L2C_H
