#!/bin/bash
gcc -o ../mapmaker ./mapmaker.c -pthread -lrt `pkg-config --libs allegro-5 allegro_image-5 allegro_primitives-5 allegro_font-5 allegro_ttf-5`
