/*
 *  @file v4l2wrapper.c
 *  @brief Wrapper functions for easy use of V4L2 devices.
 *         This can capture frame like query frame in OpenCV.
 */

/**********************************
 * Header files
 **********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>             /* getopt_long() */

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <asm/types.h>          /* for videodev2.h */

#include <linux/videodev2.h>

#include "v4l2w.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))

#define USE_MJPEG

/**********************************
 * Definitions
 **********************************/
typedef enum {
        IO_METHOD_READ,
        IO_METHOD_MMAP,
        IO_METHOD_USERPTR,
} io_method;

struct buffer {
        void *                  start;
        size_t                  length;
};

struct V4L2W_HANDLER {
    char *           dev_name;
    io_method        io;
    int              fd;
    struct buffer *  buffers;
    unsigned int     n_buffers;
    char             dev_name_tmp[512];
    // image param
    struct V4L2W_IMGPARAM imgparam;
    // image output
    unsigned char *  outimg;
    // file descriptor output
    int outputfd;
    // data size of current frame.
    int bytes_in_frame;
};

/**********************************
 * Transformation of Image format
 **********************************/
#if defined(USE_MJPEG)
#include <jpeghelper.h>
static void trans_mjpeg2bgr(const unsigned char * src, unsigned char * dst, int width, int height, int srcsize)
{
    unsigned char * bmp;
    unsigned char * jpegbuff;
    jpegbuff = malloc(srcsize + 0x1a4);
    int newsrcsize = mjpeg2jpeg(jpegbuff, src, srcsize);
    bmp = load_memory_jpg(jpegbuff, newsrcsize);
    memcpy(dst, bmp, width * height * 3);
    free(jpegbuff);
    free(bmp);
}
#endif // USE_MJPEG

static void trans_yuyv2bgr(const unsigned char * src, unsigned char * dst, int width, int height, int srcsize)
{
    int y, x;
    int b, g, r;
    float y0, y1, cb, cr;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width/2; x++) {
            y0 = *src++;
            cb = *src++;
            y1 = *src++;
            cr = *src++;
            y0 -= 16;
            y1 -= 16;
            cb -= 128;
            cr -= 128;
            b = 1.164 * y0 + 2.018 * cb;
            if (b < 0) b = 0;
            if (b > 255) b = 255;
            *dst++ = b;
            g = 1.164 * y0 - 0.391 * cb - 0.813 * cr;
            if (g < 0) g = 0;
            if (g > 255) g = 255;
            *dst++ = g;
            r = 1.164 * y0 + 1.596 * cr;
            if (r < 0) r = 0;
            if (r > 255) r = 255;
            *dst++ = r;
            b = 1.164 * y1 + 2.018 * cb;
            if (b < 0) b = 0;
            if (b > 255) b = 255;
            *dst++ = b;
            g = 1.164 * y1 - 0.391 * cb - 0.813 * cr;
            if (g < 0) g = 0;
            if (g > 255) g = 255;
            *dst++ = g;
            r = 1.164 * y1 + 1.596 * cr;
            if (r < 0) r = 0;
            if (r > 255) r = 255;
            *dst++ = r;
        }
    }
}

static void trans_yuyv2grey(const unsigned char * src, unsigned char * dst, int width, int height, int srcsize)
{
    int y, x;
    float y0, y1, cb, cr;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width/2; x++) {
            y0 = *src++;
            cb = *src++;
            y1 = *src++;
            cr = *src++;
            *dst++ = y0;
            *dst++ = y1;
        }
    }
}

/**********************************
 * V4L2 Control functions
 **********************************/
static void
errno_exit                      (const char *           s)
{
        fprintf (stderr, "%s error %d, %s\n",
                 s, errno, strerror (errno));

        exit (EXIT_FAILURE);
}

static int
xioctl                          (int                    fd,
                                 int                    request,
                                 void *                 arg)
{
        int r;

        do r = ioctl (fd, request, arg);
        while (-1 == r && EINTR == errno);

        return r;
}

