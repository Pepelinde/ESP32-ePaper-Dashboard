#pragma once
#if __has_include("dashboard_photo.local.h")
#include "dashboard_photo.local.h"
#else
// Imagen neutra. Sustituir localmente por una imagen propia de 220 x 220.
static const int DASHBOARD_PHOTO_WIDTH = 220;
static const int DASHBOARD_PHOTO_HEIGHT = 220;
static const unsigned char DASHBOARD_PHOTO_BITMAP[6160] PROGMEM = {0};
#endif
