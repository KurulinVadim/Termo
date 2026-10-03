#pragma once
#include <Arduino.h>
static const char webPage[] PROGMEM = R"HTML(<!doctype html>
<html lang="ru"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Termo</title><style>
*{box-sizing:border-box}body{margin:0;background:#101820;color:#edf4f7;font:17px system-ui}main{max-width:700px;margin:40px auto;padding:20px}h1{margin:0}p{color:#afc3d0}.grid{display:grid;grid-template-columns:1fr 1fr;gap:16px}.card{background:#1c2b36;border-radius:16px;padding:24px;margin:16px 0}.grid .card{margin:0}.value{font-size:36px;font-weight:650;margin-top:12px}label{display:block;margin-bottom:12px}input,button{font:inherit;border-radius:9px;padding:12px;border:0}input{width:90px}button{background:#72dfbc;color:#112c25;cursor:pointer;margin:6px 8px 6px 0}button:disabled{opacity:.4;cursor:default}.off{background:#d3dce2}#message{min-height:25px}small{color:#afc3d0}@media(max-width:450px){.grid{grid-template-columns:1fr}main{margin:12px auto}}
</style><main><h1>Termo</h1><p id="connection" role="status">Подключение…</p>
<div class="grid"><section class="card"><small>Температура сейчас</small><div class="value" id="temperature">—</div></section><section class="card"><small id="averageLabel">Средняя температура</small><div class="value" id="average">—</div></section></div>
<section class="card"><h2>Усреднение</h2><form id="settings"><label for="minutes">Период, от 1 до 60 минут</label><input id="minutes" type="number" min="1" max="60" step="1" required value="5"> <button disabled>Сохранить</button></form></section>
<section class="card"><h2>Насос · ручное управление</h2><p id="pump">Состояние неизвестно</p><button id="on" disabled>Включить</button><button id="off" class="off" disabled>Выключить</button><p><small>Показано состояние выхода реле. Обратной связи от насоса нет. После перезапуска выход выключен.</small></p></section>
<p id="message" role="status"></p></main><script>
let token='',busy=false,online=false,initialized=false;
const $=id=>document.getElementById(id);
function controls(){document.querySelectorAll('button').forEach(b=>b.disabled=busy||!online)}
function degrees(v){return v===null?'Нет данных':v.toFixed(2)+' °C'}
async function refresh(){try{const r=await fetch('/api/status',{cache:'no-store'});if(!r.ok)throw Error();const s=await r.json();token=s.controlToken;online=true;$('temperature').textContent=degrees(s.temperature);$('average').textContent=degrees(s.average);$('averageLabel').textContent='Среднее за '+s.minutes+' мин';$('pump').textContent=s.pumpOn?'Выход реле включён':'Выход реле выключен';$('connection').textContent='На связи · '+s.ip;if(!initialized){$('minutes').value=s.minutes;initialized=true}}catch(e){online=false;$('connection').textContent='Нет связи с устройством';$('temperature').textContent='—';$('average').textContent='—';$('pump').textContent='Состояние неизвестно'}controls()}
async function change(path,data){busy=true;controls();$('message').textContent='Отправка…';try{const r=await fetch(path,{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded','X-Termo-Control':token},body:new URLSearchParams(data)});if(!r.ok)throw Error(await r.text());$('message').textContent='Готово';await refresh()}catch(e){$('message').textContent='Ошибка: '+e.message}finally{busy=false;controls()}}
$('settings').onsubmit=e=>{e.preventDefault();change('/api/average',{minutes:$('minutes').value})};$('on').onclick=()=>change('/api/pump',{on:'1'});$('off').onclick=()=>change('/api/pump',{on:'0'});
(async function poll(){await refresh();setTimeout(poll,2000)})();
</script></html>)HTML";