static int 
process_image                   (V4L2WHandler_t * handle, const void *           p, int size)
{
    switch (handle->imgparam.img_fmt) {
    case V4L2W_IMGFORMAT_GREY:
        switch (handle->imgparam.pix_fmt) {
        case V4L2_PIX_FMT_YUYV:
            trans_yuyv2grey(p, handle->outimg, handle->imgparam.width, handle->imgparam.height, size);
            break;
        case V4L2_PIX_FMT_MJPEG:
        default:
            fprintf(stderr, "Sorry this pix format and image format is not supported. (pix: %08x, img:%08x)\n", handle->imgparam.pix_fmt, handle->imgparam.img_fmt);
            exit(0);
        }
        break;
    case V4L2W_IMGFORMAT_BGR24:
        switch (handle->imgparam.pix_fmt) {
        case V4L2_PIX_FMT_YUYV:
            trans_yuyv2bgr(p, handle->outimg, handle->imgparam.width, handle->imgparam.height, size);
            break;
        case V4L2_PIX_FMT_MJPEG:
            trans_mjpeg2bgr(p, handle->outimg, handle->imgparam.width, handle->imgparam.height, size);
            break;
        default:
            fprintf(stderr, "Sorry this pix format and image format is not supported. (pix: %08x, img:%08x)\n", handle->imgparam.pix_fmt, handle->imgparam.img_fmt);
            exit(0);
        }
        break;
    }
    return size;
}

static int 
output2fd (V4L2WHandler_t * handle, const void * p, int size)
{
    write(handle->outputfd, p, size);
    return size;
}

static int
read_frame (V4L2WHandler_t * handle, int (*process_frame)(V4L2WHandler_t *, const void *, int))
{
        struct v4l2_buffer buf;
        unsigned int i;

        switch (handle->io) {
        case IO_METHOD_READ:
                if (-1 == read (handle->fd, handle->buffers[0].start, handle->buffers[0].length)) {
                        switch (errno) {
                        case EAGAIN:
                                return 0;

                        case EIO:
                                /* Could ignore EIO, see spec. */

                                /* fall through */

                        default:
                                errno_exit ("read");
                        }
                }

                process_frame (handle, handle->buffers[0].start, buf.bytesused);

                break;

        case IO_METHOD_MMAP:
                CLEAR (buf);

                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;

                if (-1 == xioctl (handle->fd, VIDIOC_DQBUF, &buf)) {
                        switch (errno) {
                        case EAGAIN:
                                return 0;

                        case EIO:
                                /* Could ignore EIO, see spec. */

                                /* fall through */

                        default:
                                errno_exit ("VIDIOC_DQBUF");
                        }
                }

                assert (buf.index < handle->n_buffers);

                process_frame (handle, handle->buffers[buf.index].start, buf.bytesused);
                // fprintf(stderr, "%d\n", buf.bytesused);

                if (-1 == xioctl (handle->fd, VIDIOC_QBUF, &buf))
                        errno_exit ("VIDIOC_QBUF");

                break;

        case IO_METHOD_USERPTR:
                CLEAR (buf);

                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_USERPTR;

                if (-1 == xioctl (handle->fd, VIDIOC_DQBUF, &buf)) {
                        switch (errno) {
                        case EAGAIN:
                                return 0;

                        case EIO:
                                /* Could ignore EIO, see spec. */

                                /* fall through */

                        default:
                                errno_exit ("VIDIOC_DQBUF");
                        }
                }

                for (i = 0; i < handle->n_buffers; ++i)
                        if (buf.m.userptr == (unsigned long) handle->buffers[i].start
                            && buf.length == handle->buffers[i].length)
                                break;

                assert (i < handle->n_buffers);

                process_frame (handle, (void *) buf.m.userptr, buf.bytesused);

                if (-1 == xioctl (handle->fd, VIDIOC_QBUF, &buf))
                        errno_exit ("VIDIOC_QBUF");

                break;
        }

        return 1;
}

#if 0
static void
mainloop                        (void)
{
        unsigned int count;

        count = 100;

        while (count-- > 0) {
                for (;;) {
                        fd_set fds;
                        struct timeval tv;
                        int r;

                        FD_ZERO (&fds);
                        FD_SET (fd, &fds);

                        /* Timeout. */
                        tv.tv_sec = 2;
                        tv.tv_usec = 0;

                        r = select (fd + 1, &fds, NULL, NULL, &tv);

                        if (-1 == r) {
                                if (EINTR == errno)
                                        continue;

                                errno_exit ("select");
                        }

                        if (0 == r) {
                                fprintf (stderr, "select timeout\n");
                                exit (EXIT_FAILURE);
                        }

                        if (read_frame ())
                                break;
        
                        /* EAGAIN - continue select loop. */
                }
        }
}
#endif

static void
stop_capturing                  (V4L2WHandler_t * handle)
{
        enum v4l2_buf_type type;

        switch (handle->io) {
        case IO_METHOD_READ:
                /* Nothing to do. */
                break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

                if (-1 == xioctl (handle->fd, VIDIOC_STREAMOFF, &type))
                        errno_exit ("VIDIOC_STREAMOFF");

                break;
        }
}

