DEST=localhost

# tuning parameters to make the sender send the streams out of sync. Can be used
# ot test the client RTCP synchronisation.
#VOFFSET=900000000
VOFFSET=0
AOFFSET=0

# H264 encode from the source
# VELEM="v4l2src device=/dev/video0"
VELEM="filesrc location=camera.h264 ! h264parse"

VCAPS="video/x-raw,width=1920,height=1080,framerate=30/1"
VSOURCE="$VELEM"
VENC="rtph264pay"

VRTPSINK="udpsink port=5000 host=$DEST ts-offset=$VOFFSET name=vrtpsink"
VRTCPSINK="udpsink port=5001 host=$DEST sync=false async=false name=vrtcpsink"
VRTCPSRC="udpsrc port=5005 name=vrtpsrc"

gst-launch-1.0 -v rtpbin name=rtpbin \
    $VSOURCE ! $VENC ! rtpbin.send_rtp_sink_0 \
        rtpbin.send_rtp_src_0 ! $VRTPSINK     \
        rtpbin.send_rtcp_src_0 ! $VRTCPSINK   \
      $VRTCPSRC ! rtpbin.recv_rtcp_sink_0  