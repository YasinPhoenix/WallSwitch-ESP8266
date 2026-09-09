#ifndef WEB_UI_H
#define WEB_UI_H

#include <Arduino.h>

// All page content lives in flash (PROGMEM) as raw string literals and is
// streamed directly via server.send_P() - nothing here is ever copied into
// a RAM buffer or built with String concatenation. Switch count is never
// hardcoded in the HTML/JS: the client derives it from data.r.length in the
// /state response, so this file does not change when SWITCH_COUNT changes.
//
// No external fonts/CDNs anywhere - the device is an access point with no
// internet access while a phone is connected to it, so anything hosted
// elsewhere would just fail to load. Every page is fully self-contained.

static const char MAIN_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fa" dir="rtl">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1">
<title>کنترل کلیدها</title>
<style>
:root{
  --bg:#0f0f11;--surface:#1b1b1f;--surface-2:#232328;--border:#2e2e34;
  --text:#f2f2f4;--muted:#9a9aa4;--accent:#e8a33d;--accent-2:#c9822a;--accent-text:#1a1408;
  --wifi-btn:#2b6f77;--wifi-btn-text:#eafcff;
  --radius-lg:20px;--radius-md:14px;--shadow:0 4px 14px rgba(0,0,0,.35);
}
*{box-sizing:border-box}
body{
  margin:0;background:var(--bg);color:var(--text);
  font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",Tahoma,sans-serif;
  padding:18px;padding-bottom:48px;max-width:480px;margin-inline:auto;
  -webkit-tap-highlight-color:transparent;
}
header{margin-bottom:18px}
header h1{font-size:1.3rem;margin:0;font-weight:800}
header p{margin:4px 0 0;color:var(--muted);font-size:.85rem}

.relay-grid{display:grid;grid-template-columns:repeat(2,1fr);gap:12px}
.relay-btn{
  aspect-ratio:1;border:none;border-radius:var(--radius-lg);
  background:var(--surface-2);color:var(--text);
  display:flex;flex-direction:column;align-items:center;justify-content:center;
  box-shadow:var(--shadow);cursor:pointer;
  transition:background .18s ease,transform .1s ease,box-shadow .18s ease;
  -webkit-user-select:none;user-select:none;
}
.relay-btn:active{transform:scale(.96)}
.relay-btn.on{
  background:linear-gradient(160deg,var(--accent),var(--accent-2));
  color:var(--accent-text);box-shadow:0 4px 18px rgba(232,163,61,.35);
}
.relay-btn .num{font-size:2rem;font-weight:800;line-height:1}
.relay-btn .state{font-size:.85rem;margin-top:6px;opacity:.8;font-weight:600}

details.rgb-panel{
  margin-top:22px;background:var(--surface);border:1px solid var(--border);
  border-radius:var(--radius-md);overflow:hidden;
}
details.rgb-panel summary{
  list-style:none;cursor:pointer;padding:14px 16px;
  display:flex;align-items:center;justify-content:space-between;
  font-size:.95rem;color:var(--muted);font-weight:700;
}
details.rgb-panel summary::-webkit-details-marker{display:none}
details.rgb-panel summary .chev{transition:transform .2s ease;font-size:.8rem}
details.rgb-panel[open] summary .chev{transform:rotate(180deg)}
details.rgb-panel .body{padding:4px 16px 16px}