static void
start_capturing                 (V4L2WHandler_t * handle)
{
        unsigned int i;
        enum v4l2_buf_type type;

        switch (handle->io) {
        case IO_METHOD_READ:
                /* Nothing to do. */
                break;

        case IO_METHOD_MMAP:
                for (i = 0; i < handle->n_buffers; ++i) {
                        struct v4l2_buffer buf;

                        CLEAR (buf);

                        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                        buf.memory      = V4L2_MEMORY_MMAP;
                        buf.index       = i;

                        if (-1 == xioctl (handle->fd, VIDIOC_QBUF, &buf))
                                errno_exit ("VIDIOC_QBUF");
                }
                
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

                if (-1 == xioctl (handle->fd, VIDIOC_STREAMON, &type))
                        errno_exit ("VIDIOC_STREAMON");

                break;

        case IO_METHOD_USERPTR:
                for (i = 0; i < handle->n_buffers; ++i) {
                        struct v4l2_buffer buf;

                        CLEAR (buf);

                        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                        buf.memory      = V4L2_MEMORY_USERPTR;
                        buf.index       = i;
                        buf.m.userptr   = (unsigned long) handle->buffers[i].start;
                        buf.length      = handle->buffers[i].length;

                        if (-1 == xioctl (handle->fd, VIDIOC_QBUF, &buf))
                                errno_exit ("VIDIOC_QBUF");
                }

                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

                if (-1 == xioctl (handle->fd, VIDIOC_STREAMON, &type))
                        errno_exit ("VIDIOC_STREAMON");

                break;
        }
}

static void
uninit_device                   (V4L2WHandler_t * handle)
{
        unsigned int i;

        switch (handle->io) {
        case IO_METHOD_READ:
                free (handle->buffers[0].start);
                break;

        case IO_METHOD_MMAP:
                for (i = 0; i < handle->n_buffers; ++i)
                        if (-1 == munmap (handle->buffers[i].start, handle->buffers[i].length))
                                errno_exit ("munmap");
                break;

        case IO_METHOD_USERPTR:
                for (i = 0; i < handle->n_buffers; ++i)
                        free (handle->buffers[i].start);
                break;
        }

        free (handle->buffers);
}

static void
init_read                       (V4L2WHandler_t * handle, unsigned int           buffer_size)
{
        handle->buffers = calloc (1, sizeof (*handle->buffers));
    
        if (!handle->buffers) {
                fprintf (stderr, "Out of memory\n");
                exit (EXIT_FAILURE);
        }

        handle->buffers[0].length = buffer_size;
        handle->buffers[0].start = malloc (buffer_size);

        if (!handle->buffers[0].start) {
                fprintf (stderr, "Out of memory\n");
                exit (EXIT_FAILURE);
        }
}

static void
init_mmap                       (V4L2WHandler_t * handle)
{
        struct v4l2_requestbuffers req;

        CLEAR (req);

        req.count               = 2;
        req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory              = V4L2_MEMORY_MMAP;

        if (-1 == xioctl (handle->fd, VIDIOC_REQBUFS, &req)) {
                if (EINVAL == errno) {
                        fprintf (stderr, "%s does not support "
                                 "memory mapping\n", handle->dev_name);
                        exit (EXIT_FAILURE);
                } else {
                        errno_exit ("VIDIOC_REQBUFS");
                }
        }

        if (req.count < 2) {
                fprintf (stderr, "Insufficient buffer memory on %s\n",
                         handle->dev_name);
                exit (EXIT_FAILURE);
        }

        handle->buffers = calloc (req.count, sizeof (*handle->buffers));

        if (!handle->buffers) {
                fprintf (stderr, "Out of memory\n");
                exit (EXIT_FAILURE);
        }

        for (handle->n_buffers = 0; handle->n_buffers < req.count; ++handle->n_buffers) {
                struct v4l2_buffer buf;

                CLEAR (buf);

                buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory      = V4L2_MEMORY_MMAP;
                buf.index       = handle->n_buffers;

                if (-1 == xioctl (handle->fd, VIDIOC_QUERYBUF, &buf))
                        errno_exit ("VIDIOC_QUERYBUF");

                handle->buffers[handle->n_buffers].length = buf.length;
                handle->buffers[handle->n_buffers].start =
                        mmap (NULL /* start anywhere */,
                              buf.length,
                              PROT_READ | PROT_WRITE /* required */,
                              MAP_SHARED /* recommended */,
                              handle->fd, buf.m.offset);

                if (MAP_FAILED == handle->buffers[handle->n_buffers].start)
                        errno_exit ("mmap");
        }
}

