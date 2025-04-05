
# Rendering-C
## por: JUAN DAVID DELGADO BURBANO.
Un proyecto de Rendering (Intersección rayo-figura), que genera frames en pgm, convirtiendolos en un gif.
-
![image](https://github.com/user-attachments/assets/b8b4b4e3-b3b2-4c6c-9ff2-f7fe3e30e4e7)
![image](https://github.com/user-attachments/assets/32f99a6d-df8c-46ec-b31d-c29e13a75b80)

-

https://replit.com/@juandaplaystati/raytracing#animacion2.gif
-
## video explicativo para compilar he visualisar gif desde replit.
-
https://livejaverianaedu.sharepoint.com/:v:/s/YOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO/ET7mQbbTGDBBjNbwMFyhYtMByT81fCOE_RAKSrKLz9XKEw?nav=eyJyZWZlcnJhbEluZm8iOnsicmVmZXJyYWxBcHAiOiJTdHJlYW1XZWJBcHAiLCJyZWZlcnJhbFZpZXciOiJTaGFyZURpYWxvZy1MaW5rIiwicmVmZXJyYWxBcHBQbGF0Zm9ybSI6IldlYiIsInJlZmVycmFsTW9kZSI6InZpZXcifX0%3D&e=7eyT5M
## Método de compilación y ejecución
-

## Para compilar y ejecutar el proyecto, sigue estos pasos:
### paso 0:
-ejecutar el programa "es muy importante primero ejecutar el programa para que se cree la carpeta de frames ).

### 1. abrir la shell:
- una vez abierta la shell, escribir el siguiente comando:
   "chmod +x run.sh && ./run.sh"
  - con ese comando hara que el archivo "run.sh" se convierta en ejecutable y creara gif, que se lo encontrara junto con los codigos 
![image](https://github.com/user-attachments/assets/eaae97d9-b61a-4f2b-bd37-478b60d1f553)
![Uploading image.png…]()


### 2. opcion 2:

- se puede escribir este comando desde la shell
- convert -delay 8 -loop 0 frames/frame_*.pgm animacion.gif
-
- ese comando generara un gif, nota: (sugiero ver el video explicativo con esta opcion, puesto que al ponerlo por primera vez, aparece una serie de opciones para escoger, y es la tercera opcion).



## proyecto estructura:
- run.sh                # Script de ejecución principal
-  main.c                # Código fuente del ray tracer
-   EDM.h                 # Librería de matemáticas 3D
-   EDM.c                 # Implementación de operaciones 3D
-   generate_gif.py       # Script alternativo para GIF (Python) "convierte los frames de pgm, a png, y genera un gif con ellos."
-   frames               # Directorio con los frames generados
-   frame_*.pgm       # Frames de la animación pgm,.
-   animacion.gif         # GIF resultante (se genera al ejecutar)
