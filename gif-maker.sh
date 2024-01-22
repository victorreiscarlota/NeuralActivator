#!/bin/sh

set -xe

# Assuming gcc is the C compiler
gcc -Wall -Wextra -ggdb -o main main.c -lm

./main
ffmpeg -y -i data/weights-%03d.ppm demo.mp4
ffmpeg -y -i data/demo.mp4 -vf palettegen data/palette.png
ffmpeg -y -i data/demo.mp4 -i data/palette.png -filter_complex paletteuse demo.gif
