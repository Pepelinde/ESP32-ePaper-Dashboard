// Personal dashboard for the Waveshare 7.5-inch 800x480 display.
// Live data and explicit unavailable/cached states.
#include "dashboard_photo.h"
void drawString(int x, int y, String text, alignment align);
void DisplayConditionsSection(int x, int y, String icon, bool size);

String DashboardLabel(const char* title, bool available, bool fresh) {
  return String(title) + (available && !fresh ? " · GUARDADO" : "");
}

String GoogleUnavailableText() {
  return GoogleDashboardConfigured() ? "No disponible" : "Configurar Google";
}

void DashboardText(int x, int baseline, String text, int maxWidth = 230) {
  while (text.length() && u8g2Fonts.getUTF8Width(text.c_str()) > maxWidth) {
    // Remove a complete UTF-8 character, never half of an accented letter.
    int last = text.length() - 1;
    while (last > 0 && (static_cast<unsigned char>(text[last]) & 0xC0) == 0x80) --last;
    text.remove(last);
  }
  u8g2Fonts.setCursor(x, baseline);
  u8g2Fonts.print(text);
}

void DisplayPersonalDashboard() {
  const char* months[] = {"ENERO", "FEBRERO", "MARZO", "ABRIL", "MAYO", "JUNIO",
                          "JULIO", "AGOSTO", "SEPTIEMBRE", "OCTUBRE", "NOVIEMBRE", "DICIEMBRE"};
  const char* weekdays[] = {"DOM", "LUN", "MAR", "MIÉ", "JUE", "VIE", "SÁB"};
  struct tm now = {};
  bool validTime = getLocalTime(&now, 10);
  char clock[6] = "--:--";
  if (validTime) strftime(clock, sizeof(clock), "%H:%M", &now);
  u8g2Fonts.setFont(u8g2_font_luRS10_tf);
  DashboardText(14, 18, validTime ? String(now.tm_mday) + " " + months[now.tm_mon] + " " + String(now.tm_year + 1900) : "FECHA NO DISPONIBLE", 320);
  u8g2Fonts.setFont(u8g2_font_luBS08_tf);
  String connection = String("WiFi OK · Actualizado ") + clock;
  DashboardText(786 - u8g2Fonts.getUTF8Width(connection.c_str()), 18, connection, 340);
  display.drawFastHLine(0, 25, 800, GxEPD_BLACK);
  display.drawFastVLine(255, 36, 430, GxEPD_BLACK);
  display.drawFastVLine(545, 36, 430, GxEPD_BLACK);

  // Calendar starts on Monday; mktime handles leap years and month lengths.
  u8g2Fonts.setFont(u8g2_font_luBS10_tf);
  DashboardText(14, 45, "CALENDARIO");
  int calendarBottom = 229;
  if (validTime) {
    u8g2Fonts.setFont(u8g2_font_luRS10_tf);
    DashboardText(14, 77, String(months[now.tm_mon]) + " " + String(now.tm_year + 1900));
    const char* letters[] = {"L", "M", "X", "J", "V", "S", "D"};
    struct tm first = now;
    first.tm_mday = 1;
    first.tm_hour = 12;
    first.tm_isdst = -1;
    mktime(&first);
    int offset = (first.tm_wday + 6) % 7;
    struct tm last = first;
    ++last.tm_mon;
    last.tm_mday = 0;
    last.tm_isdst = -1;
    mktime(&last);
    calendarBottom = 129 + ((offset + last.tm_mday - 1) / 7) * 25;
    for (int col = 0; col < 7; ++col) DashboardText(20 + col * 33, 103, letters[col], 28);
    for (int day = 1; day <= last.tm_mday; ++day) {
      int cell = offset + day - 1;
      int x = 20 + (cell % 7) * 33;
      int y = 129 + (cell / 7) * 25;
      bool selected = day == now.tm_mday;
      if (selected) {
        display.fillRoundRect(x - 6, y - 17, 31, 24, 4, GxEPD_BLACK);
        u8g2Fonts.setForegroundColor(GxEPD_WHITE);
        u8g2Fonts.setBackgroundColor(GxEPD_BLACK);
      }
      if (dashboardData.calendarAvailable && dashboardData.eventDays[day]) display.fillCircle(x + 21, y - 3, 2, selected ? GxEPD_WHITE : GxEPD_BLACK);
      DashboardText(x, y, String(day), 28);
      u8g2Fonts.setForegroundColor(GxEPD_BLACK);
      u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
    }
  }
  u8g2Fonts.setFont(u8g2_font_luRS18_tf);
  int clockBaseline = calendarBottom + 30;
  DashboardText(14, clockBaseline, clock);
  u8g2Fonts.setFont(u8g2_font_luBS08_tf);
  DashboardText(14, clockBaseline + 18, "Último refresco");
  display.drawFastHLine(14, clockBaseline + 27, 225, GxEPD_BLACK);
  u8g2Fonts.setFont(u8g2_font_luBS10_tf);
  DashboardText(14, clockBaseline + 48, DashboardLabel("EVENTOS", dashboardData.calendarAvailable, dashboardData.calendarFresh));
  u8g2Fonts.setFont(u8g2_font_luBS08_tf);
  if (!dashboardData.calendarAvailable) DashboardText(14, clockBaseline + 73, GoogleUnavailableText());
  else if (!dashboardData.eventCount) DashboardText(14, clockBaseline + 73, "Sin próximos eventos este mes");
  for (int i = 0; i < min(3, dashboardData.eventCount); ++i) {
    const DashboardEvent& event = dashboardData.events[i];
    int y = clockBaseline + 65 + i * 30;
    u8g2Fonts.setFont(u8g2_font_luBS08_tf);
    DashboardText(14, y, event.date.substring(8, 10) + "  " + event.hour);
    u8g2Fonts.setFont(u8g2_font_luRS10_tf);
    DashboardText(14, y + 18, event.title);
  }

  display.drawFastHLine(14, 435, 225, GxEPD_BLACK);
  u8g2Fonts.setFont(u8g2_font_luBS10_tf);
  DashboardText(14, 450, DashboardLabel("CORREO", dashboardData.mailAvailable, dashboardData.mailFresh));
  u8g2Fonts.setFont(u8g2_font_luRS14_tf);
  DashboardText(14, 474, dashboardData.mailAvailable ? String(dashboardData.unreadMail) + " sin leer" : GoogleUnavailableText());

  u8g2Fonts.setFont(u8g2_font_luBS10_tf);
  DashboardText(270, 45, "TIEMPO · " + City, 260);
  dashboardIconScale = 7;
  DisplayConditionsSection(316, 91, WxConditions[0].Icon, SmallIcon);
  dashboardIconScale = 5;
  u8g2Fonts.setFont(u8g2_font_logisoso32_tn);
  String temperature = String(WxConditions[0].Temperature, 0);
  int unitX = min(493, 367 + static_cast<int>(u8g2Fonts.getUTF8Width(temperature.c_str())) + 9);
  DashboardText(367, 113, temperature, 125);
  u8g2Fonts.setFont(u8g2_font_luRS14_tf);
  DashboardText(unitX, 113, Units == "M" ? "°C" : "°F", 38);
  u8g2Fonts.setFont(u8g2_font_luRS10_tf);
  DashboardText(270, 138, WxConditions[0].Description, 260);
  u8g2Fonts.setFont(u8g2_font_luRS08_tf);
  DashboardText(270, 156, "Sensación", 120);
  DashboardText(415, 156, "Humedad", 115);
  u8g2Fonts.setFont(u8g2_font_luRS12_tf);
  DashboardText(270, 176, String(WxConditions[0].FeelsLike, 0) + "°", 120);
  DashboardText(415, 176, String(WxConditions[0].Humidity, 0) + " %", 115);
  display.drawFastHLine(270, 188, 260, GxEPD_BLACK);
  u8g2Fonts.setFont(u8g2_font_luBS10_tf);
  DashboardText(270, 205, DashboardLabel("AIRE", dashboardData.airAvailable, dashboardData.airFresh), 260);
  const char* airLabels[] = {"", "Buena", "Aceptable", "Moderada", "Mala", "Muy mala"};
  u8g2Fonts.setFont(u8g2_font_luRS10_tf);
  DashboardText(270, 226, dashboardData.airAvailable ? "AQI " + String(dashboardData.aqi) + " · " + airLabels[dashboardData.aqi] : "No disponible", 260);
  if (dashboardData.airAvailable) {
    u8g2Fonts.setFont(u8g2_font_luRS08_tf);
    DashboardText(270, 245, "PM2.5", 120);
    DashboardText(415, 245, "PM10", 115);
    u8g2Fonts.setFont(u8g2_font_luRS12_tf);
    DashboardText(270, 265, String(dashboardData.pm25, 1), 120);
    DashboardText(415, 265, String(dashboardData.pm10, 1), 115);
    u8g2Fonts.setFont(u8g2_font_luRS08_tf);
    DashboardText(270, 281, "µg/m³", 120);
    DashboardText(415, 281, "µg/m³", 115);
  }

  u8g2Fonts.setFont(u8g2_font_luBS10_tf);
  DashboardText(270, 300, "PREVISIÓN", 180);
  u8g2Fonts.setFont(u8g2_font_luRS08_tf);
  DashboardText(456, 300, "MÁX/MÍN", 74);
  for (int i = 0; i < 4; ++i) {
    int x = 277 + i * 65;
    u8g2Fonts.setFont(u8g2_font_luBS08_tf);
    if (Daily[i + 1].Dt <= 0) { DashboardText(x, 316, "--", 58); continue; }
    time_t localDay = Daily[i + 1].Dt + WxConditions[0].Timezone;
    struct tm forecastDay = {};
    gmtime_r(&localDay, &forecastDay);
    u8g2Fonts.setFont(u8g2_font_luBS08_tf);
    DashboardText(x, 316, weekdays[forecastDay.tm_wday], 58);
    dashboardIconScale = 4;
    DisplayConditionsSection(x + 23, 335, Daily[i + 1].Icon, SmallIcon);
    dashboardIconScale = 5;
    u8g2Fonts.setFont(u8g2_font_luBS08_tf);
    DashboardText(x, 358, String(Daily[i + 1].High, 0) + "°/" + String(Daily[i + 1].Low, 0) + "°", 58);
  }

  display.drawFastHLine(270, 367, 260, GxEPD_BLACK);
  u8g2Fonts.setFont(u8g2_font_luBS10_tf);
  DashboardText(270, 384, DashboardLabel("NOTA", dashboardData.noteAvailable, dashboardData.noteFresh), 260);
  u8g2Fonts.setFont(u8g2_font_luRS10_tf);
  if (!dashboardData.noteAvailable) DashboardText(270, 405, GoogleUnavailableText(), 260);
  else if (!dashboardData.note.length()) DashboardText(270, 405, "Sin nota", 260);
  else {
    String text = dashboardData.note;
    text.replace("\r", " ");
    text.replace("\n", " ");
    for (int line = 0; line < 4 && text.length(); ++line) {
      text.trim();
      int fit = 0;
      int space = -1;
      for (int end = 1; end <= static_cast<int>(text.length()); ++end) {
        if (end < static_cast<int>(text.length()) && (static_cast<unsigned char>(text[end]) & 0xC0) == 0x80) continue;
        if (u8g2Fonts.getUTF8Width(text.substring(0, end).c_str()) > 260) break;
        fit = end;
        if (text[end - 1] == ' ') space = end;
      }
      if (!fit) break;
      int split = fit < static_cast<int>(text.length()) && space > 0 ? space : fit;
      String row = text.substring(0, split);
      row.trim();
      DashboardText(270, 405 + line * 21, row, 260);
      text.remove(0, split);
    }
  }

  u8g2Fonts.setFont(u8g2_font_luBS10_tf);
  DashboardText(560, 45, DashboardLabel("TAREAS", dashboardData.tasksAvailable, dashboardData.tasksFresh));
  u8g2Fonts.setFont(u8g2_font_luRS10_tf);
  if (!dashboardData.tasksAvailable) DashboardText(560, 105, GoogleUnavailableText());
  else if (!dashboardData.taskCount) DashboardText(560, 105, "Sin tareas pendientes");
  for (int i = 0; i < min(3, dashboardData.taskCount); ++i) {
    int y = 93 + i * 38;
    display.drawRect(561, y - 12, 12, 12, GxEPD_BLACK);
    DashboardText(584, y, dashboardData.tasks[i], 201);
  }
  u8g2Fonts.setFont(u8g2_font_luBS08_tf);
  if (dashboardData.taskCount > 3) DashboardText(560, 216, "Hay más tareas pendientes");
  display.drawBitmap(562, 258, DASHBOARD_PHOTO_BITMAP,
                     DASHBOARD_PHOTO_WIDTH, DASHBOARD_PHOTO_HEIGHT,
                     GxEPD_BLACK, GxEPD_WHITE);
  display.drawRect(560, 256, DASHBOARD_PHOTO_WIDTH + 4, DASHBOARD_PHOTO_HEIGHT + 4, GxEPD_BLACK);
}
