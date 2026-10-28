// Local contract tests with mocked Google services; no account or network access.
const assert = require('node:assert/strict');
const fs = require('node:fs');
const vm = require('node:vm');
const source = fs.readFileSync(__dirname + '/Code.gs', 'utf8');
const fixedNow = new Date('2026-10-04T10:00:00Z');
class FixedDate extends Date {
  constructor(...args) { super(...(args.length ? args : [fixedNow.getTime()])); }
}
const props = {DASHBOARD_TOKEN: 'a'.repeat(64), DASHBOARD_NOTE: 'Preparar   la clase\nmañana'};
let calls = 0, tasksFail = false;
const timed = (start, end, summary) => ({start: {dateTime: start}, end: {dateTime: end}, summary});
const pages = [
  {items: [
    timed('2026-10-03T10:00:00+02:00', '2026-10-03T11:00:00+02:00', 'Pasado'),
    timed('2026-10-05T10:00:00+02:00', '2026-10-05T11:00:00+02:00', 'Reunión'),
    {start: {date: '2026-10-06'}, end: {date: '2026-10-08'}, summary: 'Dos días'},
    {...timed('2026-10-09T10:00:00+02:00', '2026-10-09T11:00:00+02:00', 'Cancelado'), status: 'cancelled'},
    {start: {date: '2026-09-30'}, end: {date: '2026-10-02'}, summary: 'Cruza mes'}
  ], nextPageToken: 'second'},
  {items: [timed('2026-10-12T10:00:00+02:00', '2026-10-12T11:00:00+02:00', 'Segundo lote')]}
];
const context = vm.createContext({
  Date: FixedDate, console: {log() {}, warn() {}},
  PropertiesService: {getScriptProperties: () => ({getProperty: k => props[k] || null})},
  Session: {getScriptTimeZone: () => 'Europe/Madrid'},
  Utilities: {
    formatDate(date, timezone, format) {
      const parts = Object.fromEntries(new Intl.DateTimeFormat('en-GB', {timeZone: timezone,
        year: 'numeric', month: '2-digit', day: '2-digit', hour: '2-digit', minute: '2-digit', hourCycle: 'h23'
      }).formatToParts(date).map(p => [p.type, p.value]));
      return {yyyy: parts.year, MM: parts.month, 'yyyy-MM-dd': `${parts.year}-${parts.month}-${parts.day}`,
        'HH:mm': `${parts.hour}:${parts.minute}`}[format];
    },
    parseDate(date) {
      // Midnight in Madrid for the October/November dates in these fixtures.
      return new Date(date + (date >= '2026-10-26' ? 'T00:00:00+01:00' : 'T00:00:00+02:00'));
    }
  },
  Calendar: {Events: {list(id, options) {
    ++calls;
    assert.equal(id, 'primary'); assert.equal(options.singleEvents, true);
    assert.equal(options.timeMin, '2026-09-30T22:00:00.000Z');
    assert.equal(options.timeMax, '2026-10-31T23:00:00.000Z');
    return pages[options.pageToken ? 1 : 0];
  }}},
  Tasks: {Tasks: {list(id, options) {
    ++calls;
    if (tasksFail) throw new Error('Service disabled');
    assert.equal(id, '@default'); assert.equal(options.showCompleted, false);
    return options.pageToken ? {items: [{title: 'Tres', status: 'needsAction', updated: '2026-10-04T10:00:00Z'},
      {title: 'Cuatro', status: 'needsAction', updated: '2026-10-02T10:00:00Z'}, {title: 'Cinco', status: 'needsAction', updated: '2026-10-01T10:00:00Z'}]} :
      {items: [{title: 'Uno', status: 'needsAction', updated: '2026-10-01T10:00:00Z'}, {title: 'Hecha', status: 'completed'},
        {title: 'Dos', status: 'needsAction', updated: '2026-10-03T10:00:00Z'}], nextPageToken: 'next'};
  }}},
  Gmail: {Users: {Labels: {get(user, label) { ++calls; assert.equal(label, 'UNREAD'); return {messagesUnread: 0}; }}}},
  ContentService: {MimeType: {JSON: 'json'}, createTextOutput: text => ({text, setMimeType() {return this;}})}
});
vm.runInContext(source, context);
function invoke(token) { return JSON.parse(context.doGet({parameter: {token}}).text); }
assert.equal(invoke('wrong').ok, false);
assert.equal(calls, 0, 'Unauthorized request must not access Google data');
const result = invoke(props.DASHBOARD_TOKEN);
assert.equal(result.schemaVersion, 1);
assert.deepEqual(result.calendar.days, [1, 3, 5, 6, 7, 12]);
assert.deepEqual(result.calendar.events.map(e => e.title), ['Reunión', 'Dos días', 'Segundo lote']);
assert.equal(result.calendar.events[0].time, '10:00');
assert.equal(result.calendar.events[1].time, 'Todo el día');
assert.deepEqual(result.tasks.items, ['Tres', 'Dos', 'Cuatro', 'Uno']);
assert.equal(result.tasks.items.length > 3, true, 'Fourth item signals additional pending tasks');
assert.equal(result.note.text, 'Preparar la clase mañana');
assert.equal(result.mail.unread, 0, 'Zero unread mail is a successful result');
tasksFail = true;
const partial = invoke(props.DASHBOARD_TOKEN);
assert.equal(partial.tasks.ok, false);
assert.equal(partial.note.ok, true, 'Tasks failure must not affect the manual note');
assert.equal(partial.calendar.ok, true);
assert.equal(partial.mail.ok, true);
props.DASHBOARD_TOKEN = '';
const before = calls;
assert.equal(invoke('').ok, false);
assert.equal(calls, before, 'Blank token must fail closed');
console.log('PASS: auth, calendar pagination/month/DST boundaries/multiday, tasks pagination, note, zero mail, independent failures');
