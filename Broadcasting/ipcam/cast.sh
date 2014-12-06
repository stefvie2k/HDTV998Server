#!/bin/bash

# TS File Encoding Restrictions:
#
# Lead in/out black frames: No restriction
# Fade-in/out: No restriction
# TS Encoding: 1080i
# Maximum Programs: 1 Program containing 1 Video and 1 Audio service
# Program PID definitions: 
# Video PID: 17 (0x011)
# Audio PID: 20 (0x014)
# TS clip start: Closed GOP
# TS clip end: No restriction
#
# PSI / PSIP Tables: Placeholders for minimal set of ATSC PSI/PSIP tables must be present in TS
# file. PSI/PSIP should be multiplexed at the PID locations and repetition rates given below:

# PAT PID: 0 (0x00), 100ms
# PMT PID: 16 (0x10), 400ms
# MGT PID: 8187 (0x1FFB), 150ms
# TVCT PID: 8187 (0x1FFB), 400ms
# STT PID: 8187 (0x1FFB), 1000ms
# RTT PID: 8187 (0x1FFB), 60s
# EIT-0 PID: 8144 (0x1FD0), 500ms
# EIT-1 PID: 8145 (0x1FD1), 500ms
# EIT-2 PID: 8146 (0x1FD2), 500ms
# EIT-3 PID: 8147 (0x1FD3), 500ms

video_pid=49

let atsc_bitrate="19392658"
echo "ATSC bitrate: $atsc_bitrate"

let ts_pat_bitrate="`stat -c %s psid/pat.ts` * 8 * 1000 / 100"		# 100ms
echo "PAT birrate: $ts_pat_bitrate"

let ts_pmt_bitrate="`stat -c %s psid/pmt.ts` * 8 * 1000 / 400"		# 400ms
echo "PMT bitrate: $ts_pmt_bitrate"

let ts_mgt_bitrate="`stat -c %s psid/mgt.ts` * 8 * 1000 / 150"		# 150ms
echo "MGT bitrate: $ts_mgt_bitrate"

let ts_tvct_bitrate="`stat -c %s psid/tvct.ts` * 8 * 1000 / 400"	# 400ms
echo "TVCT bitrate: $ts_tvct_bitrate"

let video_frame_rate="30"
echo "Video frame rate: $video_frame_rate"

let video_bitrate="2000000"
echo "Video bitrate: $video_bitrate"

let ts_video_bitrate=$(echo "$video_bitrate * 1.15" | bc | cut -f1 -d.)
echo "PID $video_pid (Video) bitrate: $ts_video_bitrate"

let vbv_bufsize="1343488" #"$half_vbv * 16 * 1024"
echo "VBV buffer size: $vbv_bufsize"


# Check if our null.ts file exist, if not, create it...

if [ ! -f null.ts ]; then
	echo -n $'\x47' > null.ts
	echo -n $'\x1f' >> null.ts
	echo -n $'\xff' >> null.ts
	echo -n $'\x10' >> null.ts
	dd if=/dev/zero oflag=append conv=notrunc of=null.ts bs=1 count=184
fi


# Recreate fifo files
[ -e video.es ] && rm video.es
mkfifo video.es

[ -e video.pes ] && rm video.pes
mkfifo video.pes

[ -e video.ts ] && rm video.ts
mkfifo video.ts

[ -e muxed.ts ] && rm muxed.ts
mkfifo muxed.ts


# Create video elementary stream, from ffmpeg smpte bars, and output video.es

ffmpeg -re -r $video_frame_rate -rtsp_transport tcp \
 	-i "rtsp://admin:12345@cam_address:554/Streaming/Channels/2" -map 0:0 -an \
	-c:v mpeg2video -pix_fmt yuv420p -aspect 16/9 -f mpeg2video \
	-b:v $video_bitrate -maxrate $video_bitrate -minrate $video_bitrate -bf 2 -bufsize $vbv_bufsize -y video.es &


# Convert video elementary stream to packetized elementary stream using named pipe

esvideompeg2pes video.es > video.pes &


# Convert video packetsized elementary stream to transport stream using named pipe

pesvideo2ts $video_pid $video_frame_rate 112 $ts_video_bitrate 0 video.pes > video.ts &  


# Mux to whole thing together adding PMT crap and null packet to make sure the stream is ATSC  19392658 bps (19.39Mbps)

tscbrmuxer2 \
  b:$ts_pat_bitrate psid/pat.ts \
  b:$ts_pmt_bitrate psid/pmt.ts \
  b:$ts_mgt_bitrate psid/mgt.ts \
  b:$ts_tvct_bitrate psid/tvct.ts \
  b:$ts_video_bitrate video.ts \
  o:$atsc_bitrate null.ts \
  > muxed.ts &


# Add the PCR stamping

tstcpsend muxed.ts hdtv998_address 9980 $atsc_bitrate