static void
init_userp                      (V4L2WHandler_t * handle, unsigned int           buffer_size)
{
        struct v4l2_requestbuffers req;
        unsigned int page_size;

        page_size = getpagesize ();
        buffer_size = (buffer_size + page_size - 1) & ~(page_size - 1);

        CLEAR (req);

        req.count               = 4;
        req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory              = V4L2_MEMORY_USERPTR;

        if (-1 == xioctl (handle->fd, VIDIOC_REQBUFS, &req)) {
                if (EINVAL == errno) {
                        fprintf (stderr, "%s does not support "
                                 "user pointer i/o\n", handle->dev_name);
                        exit (EXIT_FAILURE);
                } else {
                        errno_exit ("VIDIOC_REQBUFS");
                }
        }

        handle->buffers = calloc (4, sizeof (*handle->buffers));

        if (!handle->buffers) {
                fprintf (stderr, "Out of memory\n");
                exit (EXIT_FAILURE);
        }

        for (handle->n_buffers = 0; handle->n_buffers < 4; ++handle->n_buffers) {
                handle->buffers[handle->n_buffers].length = buffer_size;
                handle->buffers[handle->n_buffers].start = memalign (/* boundary */ page_size,
                                                                     buffer_size);

                if (!handle->buffers[handle->n_buffers].start) {
                        fprintf (stderr, "Out of memory\n");
                        exit (EXIT_FAILURE);
                }
        }
}

static void
init_device                     (V4L2WHandler_t * handle)
{
        struct v4l2_capability cap;
        struct v4l2_cropcap cropcap;
        struct v4l2_crop crop;
        struct v4l2_format fmt;
        struct v4l2_streamparm stream;
        unsigned int min;

        if (-1 == xioctl (handle->fd, VIDIOC_QUERYCAP, &cap)) {
                if (EINVAL == errno) {
                        fprintf (stderr, "%s is no V4L2 device\n",
                                 handle->dev_name);
                        exit (EXIT_FAILURE);
                } else {
                        errno_exit ("VIDIOC_QUERYCAP");
                }
        }

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
                fprintf (stderr, "%s is no video capture device\n",
                         handle->dev_name);
                exit (EXIT_FAILURE);
        }

        switch (handle->io) {
        case IO_METHOD_READ:
                if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
                        fprintf (stderr, "%s does not support read i/o\n",
                                 handle->dev_name);
                        exit (EXIT_FAILURE);
                }

                break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
                if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
                        fprintf (stderr, "%s does not support streaming i/o\n",
                                 handle->dev_name);
                        exit (EXIT_FAILURE);
                }

                break;
        }


        /* Select video input, video standard and tune here. */


        CLEAR (cropcap);

        cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (0 == xioctl (handle->fd, VIDIOC_CROPCAP, &cropcap)) {
                crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                crop.c = cropcap.defrect; /* reset to default */

                if (-1 == xioctl (handle->fd, VIDIOC_S_CROP, &crop)) {
                        switch (errno) {
                        case EINVAL:
                                /* Cropping not supported. */
                                break;
                        default:
                                /* Errors ignored. */
                                break;
                        }
                }
        } else {        
                /* Errors ignored. */
        }


        CLEAR (fmt);

        fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width       = handle->imgparam.width; 
        fmt.fmt.pix.height      = handle->imgparam.height;
        fmt.fmt.pix.pixelformat = handle->imgparam.pix_fmt;
        fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

        if (-1 == xioctl (handle->fd, VIDIOC_S_FMT, &fmt))
                errno_exit ("VIDIOC_S_FMT");

        CLEAR (stream);

        stream.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;        
        stream.parm.capture.timeperframe.numerator = 1;
        stream.parm.capture.timeperframe.denominator = handle->imgparam.fps;
        /* stream.parm.capture.timeperframe.numerator = 1; */
        /* stream.parm.capture.timeperframe.denominator = 10; */
        stream.parm.capture.readbuffers = 4;

        if (-1 == xioctl (handle->fd, VIDIOC_S_PARM, &stream))
                errno_exit ("VIDIOC_S_PARM");
        //printf("%d %d/n", stream.parm.capture.timeperframe.numerator, stream.parm.capture.timeperframe.denominator);
        stream.parm.capture.readbuffers = 4;

        /* Note VIDIOC_S_FMT may change width and height. */

        /* Buggy driver paranoia. */
        min = fmt.fmt.pix.width * 2;
        if (fmt.fmt.pix.bytesperline < min)
                fmt.fmt.pix.bytesperline = min;
        min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
        if (fmt.fmt.pix.sizeimage < min)
                fmt.fmt.pix.sizeimage = min;

        switch (handle->io) {
        case IO_METHOD_READ:
                init_read (handle, fmt.fmt.pix.sizeimage);
                break;

        case IO_METHOD_MMAP:
                init_mmap (handle);
                break;

        case IO_METHOD_USERPTR:
                init_userp (handle, fmt.fmt.pix.sizeimage);
                break;
        }

        int ii;
                for (ii = 0; ii < handle->n_buffers; ii++) {
                    memset(handle->buffers[ii].start, 0, handle->buffers[ii].length);
                }
}

