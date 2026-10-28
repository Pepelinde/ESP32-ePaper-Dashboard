#pragma once
// Paste the Apps Script web-app URL ending in /exec, without query parameters.
static const char GOOGLE_DASHBOARD_URL[] = "Pon aqui tu URL de Google Apps Script";
// Same long random token as DASHBOARD_TOKEN in Apps Script properties.
// Keep both values private. Leave blank until Google has been configured.
static const char GOOGLE_DASHBOARD_TOKEN[] = "Pon aqui tu credencial de Google";
static const unsigned long DASHBOARD_CACHE_MAX_AGE = 24UL * 60UL * 60UL;
