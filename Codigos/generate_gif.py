# generate_gif.py
import os
from PIL import Image

# Configuración
input_dir = "frames"
output_dir = "png_frames"
gif_name = "smooth_rotation.gif"
duration = 50  # ms por frame (menos = más rápido)

os.makedirs(output_dir, exist_ok=True)

# Convertir PGM a PNG
for filename in sorted(os.listdir(input_dir)):
    if filename.endswith(".pgm"):
        pgm_path = os.path.join(input_dir, filename)
        png_path = os.path.join(output_dir, filename.replace(".pgm", ".png"))
        with Image.open(pgm_path) as img:
            img.save(png_path)

# Crear GIF con optimización
frames = [Image.open(os.path.join(output_dir, f)) for f in sorted(os.listdir(output_dir))]
frames[0].save(
    gif_name,
    save_all=True,
    append_images=frames[1:],
    duration=duration,
    loop=0,
    optimize=True,
    quality=95
)

print(f"GIF generado como {gif_name}")












