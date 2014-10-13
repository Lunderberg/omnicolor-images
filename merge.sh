#!/bin/bash

input_video=video3.avi
input_audio=Virtualman_-_Sleepwalk.mp3
output=output.avi
length="3:00"

intermediate=temp.avi

text="Music\\: Sleepwalk, by Virtualman, licensed under Creative Commons"

rm -f $output $intermediate

#Add text overlay
ffmpeg -i $input_video -vf drawtext="fontfile=/usr/share/fonts/truetype/ubuntu-font-family/Ubuntu-L.ttf: \
                                     text='$text': \
                                     fontcolor=white: fontsize=24: \
                                     box=1: boxcolor=black: \
                                     x=0: y=0: \
                                     enable='between(t,0,5)
         " -vcodec h264 -crf 18 $intermediate

#Merge audio/vidoe stream
ffmpeg -i $intermediate -i $input_audio -vcodec copy -acodec copy -to $length $output