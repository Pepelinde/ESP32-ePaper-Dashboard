#pragma once
#include <WiFiClientSecure.h>
#include <Preferences.h>
#include "dashboard_config.h"
#include "dashboard_ca.h"

struct DashboardEvent {
  String date, hour, title;
  long long endEpoch = 0;
};
struct DashboardData {
  bool airAvailable = false, airFresh = false;
  int aqi = 0;
  float pm25 = 0, pm10 = 0;
  bool calendarAvailable = false, calendarFresh = false;
  int month = 0, year = 0, eventCount = 0;
  bool eventDays[32] = {};
  DashboardEvent events[3];
  bool tasksAvailable = false, tasksFresh = false;
  int taskCount = 0;
  String tasks[4];
  bool noteAvailable = false, noteFresh = false;
  String note;
  bool mailAvailable = false, mailFresh = false;
  int unreadMail = 0;
} dashboardData;

bool GoogleDashboardConfigured() {
  return strlen(GOOGLE_DASHBOARD_URL) && strlen(GOOGLE_DASHBOARD_TOKEN)
    && strncmp(GOOGLE_DASHBOARD_URL, "https://", 8) == 0
    && strncmp(GOOGLE_DASHBOARD_TOKEN, "Pon aqui", 8) != 0;
}

String DashboardEncode(const char* value) {
  String result;
  const char* hex = "0123456789ABCDEF";
  for (const unsigned char* p = reinterpret_cast<const unsigned char*>(value); *p; ++p) {
    if (isalnum(*p) || *p == '-' || *p == '_' || *p == '.' || *p == '~') result += char(*p);
    else { result += '%'; result += hex[*p >> 4]; result += hex[*p & 15]; }
  }
  return result;
}

bool DashboardGetJson(const String& url, JsonDocument& doc, const char* service) {
  WiFiClientSecure client;
  client.setCACert(DASHBOARD_CA);
  client.setHandshakeTimeout(12);
  HTTPClient http;
  http.setConnectTimeout(10000);
  http.setTimeout(15000);
  // ContentService redirects to script.googleusercontent.com.
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  if (!http.begin(client, url)) return false;
  int code = http.GET();
  bool ok = false;
  if (code == HTTP_CODE_OK) {
    // getString decodes HTTP chunked responses before JSON parsing.
    String payload = http.getString();
    if (payload.length() <= 16384) ok = !deserializeJson(doc, payload);
  }
  http.end();
  Serial.printf("%s: HTTP %d, JSON %s\n", service, code, ok ? "OK" : "no disponible");
  return ok;
}

bool ApplyDashboardSection(const char* key, JsonObjectConst data, bool fresh) {
  if (data["ok"] != true) return false;
  time_t now = time(nullptr);
  long long fetchedAt = data["fetchedAt"] | 0LL;
  if (fetchedAt <= 0 || fetchedAt > now + 300 || now - fetchedAt > DASHBOARD_CACHE_MAX_AGE) return false;
  String section(key);
  if (section == "air") {
    if (!data["aqi"].is<int>() || data["aqi"].as<int>() < 1 || data["aqi"].as<int>() > 5 ||
        !data["pm25"].is<float>() || !data["pm10"].is<float>() ||
        data["pm25"].as<float>() < 0 || data["pm10"].as<float>() < 0) return false;
    dashboardData.aqi = data["aqi"];
    dashboardData.pm25 = data["pm25"];
    dashboardData.pm10 = data["pm10"];
    dashboardData.airAvailable = true; dashboardData.airFresh = fresh;
  } else if (section == "calendar") {
    struct tm today = {};
    localtime_r(&now, &today);
    if (data["month"].as<int>() != today.tm_mon + 1 || data["year"].as<int>() != today.tm_year + 1900 ||
        !data["events"].is<JsonArrayConst>() || !data["days"].is<JsonArrayConst>()) return false;
    dashboardData.month = data["month"]; dashboardData.year = data["year"];
    memset(dashboardData.eventDays, 0, sizeof(dashboardData.eventDays));
    for (JsonVariantConst day : data["days"].as<JsonArrayConst>()) {
      int d = day.as<int>(); if (d >= 1 && d <= 31) dashboardData.eventDays[d] = true;
    }
    dashboardData.eventCount = 0;
    for (JsonObjectConst event : data["events"].as<JsonArrayConst>()) {
      if (!event["title"].is<const char*>() || !event["date"].is<const char*>() ||
          !event["time"].is<const char*>() || (event["endEpoch"] | 0LL) <= now) continue;
      if (dashboardData.eventCount == 3) break;
      DashboardEvent& target = dashboardData.events[dashboardData.eventCount++];
      target.date = event["date"].as<String>(); target.hour = event["time"].as<String>();
      target.title = event["title"].as<String>(); target.endEpoch = event["endEpoch"];
    }
    dashboardData.calendarAvailable = true; dashboardData.calendarFresh = fresh;
  } else if (section == "tasks") {
    if (!data["items"].is<JsonArrayConst>()) return false;
    for (JsonVariantConst task : data["items"].as<JsonArrayConst>()) if (!task.is<const char*>()) return false;
    dashboardData.taskCount = 0;
    for (JsonVariantConst task : data["items"].as<JsonArrayConst>()) {
      if (dashboardData.taskCount < 4) dashboardData.tasks[dashboardData.taskCount++] = task.as<String>();
    }
    dashboardData.tasksAvailable = true; dashboardData.tasksFresh = fresh;
  } else if (section == "note") {
    if (!data["text"].is<const char*>()) return false;
    dashboardData.note = data["text"].as<String>();
    dashboardData.noteAvailable = true; dashboardData.noteFresh = fresh;
  } else if (section == "mail") {
    if (!data["unread"].is<int>() || data["unread"].as<int>() < 0) return false;
    dashboardData.unreadMail = data["unread"];
    dashboardData.mailAvailable = true; dashboardData.mailFresh = fresh;
  } else return false;
  return true;
}

