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

half_vbv=112
audio_frame_size=1536 # 1152 for mp2 and 1536 for ac3
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

let audio_sample_rate="48000"	# 48KHz
echo "Audio sample rate: $audio_sample_rate"

# (3 ts_packets * 188 size_ts_packet * 90KHz pts_ticks) / (1152 sample_per_frame * 90KHz pts_tick / audio_bitrate)
let ts_audio_bitrate="(3 * 188 * 8 * 90000) / (512 * 90000 / $audio_sample_rate)"
echo "PID $audio_pid (Audio) bitrate: $ts_audio_bitrate" # 211,500bps

let video_frame_rate="30"
echo "Video frame rate: $video_frame_rate"

let video_bitrate="2000000"
echo "Video bitrate: $video_bitrate"

let ts_video_bitrate=$(echo "$video_bitrate * 1.15" | bc | cut -f1 -d.)
echo "PID $video_pid (Video) bitrate: $ts_video_bitrate"

let first_pts="90000 / $video_frame_rate"
echo "First PTS: $first_pts"

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
[ -e audio.es ] && rm audio.es
#mkfifo audio.es

[ -e video.es ] && rm video.es
mkfifo video.es

[ -e audio.pes ] && rm audio.pes
#mkfifo audio.pes

[ -e video.pes ] && rm video.pes
mkfifo video.pes

[ -e audio.ts ] && rm audio.ts
#mkfifo audio.ts

[ -e audio-loop.ts ] && rm audio-loop.ts
mkfifo audio-loop.ts

[ -e video.ts ] && rm video.ts
mkfifo video.ts

[ -e muxed.ts ] && rm muxed.ts
mkfifo muxed.ts

[ -e output.ts ] && rm output.ts
mkfifo output.ts


# Create 1kHz tone

sox -n audio.wav synth 10 sin 400 

# arguments:
# -n	 		use the `null' file handler; e.g. with synth effect
# audio.wav		output file
# synth			use synth effect
# 10			duration
# sin			sin wave
# 400			Hz frequency


# Create audio elementary stream, from 400Hz ton, and output audio.es

ffmpeg -i audio.wav -vn -acodec ac3 -ac 1 -ab 128k -ar $audio_sample_rate -f ac3 -y audio.es

# arguments:
# -i audio.wav		input file
# -vn			no video
# -acodec ac3		audio codec mpeg audio layer 2
# -ac 2			2 channels (stereo)
# -f ac3		format ac3 audio for elementary stream
# -ab 128k		audio bit rate in kbps
# -ar 48000             audio sample rate
# -y                    overwrite existing file
# audio.es		output file


# Create video elementary stream, from ffmpeg smpte bars, and output video.es

ffmpeg -re -f lavfi -i "smptehdbars=size=1920x1080:rate=$video_frame_rate" -an -vcodec mpeg2video \
  -vf drawtext="fontsize=15:fontfile=/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf:timecode='00\:00\:00\:00':rate=$video_frame_rate:text='TCR\:':fontsize=72:fontcolor='white':boxcolor=0x000000AA:box=1:x=940-text_w/2:y=650" \
  -f mpeg2video -b:v $video_bitrate -maxrate $video_bitrate -minrate $video_bitrate -bf 2 -bufsize $vbv_bufsize \
  -flags +ilme+ildct -alternate_scan 1 -top 0 -y video.es &


# arguments:
# -f lavfi		format lavfi
# -i "..."		smptehdbars parameters (60 second clip at 1920x1080@30)
# -an			no audio
# -vcodec mpeg2video	video codec mpeg2 video
# -vf drawtext="..."	draw time code count
# -f mpeg2video		format mpeg2 video for elementary stream
# -b:v, -maxrate, -minrate	bitrate in kbps
# -bf 2			number of b-frame for gop
# -bufsize 		vbv buffersize
# -y                    overwrite existing file


# Convert audio elementary stream to packetized elementary stream using named pipe

esaudio2pes audio.es $audio_frame_size $audio_sample_rate 512 -1 $first_pts > audio.pes &

# arguments:
# audio.es		input file
# 1536			audio frame size (1536 for ac3)
# 48000			sample rate
# 1024			es frame size
# -1			disable audio description header
# 3600			first pts (90KHz / frame_rate)
# > audio.pes		output to named pipe			
# &			send to background


# Convert audio packetized elementary stream to transport stream using named pipe

pesaudio2ts $audio_pid $audio_frame_size $audio_sample_rate 512 -1 audio.pes > audio.ts &

# arguments:
# 2068			audio pid number
# 1536			number of sampler per frame (1152 for mp2 audio, 1536 for ac3 audio)
# 48000			sample rate
# 768			es frame size
# -1			no looping
# audio.es		input file
# > audio.ts		output file
# &			send to background


# Loop audio tone continuously

tsloop audio.ts > audio-loop.ts &


# Convert video elementary stream to packetized elementary stream using named pipe

esvideompeg2pes video.es > video.pes &

# arguments:
# video.es		input file
# > video.pes	output file
# &			send to background


# Convert video packetsized elementary stream to transport stream using named pipe

pesvideo2ts $video_pid $video_frame_rate $half_vbv $ts_video_bitrate 0 video.pes > video.ts &  

# arguments:
# 2064			video pid number
# 60			frame rate
# 112			half vbv
# 6900000		ts bit rate (15% bigger)
# video.pes		input file
# > video.ts		output file
# &			send to background


# Mux to whole thing together adding PMT crap and null packet to make sure the stream is ATSC  19392658 bps (19.39Mbps)

tscbrmuxer2 \
  b:$ts_pat_bitrate psid/pat.ts \
  b:$ts_pmt_bitrate psid/pmt.ts \
  b:$ts_mgt_bitrate psid/mgt.ts \
  b:$ts_tvct_bitrate psid/tvct.ts \
  b:$ts_video_bitrate video.ts \
  b:$ts_audio_bitrate audio-loop.ts \
  o:$atsc_bitrate null.ts \
  > muxed.ts &

# arguments:
# b:6900000		video bit rate @ 6900000 bps (6.9Mbps)
# video.ts		video transport stream
# b:20000		audio bit rate @ 20000 bps (20kbps)
# audio.ts		audio transport stream
# b:3008		pat bit rate 
# pat.ts		pat transport stream
# b:3008		pmt bit rate
# pmt.ts		pmt transport stream
# b:3008		8187 bit rate
# 8187.ts		8187 transport stream
# o:19.39Mbps		ATSC bit rate
# null.ts		null transport stream ... which is used to ensure TS is at ATSC bitrate

# Add the PCR stamping

tspcrstamp muxed.ts $atsc_bitrate > output.ts &

tstcpsend output.ts hdtv998_address 9980 $atsc_bitrate