.switchcard{
  background:var(--surface-2);border:1px solid var(--border);border-radius:var(--radius-md);
  padding:14px;margin-top:12px;
}
.switchcard:first-child{margin-top:0}
.switchcard .title{font-size:.9rem;font-weight:700;margin-bottom:12px;color:var(--muted)}
.colorblock{margin-bottom:16px}
.colorblock:last-child{margin-bottom:0}
.colorblock .label{font-size:.85rem;color:var(--muted);margin-bottom:8px}
.swatches{display:grid;grid-template-columns:repeat(4,1fr);gap:8px}
.swatch{
  aspect-ratio:1;border-radius:10px;border:2px solid var(--border);cursor:pointer;
  transition:transform .1s ease,box-shadow .15s ease;
}
.swatch:active{transform:scale(.92)}
.swatch.selected{box-shadow:0 0 0 2px var(--surface-2),0 0 0 4px var(--text)}
.sw-0{background:#2b2b2b}.sw-1{background:#e74c3c}.sw-2{background:#2ecc71}.sw-3{background:#3498db}
.sw-4{background:#f1c40f}.sw-5{background:#1abc9c}.sw-6{background:#9b59b6}.sw-7{background:#ffffff}

.wifi-link{
  display:flex;align-items:center;justify-content:center;gap:8px;
  margin-top:26px;padding:14px;border-radius:var(--radius-md);
  background:var(--wifi-btn);color:var(--wifi-btn-text);
  text-decoration:none;font-size:.95rem;font-weight:700;box-shadow:var(--shadow);
}
.wifi-link:active{transform:scale(.98)}

.loading{grid-column:1/-1;color:var(--muted);text-align:center;padding:24px;font-size:.9rem}
</style>
</head>
<body>
<header>
  <h1>کنترل کلیدها</h1>
  <p>برای روشن یا خاموش کردن هر کلید، روی آن ضربه بزنید</p>
</header>

<div id="relays" class="relay-grid">
  <div class="loading">در حال بارگذاری...</div>
</div>

<details class="rgb-panel">
  <summary>تنظیمات رنگ <span class="chev">&#9662;</span></summary>
  <div class="body" id="colors"></div>
</details>

<a class="wifi-link" href="/wifi">&#9881;&#65039; تنظیمات Wi-Fi دستگاه</a>

<script>
var COLOR_NAMES = ["خاموش","قرمز","سبز","آبی","زرد","فیروزه‌ای","بنفش","سفید"];
var relaysEl = document.getElementById('relays');
var colorsEl = document.getElementById('colors');

function swatchRow(switchIndex, which, current) {
  var wrap = document.createElement('div');
  wrap.className = 'swatches';
  for (var c = 0; c < 8; c++) {
    (function (c) {
      var sw = document.createElement('div');
      sw.className = 'swatch sw-' + c + (c === current ? ' selected' : '');
      sw.title = COLOR_NAMES[c];
      sw.onclick = function () { setColor(switchIndex, which, c); };
      wrap.appendChild(sw);
    })(c);
  }
  return wrap;
}

function render(data) {
  relaysEl.innerHTML = '';
  colorsEl.innerHTML = '';
  var count = data.r.length;
  for (var i = 0; i < count; i++) {
    (function (i) {
      var btn = document.createElement('button');
      btn.className = 'relay-btn' + (data.r[i] ? ' on' : '');
      var num = document.createElement('div');
      num.className = 'num';
      num.textContent = i + 1;
      var state = document.createElement('div');
      state.className = 'state';
      state.textContent = data.r[i] ? 'روشن' : 'خاموش';
      btn.appendChild(num);
      btn.appendChild(state);
      btn.onclick = function () { setRelay(i, data.r[i] ? 0 : 1); };
      relaysEl.appendChild(btn);

      var sc = document.createElement('div');
      sc.className = 'switchcard';
      var title = document.createElement('div');
      title.className = 'title';
      title.textContent = 'کلید ' + (i + 1);
      sc.appendChild(title);

      var onBlock = document.createElement('div');
      onBlock.className = 'colorblock';
      var onLabel = document.createElement('div');
      onLabel.className = 'label';
      onLabel.textContent = 'رنگ هنگام روشن بودن';
      onBlock.appendChild(onLabel);
      onBlock.appendChild(swatchRow(i, 1, data.on[i]));
      sc.appendChild(onBlock);

      var offBlock = document.createElement('div');
      offBlock.className = 'colorblock';
      var offLabel = document.createElement('div');
      offLabel.className = 'label';
      offLabel.textContent = 'رنگ هنگام خاموش بودن';
      offBlock.appendChild(offLabel);
      offBlock.appendChild(swatchRow(i, 0, data.off[i]));
      sc.appendChild(offBlock);

      colorsEl.appendChild(sc);
    })(i);
  }
}

function loadState() {
  fetch('/state').then(function (r) { return r.json(); }).then(render);
}

function setRelay(i, s) {
  fetch('/relay', {
    method: 'POST',
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
    body: 'i=' + i + '&s=' + s
  }).then(loadState);
}

function setColor(i, which, c) {
  fetch('/rgb', {
    method: 'POST',
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
    body: 'i=' + i + '&w=' + which + '&c=' + c
  }).then(loadState);
}

loadState();
setInterval(loadState, 1000);
</script>
</body>
</html>
)rawliteral";

static const char WIFI_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fa" dir="rtl">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1">
<title>تنظیمات Wi-Fi</title>
<style>
:root{
  --bg:#0f0f11;--surface:#1b1b1f;--surface-2:#232328;--border:#2e2e34;
  --text:#f2f2f4;--muted:#9a9aa4;--accent:#e8a33d;--accent-2:#c9822a;--accent-text:#1a1408;
  --radius-lg:20px;--radius-md:14px;--shadow:0 4px 14px rgba(0,0,0,.35);
}
*{box-sizing:border-box}
body{
  margin:0;background:var(--bg);color:var(--text);
  font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",Tahoma,sans-serif;
  padding:18px;max-width:480px;margin-inline:auto;
}
header h1{font-size:1.25rem;margin:0 0 4px;font-weight:800}
header p{margin:0 0 22px;color:var(--muted);font-size:.85rem}
.field{margin-bottom:16px}
label{display:block;margin-bottom:6px;color:var(--muted);font-size:.85rem;font-weight:600}
input[type=text],input[type=password]{
  width:100%;padding:13px 14px;border-radius:var(--radius-md);border:1px solid var(--border);
  background:var(--surface);color:var(--text);font-size:1rem;
}
input:focus{outline:2px solid var(--accent)}
.checkline{
  display:flex;align-items:center;gap:10px;margin-bottom:18px;
  background:var(--surface);border:1px solid var(--border);border-radius:var(--radius-md);padding:12px 14px;
}
.checkline input{width:20px;height:20px;accent-color:var(--accent)}
.checkline label{margin:0;color:var(--text);font-size:.9rem;font-weight:500}
button{
  width:100%;padding:15px;border:none;border-radius:var(--radius-md);
  background:linear-gradient(160deg,var(--accent),var(--accent-2));color:var(--accent-text);
  font-size:1rem;font-weight:800;cursor:pointer;box-shadow:var(--shadow);
}
button:active{transform:scale(.98)}
.note{
  color:var(--muted);font-size:.82rem;margin-top:18px;line-height:1.7;
  background:var(--surface);border:1px solid var(--border);border-radius:var(--radius-md);padding:12px 14px;
}
a.back{display:block;margin-top:22px;color:var(--muted);text-align:center;text-decoration:none;font-size:.85rem}
</style>
</head>
<body>
<header>
  <h1>تنظیمات Wi-Fi دستگاه</h1>
  <p>نام و رمز شبکه‌ای که دستگاه پخش می‌کند را تغییر دهید</p>
</header>
<form method="POST" action="/wifi">
  <div class="field">
    <label for="ssid">نام شبکه (SSID)</label>
    <input type="text" id="ssid" name="ssid" maxlength="32" required>
  </div>
  <div class="checkline">
    <input type="checkbox" id="open" name="open" onchange="document.getElementById('pw').disabled=this.checked;">
    <label for="open">بدون رمز عبور (باز)</label>
  </div>
  <div class="field">
    <label for="pw">رمز عبور (حداقل ۸ کاراکتر)</label>
    <input type="password" id="pw" name="password" maxlength="63">
  </div>
  <button type="submit">ذخیره و راه‌اندازی مجدد</button>
</form>
<div class="note">
پس از ذخیره، دستگاه راه‌اندازی مجدد می‌شود و باید به شبکه Wi-Fi جدید متصل شوید.<br><br>
رمز را فراموش کرده‌اید؟ تمام لمسی‌های دستگاه را همزمان و به مدت ۱۰ ثانیه نگه دارید تا دستگاه به تنظیمات پیش‌فرض بازگردد.
</div>
<a class="back" href="/">بازگشت به کنترل کلیدها</a>
<script>
fetch('/wifi/state').then(function (r) { return r.json(); }).then(function (d) {
  document.getElementById('ssid').value = d.ssid;
});
</script>
</body>
</html>
)rawliteral";

static const char RESTART_NOTICE_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fa" dir="rtl">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>در حال راه‌اندازی مجدد</title>
<style>
body{
  margin:0;background:#0f0f11;color:#f2f2f4;
  font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",Tahoma,sans-serif;
  display:flex;align-items:center;justify-content:center;height:100vh;text-align:center;padding:24px;
}
div{background:#1b1b1f;border:1px solid #2e2e34;border-radius:16px;padding:24px;line-height:1.8}
</style>
</head>
<body>
<div>تنظیمات ذخیره شد.<br>دستگاه در حال راه‌اندازی مجدد است...<br>لطفاً به شبکه Wi-Fi جدید متصل شوید.</div>
</body>
</html>
)rawliteral";

#endif // WEB_UI_H