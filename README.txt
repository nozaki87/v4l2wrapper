v4l2wrapper
=================

* Libraries you need
libjpeg
OpenCV-dev
Gstreamer 1.0

* How to build
mkdir build
cd build
cmake ../
make
Now you can find executable binaries under build/test/.

* Usage of vga_raw / fHD_jpeg
Just execute

* Usage of fHD_h264_stdout
fHD_h264_stdout outputs H264 ES (Elementary stream) as standard output.
./test/fHD_h264_stdout | gst-launch -v -e filesrc location=/dev/fd/0 ! h264parse ! avdec_h264 ! videoconvert ! videoscale ! ximagesink sync=false

** TCP sender
*** General webcam.
gst-launch-1.0 v4l2src ! video/x-raw-yuv,width=1920,height=1080,framerate=30/1 ! x264enc  ! h264parse ! mpegtsmux ! tcpserversink  host=127.0.0.1 port=5000
*** Logitech C920 (For HW Encoding of H.264)
./test/fHD_h264_stdout | gst-launch -v -e filesrc location=/dev/fd/0 ! h264parse ! mpegtsmux ! tcpserversink  host=127.0.0.1 port=5000

** UDP sender
*** General webcam.
gst-launch-1.0 -v -e filesrc location=camera.h264 ! h264parse ! mpegtsmux ! rtpmp2tpay ! udpsink  host=127.0.0.1 port=4000
gst-launch -v -e filesrc location=camera.h264 ! h264parse ! mpegtsmux ! rtpmp2tpay ! udpsink  host=127.0.0.1 port=4000
*** Logitech C920 (For HW Encoding of H.264)
./test/fHD_h264_stdout | gst-launch-1.0 -v -e filesrc location=/dev/fd/0 ! h264parse ! mpegtsmux ! rtpmp2tpay ! udpsink  host=127.0.0.1 port=4000
./test/fHD_h264_stdout | gst-launch -v -e filesrc location=/dev/fd/0 ! h264parse ! mpegtsmux ! rtpmp2tpay ! udpsink  host=127.0.0.1 port=4000

** TCP receiver
gst-launch-1.0 -v tcpclientsrc port=4000 caps='application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264' ! tsdemux ! h264parse ! avdec_h264 ! videoconvert ! videoscale ! ximagesink sync=false

** UDP receiver
gst-launch-1.0 -v udpsrc port=4000 caps='application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264' ! rtpmp2tdepay ! tsdemux ! h264parse ! avdec_h264 ! videoconvert ! videoscale ! ximagesink sync=false
gst-launch -v udpsrc port=4000 caps='application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264' ! rtpmp2tdepay ! mpegtsdemux ! queue ! ffdec_h264 !  ffmpegcolorspace ! deinterlace ! xvimagesink sync=false

