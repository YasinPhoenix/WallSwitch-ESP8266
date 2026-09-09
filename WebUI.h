#ifndef WEB_UI_H
#define WEB_UI_H

#include <Arduino.h>

// All page content lives in flash (PROGMEM) as raw string literals and is
// streamed directly via server.send_P() - nothing here is ever copied into
// a RAM buffer or built with String concatenation. Switch count is never
// hardcoded in the HTML/JS: the client derives it from data.r.length in the
// /state response, so this file does not change when SWITCH_COUNT changes.

static const char MAIN_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fa" dir="rtl">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1">
<title>کنترل کلیدها</title>
<style>
:root{--bg:#121212;--card:#1e1e1e;--border:#2c2c2c;--text:#f2f2f2;--muted:#9a9a9a;--on:#e8a33d;--off:#3a3a3a}
*{box-sizing:border-box}
body{margin:0;background:var(--bg);color:var(--text);font-family:Tahoma,"Segoe UI",sans-serif;padding:16px;padding-bottom:40px}
h1{font-size:1.2rem;margin:0 0 16px}
h2{font-size:1rem;color:var(--muted);margin:28px 0 12px;border-top:1px solid var(--border);padding-top:20px}
.card{background:var(--card);border:1px solid var(--border);border-radius:12px;padding:14px 16px;margin-bottom:10px;display:flex;align-items:center;justify-content:space-between}
.card .name{font-size:1.05rem}
.toggle{border:none;border-radius:24px;width:64px;height:34px;position:relative;background:var(--off);cursor:pointer;flex-shrink:0;padding:0}
.toggle .dot{position:absolute;top:3px;right:3px;width:28px;height:28px;border-radius:50%;background:#fff;transition:transform .15s}
.toggle.on{background:var(--on)}
.toggle.on .dot{transform:translateX(-30px)}
.switchcard{background:var(--card);border:1px solid var(--border);border-radius:12px;padding:14px 16px;margin-bottom:14px}
.switchcard .title{font-size:1.05rem;margin-bottom:12px}
.colorblock{margin-bottom:18px}
.colorblock:last-child{margin-bottom:0}
.colorblock .label{font-size:.9rem;color:var(--muted);margin-bottom:8px}
.swatches{display:flex;flex-wrap:wrap;gap:8px}
.swatch{width:42px;height:42px;border-radius:10px;border:2px solid var(--border);cursor:pointer}
.swatch.selected{border-color:var(--text)}
.sw-0{background:#2b2b2b}.sw-1{background:#e74c3c}.sw-2{background:#2ecc71}.sw-3{background:#3498db}
.sw-4{background:#f1c40f}.sw-5{background:#1abc9c}.sw-6{background:#9b59b6}.sw-7{background:#ffffff}
.footer-link{display:block;text-align:center;margin-top:30px;color:var(--muted);text-decoration:none;font-size:.9rem;padding:12px}
.loading{color:var(--muted);text-align:center;padding:20px}
</style>
</head>
<body>
<h1>کنترل کلیدها</h1>
<div id="relays" class="loading">در حال بارگذاری...</div>

<h2>تنظیمات رنگ</h2>
<div id="colors"></div>

<a class="footer-link" href="/wifi">تنظیمات Wi-Fi</a>

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
      var card = document.createElement('div');
      card.className = 'card';
      var name = document.createElement('div');
      name.className = 'name';
      name.textContent = 'کلید ' + (i + 1);
      var btn = document.createElement('button');
      btn.className = 'toggle' + (data.r[i] ? ' on' : '');
      var dot = document.createElement('div');
      dot.className = 'dot';
      btn.appendChild(dot);
      btn.onclick = function () { setRelay(i, data.r[i] ? 0 : 1); };
      card.appendChild(name);
      card.appendChild(btn);
      relaysEl.appendChild(card);

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
setInterval(loadState, 4000);
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
:root{--bg:#121212;--card:#1e1e1e;--border:#2c2c2c;--text:#f2f2f2;--muted:#9a9a9a;--accent:#e8a33d}
*{box-sizing:border-box}
body{margin:0;background:var(--bg);color:var(--text);font-family:Tahoma,"Segoe UI",sans-serif;padding:16px}
h1{font-size:1.2rem}
.field{margin-bottom:16px}
label{display:block;margin-bottom:6px;color:var(--muted);font-size:.9rem}
input[type=text],input[type=password]{width:100%;padding:12px;border-radius:8px;border:1px solid var(--border);background:var(--card);color:var(--text);font-size:1rem}
.checkline{display:flex;align-items:center;gap:8px;margin-bottom:16px}
.checkline input{width:20px;height:20px}
.checkline label{margin:0}
button{width:100%;padding:14px;border:none;border-radius:8px;background:var(--accent);color:#1a1a1a;font-size:1rem;font-weight:bold;cursor:pointer}
.note{color:var(--muted);font-size:.85rem;margin-top:16px;line-height:1.6}
a.back{display:block;margin-top:20px;color:var(--muted);text-align:center;text-decoration:none}
</style>
</head>
<body>
<h1>تنظیمات Wi-Fi دستگاه</h1>
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
<div class="note">پس از ذخیره، دستگاه راه‌اندازی مجدد می‌شود و باید به شبکه Wi-Fi جدید متصل شوید.</div>
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
body{margin:0;background:#121212;color:#f2f2f2;font-family:Tahoma,sans-serif;display:flex;align-items:center;justify-content:center;height:100vh;text-align:center;padding:20px}
</style>
</head>
<body>
<div>تنظیمات ذخیره شد.<br>دستگاه در حال راه‌اندازی مجدد است...<br>لطفاً به شبکه Wi-Fi جدید متصل شوید.</div>
</body>
</html>
)rawliteral";

#endif // WEB_UI_H
