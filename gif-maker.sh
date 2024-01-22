#!/bin/sh

set -xe

# Create an "output" folder if it doesn't exist
mkdir -p output

# Assuming gcc is the C compiler
gcc -Wall -Wextra -ggdb -o output/main main.c -lm

# Run the compiled program
./output/main

# Process video and create gif inside the "output" folder
ffmpeg -y -i data/weights-%03d.ppm output/test.mp4
ffmpeg -y -i output/test.mp4 -vf palettegen output/palette.png
ffmpeg -y -i output/test.mp4 -i output/palette.png -filter_complex paletteuse output/test.gif
