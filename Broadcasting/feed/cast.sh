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

video_pid=65
audio_pid=68

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

let audio_sample_rate="48000"   # 48KHz
echo "Audio sample rate: $audio_sample_rate"

# (3 ts_packets * 188 size_ts_packet * 90KHz pts_ticks) / (1152 sample_per_frame * 90KHz pts_tick / audio_bitrate)
let ts_audio_bitrate="(3 * 188 * 8 * 90000) / (768 * 90000 / $audio_sample_rate)"
echo "PID $audio_pid (Audio) bitrate: $ts_audio_bitrate" # 211,500bps

let video_frame_rate="30"
echo "Video frame rate: $video_frame_rate"

let video_bitrate="8000000"	# 8Mbps
echo "Video bitrate: $video_bitrate"

let vbv_bufsize="1835008"

let ts_video_bitrate=$(echo "$video_bitrate * 1.15" | bc | cut -f1 -d.)
echo "PID $video_pid (Video) bitrate: $ts_video_bitrate"


# Check if our null.ts file exist, if not, create it...

if [ ! -f null.ts ]; then
        echo -n $'\x47' > null.ts
        echo -n $'\x1f' >> null.ts
        echo -n $'\xff' >> null.ts
        echo -n $'\x10' >> null.ts
        dd if=/dev/zero oflag=append conv=notrunc of=null.ts bs=1 count=184
fi

# Recreate fifo files

[ -e video.ts ] && rm video.ts
mkfifo video.ts

[ -e filtered.ts ] && rm filtered.ts
mkfifo filtered.ts 

[ -e muxed.ts ] && rm muxed.ts
mkfifo muxed.ts

[ -e output.ts ] && rm output.ts
mkfifo output.ts


# Create video elementary stream, from ffmpeg smpte bars, and output video.es

ffmpeg  -re -i tcp://source_address:7000 -map 0:0 -map 0:1 -r $video_frame_rate \
	-c:v mpeg2video -b:v $video_bitrate -minrate:v $video_bitrate -maxrate:v $video_bitrate \
	-bf:v 2 -bufsize:v $vbv_bufsize -streamid:v 0:65 \
	-c:a ac3 -b:a 192k -ar $audio_sample_rate -ac 2 -streamid:a 1:68 \
	-f mpegts -y video.ts &


# Clean up the TS that ffmpeg created since we'll be using our own PMT/PAT/etc

tsfilter video.ts +65 +68 > filtered.ts &


# Mux to whole thing together adding PMT crap and null packet to make sure the stream is ATSC  19392658 bps (19.39Mbps)

tscbrmuxer2 \
  b:$ts_pat_bitrate psid/pat.ts \
  b:$ts_pmt_bitrate psid/pmt.ts \
  b:$ts_mgt_bitrate psid/mgt.ts \
  b:$ts_tvct_bitrate psid/tvct.ts \
  b:$ts_video_bitrate filtered.ts  \
  o:$atsc_bitrate null.ts \
  > muxed.ts &


# Add the PCR stamping

tspcrrestamp muxed.ts $atsc_bitrate > output.ts &
tstcpsend output.ts hdtv998_address 9980 $atsc_bitrate
