DEST=localhost

# this adjusts the latency in the receiver
LATENCY=0

VIDEO_CAPS="application/x-rtp,media=(string)video,clock-rate=(int)90000,encoding-name=(string)H264"

VIDEO_DEC="rtph264depay ! avdec_h264"

VIDEO_SINK="videoconvert ! autovideosink"

gst-launch-1.0 -v rtpbin name=rtpbin latency=$LATENCY \
     udpsrc caps=$VIDEO_CAPS port=5000 ! rtpbin.recv_rtp_sink_0  \
       rtpbin. ! $VIDEO_DEC ! $VIDEO_SINK  \
     udpsrc port=5001 ! rtpbin.recv_rtcp_sink_0  \
         rtpbin.send_rtcp_src_0 ! udpsink port=5005 host=$DEST sync=false async=false 
