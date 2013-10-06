/*! @file v4l2wrapper.h
 */
#ifndef V4L2WRAPPER_H
#define V4L2WRAPPER_H

#if defined(__cplusplus)
extern "C"
{
#endif

#include <linux/videodev2.h>
#include <stdio.h>

enum V4L2W_IMGFORMAT {
    V4L2W_IMGFORMAT_GREY,
    V4L2W_IMGFORMAT_BGR24,
    V4L2W_IMGFORMAT_MJPEG,
    V4L2W_IMGFORMAT_H264
};

struct V4L2W_HANDLER;
typedef struct V4L2W_HANDLER V4L2WHandler_t;

struct V4L2W_IMGPARAM {
	int width;
	int height;
	int pix_fmt;
	int img_fmt;
	int fps;
};

V4L2WHandler_t * V4L2W_initialize(const char * device_name0, int width, int height, int fps, int output_format, int capture_format);
V4L2WHandler_t * V4L2W_init_w_param(const char * device_name0, struct V4L2W_IMGPARAM imgparam);
void * V4L2W_capture(V4L2WHandler_t * handle);
int V4L2W_output_stream(V4L2WHandler_t * handle, int fp);
void V4L2W_finalize(V4L2WHandler_t * handle);
int V4L2W_getctrl(V4L2WHandler_t * handle, int type);
void V4L2W_setctrl(V4L2WHandler_t * handle, int type, int value);
void V4L2W_settpf(V4L2WHandler_t * handle, int num, int denom);
void V4L2W_gettpf(V4L2WHandler_t * handle, int * num, int * denom);
void V4L2W_suspend(V4L2WHandler_t * handle);
void V4L2W_resume(V4L2WHandler_t * handle);

#if defined(__cplusplus)
}
#endif

#endif // V4L2WRAPPER_H