void StoreDashboardSection(const char* key, JsonObjectConst data) {
  if (!ApplyDashboardSection(key, data, true)) return;
  Preferences cache;
  if (cache.begin("dashboard", false)) {
    if (strcmp(key, "air") && cache.getString("origin", "") != GOOGLE_DASHBOARD_URL) {
      // A new endpoint must not inherit personal sections from the old account.
      const char* personalKeys[] = {"calendar", "tasks", "note", "mail"};
      for (const char* personalKey : personalKeys) cache.remove(personalKey);
      cache.putString("origin", GOOGLE_DASHBOARD_URL);
    }
    String serialized; serializeJson(data, serialized);
    cache.putString(key, serialized);
    cache.end();
  }
}

void LoadDashboardCache() {
  Preferences cache;
  if (!cache.begin("dashboard", true)) return;
  const char* keys[] = {"air", "calendar", "tasks", "note", "mail"};
  for (const char* key : keys) {
    // Do not show personal cache when Google integration has been disabled.
    if (strcmp(key, "air") && (!GoogleDashboardConfigured() || cache.getString("origin", "") != GOOGLE_DASHBOARD_URL)) continue;
    String saved = cache.getString(key, "");
    JsonDocument doc;
    if (saved.length() && !deserializeJson(doc, saved)) ApplyDashboardSection(key, doc.as<JsonObjectConst>(), false);
  }
  cache.end();
}

void FetchDashboardAir() {
  JsonDocument response;
  String url = "https://api.openweathermap.org/data/2.5/air_pollution?lat=" + LAT + "&lon=" + LON + "&appid=" + apikey;
  if (!DashboardGetJson(url, response, "OpenWeather aire")) return;
  JsonObjectConst entry = response["list"][0];
  // OpenWeather documents AQI as both 4 and 4.0. Accept either representation.
  if (!entry["main"]["aqi"].is<float>() || !entry["components"]["pm2_5"].is<float>() ||
      !entry["components"]["pm10"].is<float>()) return;
  float aqi = entry["main"]["aqi"].as<float>();
  if (aqi < 1 || aqi > 5 || aqi != static_cast<int>(aqi)) return;
  JsonDocument normalized;
  normalized["ok"] = true; normalized["fetchedAt"] = static_cast<long long>(time(nullptr));
  normalized["aqi"] = static_cast<int>(aqi);
  normalized["pm25"] = entry["components"]["pm2_5"]; normalized["pm10"] = entry["components"]["pm10"];
  StoreDashboardSection("air", normalized.as<JsonObjectConst>());
}

void FetchDashboardGoogle() {
  if (!GoogleDashboardConfigured()) { Serial.println("Google: pendiente de configurar dashboard_config.h"); return; }
  String endpoint(GOOGLE_DASHBOARD_URL);
  if (!endpoint.startsWith("https://script.google.com/macros/s/") || !endpoint.endsWith("/exec")) {
    Serial.println("Google: URL /exec no válida"); return;
  }
  JsonDocument response;
  if (!DashboardGetJson(endpoint + "?token=" + DashboardEncode(GOOGLE_DASHBOARD_TOKEN), response, "Google")) return;
  if (!response["schemaVersion"].is<int>() || response["schemaVersion"].as<int>() != 1
      || !response["ok"].is<bool>() || !response["ok"].as<bool>()) {
    Serial.println("Google: respuesta rechazada; comprobar token y despliegue"); return;
  }
  const char* keys[] = {"calendar", "tasks", "note", "mail"};
  for (const char* key : keys) {
    JsonObjectConst data = response[key];
    if (data["ok"] == true) StoreDashboardSection(key, data);
    else Serial.printf("Google %s: no disponible\n", key);
  }
}
