# Imagen personal opcional

No se incluye la fotografia original ni sus derivados. El firmware utiliza un bitmap blanco neutro.
Para usar una imagen propia, guardala como `assets/dashboard_photo_source.png`, instala Pillow y ejecuta `python tools/pack_dashboard_photo.py`. La imagen debe ser cuadrada.
El script genera `src/dashboard_photo.local.h` y vistas locales ignoradas por Git. El firmware selecciona automaticamente esa imagen local; sin ella usa el bitmap blanco neutro.