void V4L2W_setctrl(V4L2WHandler_t * handle, int type, int value)
{
    struct v4l2_control control;
    CLEAR (control);
    control.id = type;
    control.value = value;
    if (-1 == xioctl (handle->fd, VIDIOC_S_CTRL, &control))
        errno_exit ("VIDIOC_S_CTRL");
}

int V4L2W_getctrl(V4L2WHandler_t * handle, int type)
{
    struct v4l2_control control;
    CLEAR (control);
    control.id = type;
    if (-1 == xioctl (handle->fd, VIDIOC_G_CTRL, &control))
        errno_exit ("VIDIOC_G_CTRL");
    return control.value;
}

void V4L2W_gettpf(V4L2WHandler_t * handle, int * num, int * denom)
{
    struct v4l2_streamparm stream;
    CLEAR (stream);
    stream.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;        
    if (-1 == xioctl (handle->fd, VIDIOC_G_PARM, &stream))
        errno_exit ("VIDIOC_G_PARM");
    *num =  stream.parm.capture.timeperframe.numerator;
    *denom = stream.parm.capture.timeperframe.denominator;
}

void V4L2W_settpf(V4L2WHandler_t * handle, int num, int denom)
{
    struct v4l2_streamparm stream;
    CLEAR (stream);
    stream.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;        
    if (-1 == xioctl (handle->fd, VIDIOC_G_PARM, &stream))
        errno_exit ("VIDIOC_G_PARM");
    stream.parm.capture.timeperframe.numerator = num;
    stream.parm.capture.timeperframe.denominator = denom;
    if (-1 == xioctl (handle->fd, VIDIOC_S_PARM, &stream))
        errno_exit ("VIDIOC_S_PARM");        
}

void V4L2W_suspend(V4L2WHandler_t * handle)
{
    stop_capturing (handle);
}

void V4L2W_resume(V4L2WHandler_t * handle)
{
    start_capturing (handle);
}

static void
close_device                    (V4L2WHandler_t * handle)
{
        if (-1 == close (handle->fd))
                errno_exit ("close");

        handle->fd = -1;
}

