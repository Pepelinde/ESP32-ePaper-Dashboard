// Read-only dashboard endpoint. Set properties before deploying (see README).
function jsonOutput_(data) {
  return ContentService.createTextOutput(JSON.stringify(data))
    .setMimeType(ContentService.MimeType.JSON);
}

function doGet(e) {
  const properties = PropertiesService.getScriptProperties();
  const secret = properties.getProperty('DASHBOARD_TOKEN');
  // Reject unauthenticated calls before touching Calendar, Tasks or Gmail.
  if (!secret || secret.length < 32 || !e || !e.parameter || e.parameter.token !== secret) {
    return jsonOutput_({schemaVersion: 1, ok: false, error: 'unauthorized'});
  }
  return jsonOutput_(buildDashboard_());
}

function cleanText_(value, maxLength) {
  return Array.from(String(value || '').replace(/\s+/g, ' ').trim()).slice(0, maxLength).join('');
}

function buildDashboard_() {
  const properties = PropertiesService.getScriptProperties();
  const now = new Date();
  const timezone = Session.getScriptTimeZone();
  const fetchedAt = Math.floor(now.getTime() / 1000);
  function section(name, fetch) {
    try { return Object.assign({ok: true, fetchedAt: fetchedAt}, fetch()); }
    catch (error) {
      // Never return service errors or personal data to unauthorized clients.
      console.warn(name + ': ' + error.message);
      return {ok: false, error: 'service_unavailable'};
    }
  }
  return {
    schemaVersion: 1, ok: true,
    calendar: section('calendar', () => readCalendar_(properties, now, timezone)),
    tasks: section('tasks', () => readTasks_(properties)),
    note: section('note', () => ({text: cleanText_(properties.getProperty('DASHBOARD_NOTE'), 180)})),
    // UNREAD label counts unread messages, not unread conversation threads.
    mail: section('mail', () => {
      const label = Gmail.Users.Labels.get('me', 'UNREAD');
      if (!Number.isInteger(label.messagesUnread) || label.messagesUnread < 0) throw new Error('Invalid unread count');
      return {unread: label.messagesUnread};
    })
  };
}

function readCalendar_(properties, now, timezone) {
  const year = Number(Utilities.formatDate(now, timezone, 'yyyy'));
  const month = Number(Utilities.formatDate(now, timezone, 'MM'));
  const pad = n => String(n).padStart(2, '0');
  const monthStart = year + '-' + pad(month) + '-01';
  const nextMonth = month === 12 ? (year + 1) + '-01-01' : year + '-' + pad(month + 1) + '-01';
  const timeMin = Utilities.parseDate(monthStart, timezone, 'yyyy-MM-dd').toISOString();
  const timeMax = Utilities.parseDate(nextMonth, timezone, 'yyyy-MM-dd').toISOString();
  const calendarId = properties.getProperty('CALENDAR_ID') || 'primary';
  const events = [];
  const days = new Set();
  let pageToken;
  do {
    const options = {timeMin: timeMin, timeMax: timeMax, singleEvents: true,
      orderBy: 'startTime', maxResults: 2500, timeZone: timezone};
    if (pageToken) options.pageToken = pageToken;
    const page = Calendar.Events.list(calendarId, options);
    (page.items || []).forEach(event => {
      if (event.status === 'cancelled' || !event.start || !event.end) return;
      const allDay = !!event.start.date;
      const start = allDay ? Utilities.parseDate(event.start.date, timezone, 'yyyy-MM-dd') : new Date(event.start.dateTime);
      const end = allDay ? Utilities.parseDate(event.end.date, timezone, 'yyyy-MM-dd') : new Date(event.end.dateTime);
      const startDate = Utilities.formatDate(start, timezone, 'yyyy-MM-dd');
      // Mark every occupied day; an all-day end date is exclusive.
      const endDate = Utilities.formatDate(new Date(end.getTime() - 1), timezone, 'yyyy-MM-dd');
      const daysInMonth = new Date(Date.UTC(year, month, 0)).getUTCDate();
      for (let day = 1; day <= daysInMonth; ++day) {
        const date = year + '-' + pad(month) + '-' + pad(day);
        if (date >= startDate && date <= endDate) days.add(day);
      }
      if (end.getTime() > now.getTime()) events.push({
        date: startDate < monthStart ? monthStart : startDate,
        time: allDay ? 'Todo el día' : Utilities.formatDate(start, timezone, 'HH:mm'),
        title: cleanText_(event.summary || '(Sin título)', 90),
        endEpoch: Math.floor(end.getTime() / 1000), startEpoch: Math.floor(start.getTime() / 1000)
      });
    });
    pageToken = page.nextPageToken;
  } while (pageToken);
  events.sort((a, b) => a.startEpoch - b.startEpoch);
  return {month: month, year: year, days: Array.from(days).sort((a, b) => a - b), events: events.slice(0, 12)};
}

function readTasks_(properties) {
  // Default Google Tasks list, or the specific ID configured by the user.
  const listId = properties.getProperty('TASK_LIST_ID') || '@default';
  const items = [];
  let pageToken;
  do {
    const options = {showCompleted: false, showDeleted: false, showHidden: false, maxResults: 100};
    if (pageToken) options.pageToken = pageToken;
    const page = Tasks.Tasks.list(listId, options);
    (page.items || []).forEach(task => {
      if (task.status === 'needsAction' && !task.deleted ) items.push({title: cleanText_(task.title, 90), updated: task.updated || ''});
    });
    pageToken = page.nextPageToken;
  } while (pageToken);
  items.sort((a, b) => b.updated.localeCompare(a.updated));
  return {items: items.slice(0, 4).map(task => task.title)};
}

// Run once in the editor to authorize read-only services.
// Logs only availability, never event titles, notes or the token.
function testDashboard() {
  const data = buildDashboard_();
  console.log(JSON.stringify({calendar: data.calendar.ok, tasks: data.tasks.ok,
    note: data.note.ok, mail: data.mail.ok}));
}
