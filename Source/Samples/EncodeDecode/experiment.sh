#!/bin/bash
# run through git bash

# BITRATES=(4000 8000 16000) 
BITRATES=(1000) 
# BITRATES=(2000) 
# BITRATES=(500 1000 2000 4000 8000 16000 32000) 

# RESOLUTION=480

# FRAMERATE=(30 40 50 100 110) 
# FRAMERATE=(30) 
FRAMERATE=(30 40 50 60 70 80 90 100 110 120) 
# FRAMERATE=(30 40 50 60)
# FRAMERATE=(30 40 50 60 70 80 90 100 110 120 130 140 150 160 165) 

# resolutions=("1920x1080" "1536x864" "1280x720" "960x540" "854x480" "640x360") # "1200x676"
resolutions=("1920x1080" "1536x864" "1280x720" "854x480" "640x360")
# resolutions=("1920x1080")


for resolution in "${resolutions[@]}"
do
    IFS='x' read -r width height <<< "$resolution"

    echo "Processing resolution: $resolution"
    echo "Width: $width, Height: $height"
    for framerate in "${FRAMERATE[@]}"; do
        for bitrate in "${BITRATES[@]}"; do
            # Print current bitrate
            echo "============================ Setting bitrate to: $bitrate ============================"
            echo "Bitrate: $bitrate, Resolution: $height, FRAMERATE: $framerate"

            # to generate frames for training, only use encodedeco.cpp, generate_truevals.py and data_prepare.py in VRR_cvvdp
            /c/Users/15142/new/Falcor/build/windows-vs2022/bin/Debug/EncodeDecode.exe "$bitrate" "$framerate" "$width" "$height"

            # # generate decode and reference videos; change True in playBMP.py to generate reference videos
            python /c/Users/15142/Desktop/VRR/VRR_cvvdp/playBMP.py "$bitrate" "$height" "$framerate"

            python /c/Users/15142/Desktop/VRR/VRRML_Data/frames.py "$bitrate" "$height" "$framerate"
            wait
        done
    done
done


# gcc EncodeDecode.cpp -o outputfile.exe -I "../../../../Source/Falcor/"
# C:/Users/15142/source/repos/Falcor/Falcor/build/Source/PerceptualRendering/EncodeDecode

# C:/Users/15142/new/Falcor/Source/Samples/EncodeDecode

# C:/Users/15142/new/Falcor/build/windows-vs2022/bin/Debug