static void
open_device                     (V4L2WHandler_t * handle)
{
        struct stat st; 

        if (-1 == stat (handle->dev_name, &st)) {
                fprintf (stderr, "Cannot identify '%s': %d, %s\n",
                         handle->dev_name, errno, strerror (errno));
                exit (EXIT_FAILURE);
        }

        if (!S_ISCHR (st.st_mode)) {
                fprintf (stderr, "%s is no device\n", handle->dev_name);
                exit (EXIT_FAILURE);
        }

        handle->fd = open (handle->dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

        if (-1 == handle->fd) {
                fprintf (stderr, "Cannot open '%s': %d, %s\n",
                         handle->dev_name, errno, strerror (errno));
                exit (EXIT_FAILURE);
        }
}

#if 0
static void
usage                           (FILE *                 fp,
                                 int                    argc,
                                 char **                argv)
{
        fprintf (fp,
                 "Usage: %s [options]\n\n"
                 "Options:\n"
                 "-d | --device name   Video device name [/dev/video]\n"
                 "-h | --help          Print this message\n"
                 "-m | --mmap          Use memory mapped buffers\n"
                 "-r | --read          Use read() calls\n"
                 "-u | --userp         Use application allocated buffers\n"
                 "",
                 argv[0]);
}
#endif

static const char short_options [] = "d:hmru";

static const struct option
long_options [] = {
        { "device",     required_argument,      NULL,           'd' },
        { "help",       no_argument,            NULL,           'h' },
        { "mmap",       no_argument,            NULL,           'm' },
        { "read",       no_argument,            NULL,           'r' },
        { "userp",      no_argument,            NULL,           'u' },
        { 0, 0, 0, 0 }
};

#if 0
int
main                            (int                    argc,
                                 char **                argv)
{
        dev_name = "/dev/video";

        for (;;) {
                int index;
                int c;
                
                c = getopt_long (argc, argv,
                                 short_options, long_options,
                                 &index);

                if (-1 == c)
                        break;

                switch (c) {
                case 0: /* getopt_long() flag */
                        break;

                case 'd':
                        dev_name = optarg;
                        break;

                case 'h':
                        usage (stdout, argc, argv);
                        exit (EXIT_SUCCESS);

                case 'm':
                        io = IO_METHOD_MMAP;
                        break;

                case 'r':
                        io = IO_METHOD_READ;
                        break;

                case 'u':
                        io = IO_METHOD_USERPTR;
                        break;

                default:
                        usage (stderr, argc, argv);
                        exit (EXIT_FAILURE);
                }
        }

        open_device ();

        init_device ();

        start_capturing ();

        mainloop ();

        stop_capturing ();

        uninit_device ();

        close_device ();

        exit (EXIT_SUCCESS);

        return 0;
}
#endif

/**********************************
 * V4L2 Wrapper API
 **********************************/
V4L2WHandler_t * V4L2W_init_w_param(const char * device_name0, struct V4L2W_IMGPARAM imgparam)
{
    V4L2WHandler_t * handle;
    int outframesize;

    handle = (V4L2WHandler_t *)malloc(sizeof(struct V4L2W_HANDLER));
    handle->dev_name        = NULL;
    handle->io              = IO_METHOD_MMAP;
    handle->fd              = -1;
    handle->buffers         = NULL;
    handle->n_buffers       = 0;

    /* Device name */
    strcpy(handle->dev_name_tmp, device_name0);
    handle->dev_name = handle->dev_name_tmp;
    /* Device initialization */
    handle->imgparam = imgparam;
    open_device (handle);
    init_device (handle);
    start_capturing (handle);
    /* Allocate Framebuffer */
    switch (handle->imgparam.img_fmt) {
    case V4L2W_IMGFORMAT_GREY:
        outframesize = handle->imgparam.width * handle->imgparam.height;
        break;
    case V4L2W_IMGFORMAT_BGR24:
        outframesize = handle->imgparam.width * handle->imgparam.height * 3;
        break;
    }
    handle->outimg = malloc(outframesize);
    return (V4L2WHandler_t *)handle;
}

V4L2WHandler_t * V4L2W_initialize(const char * device_name0, int width, int height, int fps, int output_format, int capture_format)
{
    struct V4L2W_IMGPARAM imgparam;
    imgparam.width = width;
    imgparam.height = height;
    imgparam.fps = fps;
    imgparam.img_fmt = output_format;
    imgparam.pix_fmt = capture_format;

    return V4L2W_init_w_param(device_name0, imgparam);
}

void V4L2W_select(V4L2WHandler_t * handle)
{
    fd_set fds;
    struct timeval tv;
    int r;

RETRY:    
    FD_ZERO (&fds);
    FD_SET (handle->fd, &fds);

    /* Timeout. */
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    r = select (handle->fd + 1, &fds, NULL, NULL, &tv);
    
    if (-1 == r) {
        if (EINTR == errno)
            goto RETRY;
        
        errno_exit ("select");
    }
    
    if (0 == r) {
        fprintf (stderr, "select timeout\n");
        exit (EXIT_FAILURE);
    }
}

void * V4L2W_capture(V4L2WHandler_t * handle)
{
    V4L2W_select(handle);

    read_frame(handle, process_image);
    // unsigned char * data = malloc(handle->imgparam.width * handle->imgparam.height * 3);
    // int size = read_stream(handle, data, handle->imgparam.width * handle->imgparam.height * 3);
    // printf("%d\n", size);
    // process_image(handle, data, size);
    
    return handle->outimg;
}

int V4L2W_output_stream(V4L2WHandler_t * handle, int fd)
{
    V4L2W_select(handle);

    handle->outputfd = fd;
    read_frame(handle, output2fd);
    // size = read_stream(handle, data, size);
    
    return handle->bytes_in_frame;
}

void V4L2W_finalize(V4L2WHandler_t * handle)
{
    stop_capturing (handle);
    uninit_device (handle);
    close_device (handle);
    free(handle->outimg);
    free(handle);
}

