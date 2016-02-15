#!/bin/bash
#shared lib build
#gcc -march=armv6j -mfpu=vfp -mfloat-abi=hard -o ../game ./game.c ./drawing.c ./init.c ./collisions.c ./objects.c ./inputs.c-pthread -lrt -lm -lwiringPi `pkg-config --libs allegro-5 allegro_image-5 allegro_primitives-5 allegro_font-5 allegro_ttf-5 allegro_audio-5 allegro_acodec-5` -Wl,--verbose

#static linked build
#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/vc/lib/
export LDFLAGS="-rpath /opt/vc/lib/ "$LDFLAGS
#gcc -O2 -march=armv6j -mfpu=vfp -mfloat-abi=hard -o ../game ./game.c ./drawing.c ./init.c ./collisions.c ./objects.c ./inputs.c -pthread -lrt -lm -lwiringPi `pkg-config --libs --static allegro-static-5 allegro_image-static-5 allegro_primitives-static-5 allegro_font-static-5 allegro_ttf-static-5 allegro_audio-static-5 allegro_acodec-static-5`
gcc -march=armv6j -mfpu=vfp -mfloat-abi=hard -o ../gravstorm ./game.c ./drawing.c ./init.c ./collisions.c ./objects.c ./inputs.c -pthread -lrt -lm -lwiringPi `pkg-config --libs --static allegro-static-5 allegro_image-static-5 allegro_primitives-static-5 allegro_font-static-5 allegro_ttf-static-5 allegro_audio-static-5 allegro_acodec-static-5`

#/usr/bin/ld: cannot find -lGLESv2
#found libGLESv2.so at /opt/vc/lib/libGLESv2.so
#/usr/bin/ld: cannot find -lEGL
#/usr/bin/ld: cannot find -lbcm_host
