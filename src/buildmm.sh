#!/bin/bash
#shared lib build
#gcc -o -DRPI ../mapmaker ./mapmaker.c -pthread -lrt `pkg-config --libs allegro-5 allegro_image-5 allegro_primitives-5 allegro_font-5 allegro_ttf-5`

#static linked build
export LDFLAGS="-rpath /opt/vc/lib/ "$LDFLAGS
gcc -O2 -DRPI -march=armv6j -mfpu=vfp -mfloat-abi=hard -o ../mapmaker ./mapmaker.c -pthread -lrt -lm `pkg-config --libs --static allegro-static-5 allegro_image-static-5 allegro_primitives-static-5 allegro_font-static-5 allegro_ttf-static-5 allegro_audio-static-5 allegro_acodec-static-5`
