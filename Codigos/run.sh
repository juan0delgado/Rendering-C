#!/bin/bash

# Crear directorio para los frames si no existe
mkdir -p frames

# Compilar el programa
gcc -o raytrace main.c EDM.c -lm

# Ejecutar el programa para generar los frames
./raytrace

# Crear el GIF usando ImageMagick
convert -delay 10 -loop 0 frames/frame_*.pgm animacion.gif

echo "¡Proceso completado! El GIF se ha generado como 'animacion.gif'"

