#include "sub_http.h"
#include "sub_httpjs.h"
#include "sub_bmp.h"
#include "sub_gif.h"
#include "sub_json.h"
#include "sub_led.h"
#include "sub_udp.h"
#include "conf.h"

extern String ssid[];
extern String pass[];
char texthtml[10] = "text/html";
char textplain[11] = "text/plain";
WebServer server(80);
HTTPUpdateServer httpUpdater;
FtpServer ftpSrv;

unsigned char h2int(char c)
{
	if (c >= '0' && c <='9')
	{
		return((unsigned char)c - '0');
	}
	if (c >= 'a' && c <='f')
	{
		return((unsigned char)c - 'a' + 10);
	}
	if (c >= 'A' && c <='F')
	{
		return((unsigned char)c - 'A' + 10);
	}
	return(0);
}

String urldecode(String str)
{
	String encodedString = "";
	char c;
	char code0;
	char code1;
	for (int i =0; i < str.length(); i++)
	{
		c=str.charAt(i);
		if (c == '+')
		{
			encodedString+=' ';
		}
		else if (c == '%')
		{
			i++;
			code0 = str.charAt(i);
			i++;
			code1 = str.charAt(i);
			c = (h2int(code0) << 4) | h2int(code1);
			encodedString += c;
		}
		else
		{
			encodedString+=c;
		}
	yield();
	}
	return encodedString;
}

String urlencode(String str)
{
	String encodedString="";
	char c;
	char code0;
	char code1;
	char code2;
	for (int i =0; i < str.length(); i++)
	{
		c=str.charAt(i);
		if (c == ' ')
		{
			encodedString += '+';
		}
		else if (isalnum(c))
		{
			encodedString += c;
		}
		else
		{
			code1 = (c & 0xf) + '0';
			if ((c & 0xf) >9)
			{
				code1=(c & 0xf) - 10 + 'A';
			}
			c = (c >> 4) & 0xf;
			code0 = c + '0';
			if (c > 9)
			{
				code0 = c - 10 + 'A';
			}
			code2 = '\0';
			encodedString += '%';
			encodedString += code0;
			encodedString += code1;
			//encodedString+=code2;
		}
		yield();
	}
	return encodedString;
}

static const char content_root[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<title>poihd</title>
<meta charset='UTF-8'>
<style>
button {margin: 5px 5px}
table, td {border: 1px solid #ccc}
td {padding: 0 10px}
</style>
</head>
<body>
<a href='/'>main</a>   
<a href='/files'>files</a>   
<a href='/prog'>prog</a>
<br><br>

<table border=1><caption>LED HD <span id='ver'></span></caption>
<tr><td>ip</td><td id=ip></td></tr>
<tr><td>mac</td><td id=mac></td></tr>
<tr><td>maca</td><td id=maca></td></tr>
<tr><td>vcc</td><td id=vcc onclick="r('vcc')"></td></tr>
<tr><td>heap</td><td id='heap' onclick="r('heap')"></td></tr>
<tr><td>prog</td><td id='prog' onclick="r('prog')"></td></tr>
</table><br><br>

<table border=1><caption>управление</caption>
	<tr><td>задержка</td>
		<td><button onclick="r('wait','m')">-</button></td>
		<td id=wait></td>
		<td><button onclick="r('wait','p')">+</button></td></tr>
	<tr><td>яркость</td>
		<td><button onclick="r('brgn','m')">-</button></td>
		<td id=brgn></td>
		<td><button onclick="r('brgn','p')">+</button></td></tr>
	<tr><td>длительность</td>
		<td><input id=bpmv type=number min=10 max=65500 style='width:100px;'></td>
		<td><button onclick="r('bpm',vl('bpmv'))">set</button></td>
		<td id=bpm></td></tr>
	<tr><td>режим</td>
		<td><button onclick="r('mode','3')">файлы</button></td>
		<td><button onclick="r('mode','4')">программы</button></td>
		<td id=mode></td></tr>
	<tr><td>программы</td>
		<td><button onclick="r('prg','m')">-</button></td>
		<td><button onclick="r('prg','p')">+</button></td>
		<td id=prg></td></tr>
	<tr><td>настройки</td>
		<td><button onclick="r('cmt')">сохранить</button></td>
		<td id=cmt></td><td></td></tr>
	<tr><td>запуск</td>
		<td><button onclick="r('go')">go</button></td>
		<td><button onclick="r('stp')">stop</button></td>
		<td></td></tr>
</table><br><br>

<table border=1><caption>Подключаться к точке</caption>
<tr><td id=wfaps></td></tr>
<tr><td>pass: <input id=pass type=password>
<a href='#' class=pwd onclick='return sh_pw(this);'></a><br>
<tr><td><label><input type=radio name=sidx value=1 checked>точка 1</label>
<label><input type=radio name=sidx value=2>точка 2</label>
<input type=button value=сохранить onClick="r('ssid', vr('ssid')+'&p='+vl('pass')+'&i='+vr('sidx'))">&nbsp;
<input type=button value=reset onClick="r('ssid', 'spiffs&p=spiffs&i='+vr('sidx'))"><br><span id='ssid'></span></td></tr>
</table><br>

<table border=1><caption>Название устройства</caption>
<tr><td><input id=wprefv maxlength=15 onkeyup='this.value = this.value.replace(/[^A-Za-z0-9_]/g, "")'>&nbsp;
<input type=button value='сохранить' onClick="r('wpref',vl('wprefv'))"><br><span id=wpref></span></td></tr>
</table><br><br>

<a href='/config'>настройки</a><br><br>
<a href='/update'>обновление</a><br>
<script>
	function vl(id){var el=document.getElementById(id); return el.value;}
	function vr(nm){var ra=document.getElementsByName(nm);for(var i=0;i<ra.length;i++)if(ra[i].checked) return ra[i].value;}
	function r(p,v){fetch('/req?'+p+'='+(v?v:'1')).then((response) => {return response.text();}).then((data) => {document.getElementById(p).innerHTML = data;});}
	function load(v){for(var key in v){var el = document.getElementById(key); el.innerHTML = v[key];}}
	r('wfaps');
)=====";

void handleRoot()
{
	server.setContentLength(CONTENT_LENGTH_UNKNOWN);
	server.send(200, texthtml, content_root);
	server.sendContent(
		"\tload({ver:'" + get_answ("ver", "1") +
		"', ip: '" + get_answ("ip","1") +
		"', mac: '" + get_answ("mac","1") +
		"', maca: '" + get_answ("maca","1") +
		"', vcc: '" + get_answ("vcc", "1") +
		"', heap: '" + get_answ("heap", "1") +
		"', prog: '" + get_answ("prog", "1") +
		"', wait: '" + get_answ("wait","201") + "', brgn: '" + get_answ("brgn","0") +  "', bpm: '" + get_answ("bpm","0") + "'});\n"
	);
	server.sendContent("</script>\n</body>\n</html>");
}

static const char content_files1[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<title>poihd files</title>
<meta charset='UTF-8'>
<style>
td {padding: 0 10px;}
</style>
</head>
<body>
<a href='/'>main</a>   
<a href='/files'>files</a>   
<a href='/prog'>prog</a>
<br><br>
)=====";

static const char content_files2[] PROGMEM = R"=====(
 
<form onsubmit='return confirm("format removes all files,\ncontinue?")' style='display:inline;'>
<input type='hidden' name='format' value='1'><input type='submit' value='format'>
</form><br><br>
<form method='POST' enctype='multipart/form-data' action='/load'>
	Load <input type='file' name='data'><input type='submit' value='load'><br>
	Many <input type='file' id='files' multiple><input type='button' value='load' onclick='send()'> <span id='q'>ok</span><br>
</form><br>
<script>
	var onErr = function (err) {console.warn(err);return new Response('error');}
	async function send() {
		var fl = document.getElementById('files');
		var ans = document.getElementById('q');
		for(var i = 0; i < fl.files.length; i++){
			var fd = new FormData();
			fd.append('data',fl.files[i]);
			let r = await fetch('/load', {method: 'POST', body: fd}).catch(onErr);
			let t = await r.text();
			console.log(t);
			ans.innerHTML = t;
		}
		ans.innerHTML = 'finish';
		setTimeout('location.reload()',1000);
	}
	function showAndroidToast(toast){
		if(typeof Android!=='undefined' && Android!==null){
			Android.showToast(toast);
		} else {
			alert('Click this in android App!');
		}
	}
	function del(name, rid){
		var qwe = confirm('delete ' + name + '?');
		if(qwe) fetch('/del?f='+encodeURI(name))
			.then((response) => {return response.text();})
			.then((data) => {
				alert(data);
				if(data.indexOf('deleted')!=-1){
					document.getElementById('row_'+rid).remove();
				}
		})
	}
</script>
<table border=1>
<tr><td>#</td><td>del</td><td>name</td><td>size</td><td>pic</td><td>h</td><td>w</td><td>bits</td><td>tp</td><td>decode</td></tr>
)=====";

void handleFiles()
{
	server.setContentLength(CONTENT_LENGTH_UNKNOWN);
	if (server.args() > 0){
		String sav =server.arg(0);
		if (server.argName(0) == "format")
		{
			FILESYSTEM.end();
			bool f = FILESYSTEM.format();
			bool b = FILESYSTEM.begin();
			json_save();
			if (conf.mode == 13) bmp_savecost();
			server.send(200, texthtml, "<a href='/files'>back</a><br> format " + String(f ? "ok," : "fail,") + String(b ? " begin" : " not begin"));
			return;
		}
	}
	server.send(200, texthtml, content_files1);
	if (server.args() > 0){
		if (server.argName(0) == "del")
		{
			String sav =server.arg(0);
			if (sav.startsWith("/") == false) sav = "/" + sav;
			if (FILESYSTEM.exists(sav))
			{
				FILESYSTEM.remove(sav);
				server.sendContent(sav + " removed<br>");
			}
			else
			{
				server.sendContent("no file " + sav + "<br>");
			}
		}
	}
	server.sendContent(
		"space:" + String(FILESYSTEM.usedBytes(), DEC) + " / " + String(FILESYSTEM.totalBytes(), DEC) + F(" [free ") +  String(FILESYSTEM.totalBytes() - FILESYSTEM.usedBytes(), DEC) + F("]")
	);
	server.sendContent(content_files2);
	File root = FILESYSTEM.open("/");
	File file = root.openNextFile();
	int numr = 1, num = 1;
	while (file)
	{
		String fname = file.name();
		String ans = "<tr id='row_" +  String(numr, DEC) + "'>\n"
			"<td>" + String(numr, DEC) + "</td>\n"
			"<td><input type='button' value='del' onClick='del(\"/" + fname + "\"," + String(numr, DEC) + ")'></td>\n" +
			(bmp_check(fname) ? "<td onClick='showAndroidToast(\"" + String(num, DEC) + "\")'>" + fname + "</td>\n" : "<td><a href='" + fname + "' target='_blank'>" + fname + "</a></td>\n") +
			"<td>" + file.size() + "</td>";
		if (fname.endsWith(exgif))
		{
			ans += "<td><img src='" + fname + "'></td>";
			struct gifheader gf = gif_load(fname, false);
			char mod[4] = {gf.mod[0], gf.mod[1], gf.mod[2], 0};
			ans += "<td>" + String(gf.h, DEC) + "</td>";
			ans += "<td>" + String(gf.w, DEC) + "</td>";
			ans += "<td>" + String(gf.cdp + 1, DEC) + "</td>";
			ans += "<td>" + String(mod) + "</td>";
			ans += "<td><a target='_blank' href='/buf?f=" + urlencode(fname) + "'>dec<a>  ";
			ans += "<a href='#' onclick='this.parentNode.nextElementSibling.firstChild.src = this.parentNode.nextElementSibling.firstChild.src'>rfr<a></td>";
			num++;
		}
		if (fname.endsWith(exbmp))
		{
			ans += "<td><img src='" + fname + "'></td>";
			struct bmpheader bm = bmp_load(fname, false);
			ans += "<td>" + String(bm.h, DEC) + "</td>";
			ans += "<td>" + String(bm.w, DEC) + "</td>";
			ans += "<td>" + String(bm.bits, DEC) + "</td>";
			ans += "<td>" + String(bm.bminfo) + "</td>";
			ans += "<td><a target='_blank' href='/req?buf=1&f=" + urlencode(fname) + "'>dec<a>  ";
			ans += "<a href='#' onclick='this.parentNode.nextElementSibling.firstChild.src = this.parentNode.nextElementSibling.firstChild.src'>rfr<a></td>";
			num++;
		}
		ans += "</tr>";
		server.sendContent(ans);
		file = root.openNextFile();
		numr++;
	}
	server.sendContent("</table>\n</body>\n</html>");
	server.sendContent("");
}

void handlePics()
{
	File dir = FILESYSTEM.open("/");
	int filen = 0;
	server.setContentLength(CONTENT_LENGTH_UNKNOWN);
	server.send(200, "text/plain\nAccess-Control-Allow-Origin: *", "");
	File f = dir.openNextFile();
	while (f)
	{
		String fileName = f.name();
		if (fileName.endsWith(exbmp) || fileName.endsWith(exgif) || (server.args() == 1 && server.argName(0) == "all"))
		{
			if (filen)
				server.sendContent("\n" + fileName);
			else
				server.sendContent(fileName);
			filen++;
		}
		f = dir.openNextFile();
	}
}

static const char content_conf[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<title>poihd config</title>
<meta charset='UTF-8'>
<style>
td {padding: 0 10px;}
input {width: 75px;}
</style>
<link rel="stylesheet" type="text/css" href="./tln.min.css"/>
<script type="text/javascript" src="./tln.min.js"></script>
</head>
<body>
<a href='/'>back</a><br><br>
leds <input type='number' id=ledv  min='3'><input type=button value="set" onclick="r('leds',vl('ledv'))"> <span id='leds'></span><br><br>
<input type=button value='пои' onclick="r('mode',10)">
<input type=button value='mask 1' onclick="r('mode',11)">
<input type=button value='mask 2' onclick="r('mode',12)">
<input type=button value='свой' onclick="r('mode',13)">
<span id=mode></span><br><br>
vcc <input type='number' id=vccs> <input type=button value="set" onclick="r('vcc',gid('vccs').value)">
<span id=vcc></span><br><br>
btt <span id=uptime>0000</span> <input type=button value="begin" onclick="r('uptime','1')">
<input type=button value='pins' onclick="gid('tbp').style.display=''"><br>
<table id='tbp' style='display:none;'><tr>
<caption>pins (default is 22 23 18 19 4 5 12 14)</caption>
<td>1</td><td><input type='number' id='p0' max='34'></td>
<td>2</td><td><input type='number' id='p1' max='34'></td>
<td>3</td><td><input type='number' id='p2' max='34'></td>
</tr><tr>
<td>4</td><td><input type='number' id='p3' max='34'></td>
<td>5</td><td><input type='number' id='p4' max='34'></td>
<td>6</td><td><input type='number' id='p5' max='34'></td>
</tr><tr>
<td></td><td><input type=button value='set' onclick="r('pins',vl('p0')+' '+vl('p1')+' '+vl('p2')+' '+vl('p3')+' '+vl('p4')+' '+vl('p5'))"></td>
<td colspan=4><span id='pins'></span></td>
</tr></table>
<hr><br>
<script>
	function gid(id){return document.getElementById(id);}
	function vl(id){var el=gid(id); return el.value;}
	function r(p,v){fetch('/req?'+p+'='+(v?v:'1')).then((response) => {return response.text();}).then((data) => {gid(p).innerHTML = data;});}
	function load(v){for(var key in v){var el = gid(key); if (el.tagName.toLowerCase()=='span') el.innerHTML = v[key]; else el.value = v[key];}}
	function llen(a,r,n,t){for(var b=Math.abs(n-a),e=Math.abs(t-r),f=a<n?1:-1,h=r<t?1:-1,l=b-e,o=1;a!=n||r!=t;){o++;var s=2*l;s>-e&&(l-=e,a+=f),s<b&&(l+=b,r+=h)}return o}
	
	function draw(){
	var cn = gid('can');
	var sw = gid('sw'), sw2 = gid('sw2');
	var w = gid('w').value;
	var h = gid('h').value;
	var sc = 2;
	cn.width = w * sc;
	cn.height = h * sc;
	var ctx = cn.getContext('2d');
	ctx.fillStyle = 'rgb(0,0,0)';
	ctx.fillRect(0, 0, cn.width, cn.height)
	var tv = gid('f1').value;
	var rows = tv.split("\n");
	ctx.lineWidth = sc;
	var ers = [], cl1 = '#06B', cl2 = '#e22';
	var lens = [0,0,0], li = 0;
	for (var i = 0; i < rows.length; i++)
	{
		if (i == sw.value){ cl1 = '#f80'; cl2 = '#0f8'; li = 1;}
		if (sw2.value > 0 && i == sw2.value){ cl1 = '#06B'; cl2 = '#e22'; li = 2;}
		var trg = rows[i].match(/^\s*(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s*$/);
		if (trg == null || trg.length != 5)
		{
			ers.push(i + 1);
		} else {
			ctx.strokeStyle = i % 2 == 0 ? cl1 : cl2;
			var x1 = parseInt(trg[1]), y1 = parseInt(trg[2]), x2 = parseInt(trg[3]), y2 = parseInt(trg[4]);
			ctx.beginPath();
			ctx.moveTo(x1 * sc + 1, y1 * sc);
			ctx.lineTo(x2 * sc + 1, y2 * sc);
			ctx.moveTo((w - x1) * sc - 1, y1 * sc);
			ctx.lineTo((w - x2) * sc - 1, y2 * sc);
			ctx.stroke();
			lens[li] += llen(x1, y1, x2, y2);
		}
	}
	gid('ers').innerHTML = ers.length ? 'ошибка в строках ' + ers.join(', ') : 'все ок ' + lens.join(' ');
}
</script>
<form method='post'><table>
<tr><td>w</td><td><input type=number name=w id=w></td><td>h</td><td><input type=number name=h id=h></td></tr>
<tr>
<td>len</td><td><input type=number name=cl id=cl></td>
<td>sw1</td><td><input type=number name=sw id=sw></td>
<td>sw2</td><td><input type=number name=sw2 id=sw2></td>
</tr>
<tr><td colspan=4><div style='width:300px;height:400px;'><textarea name=f1 id=f1 rows=25>
</textarea></div></td>
<td colspan=2><canvas width=50 height=50 style='border: 1px solid black' id=can></canvas><br>
<input type=button id=d value='проверить' onClick='draw()'><br><p id=ers></p></td></tr>
<tr><td colspan=4><input type=submit value='сохранить'></td></tr>
</table></form>
)=====";

void handleConfig()
{
	server.setContentLength(CONTENT_LENGTH_UNKNOWN);
	server.send(200, texthtml, content_conf);
	server.sendContent(
		"<script>"
		"load({ledv:'" + get_answ("leds", "1") + "',mode:'" + get_answ("mode", "1") +
			"', vccs:'" + String(get_vcc() , DEC) + "', uptime:'" + get_answ("uptime", "0") +
			"', p0:'" + String(conf.pins[0]) + "', p1:'" + String(conf.pins[1]) + "', p2:'" + String(conf.pins[2]) +
			"', p3:'" + String(conf.pins[3]) + "', p4:'" + String(conf.pins[4]) + "', p5:'" + String(conf.pins[5]) +
		"'});\n"
		"</script>\n"
	);
	if (server.args() > 0 && server.argName(0) == "w")
	{
		int sti = 0, stv = 0, stq = 0;
		String stcut;
		char sbuf[30];
		int aq, a1, a2, a3, a4;
		while((sti = server.arg(5).indexOf('\n', sti + 1)) != -1)
		{
			stcut = server.arg(5).substring(stv, sti);
			stcut.replace("\r", "");
			stcut.replace("\n", "");
			stcut.toCharArray(sbuf, 30);
			aq = sscanf(sbuf, "%d%d%d%d", &a1, &a2, &a3, &a4);
			Serial.printf("%04d-%04d %d ", stv, sti, aq);
			Serial.println(stcut);
			stv = sti;
			if(aq == 4) stq++;
		}
		if (server.arg(5).length() - stv > 4)
		{
			stcut = server.arg(5).substring(stv);
			stcut.replace("\n", "");
			stcut.replace("\r", "");
			stcut.toCharArray(sbuf, 30);
			aq = sscanf(sbuf, "%d%d%d%d", &a1, &a2, &a3, &a4);
			Serial.printf("%04d-%04d ", stv, server.arg(5).length());
			Serial.println(stcut);
			if(aq == 4) stq++;
		}
		if (server.arg(2).toInt() == stq)
		{
			File c = FILESYSTEM.open("/costume.txt", "w");
			c.print("cswd=" + server.arg(0) + "\n");
			c.print("csht=" + server.arg(1) + "\n");
			c.print("coln=" + server.arg(2) + "\n");
			c.print("cos1=" + server.arg(3) + "\n");
			c.print("cos2=" + server.arg(4) + "\n");
			c.print(server.arg(5));
			c.close();
			server.sendContent("все ок, сохранено " + String(stq, DEC) + " строк");
		}
		else
		{
			server.sendContent("найдены ошибки, проверьте список");
		}
		String buf4 = server.arg(5);
		buf4.replace("\r", "");
		buf4.replace("\n", "\\n");
		server.sendContent(
			"<script>\n"
			"load({w:'" + server.arg(0) + "',h:'" +  server.arg(1) + "',cl:'" + server.arg(2) + "',sw:'" + server.arg(3) + "',sw2:'" + server.arg(4) + "',f1:'" + buf4 + "'});\n"
			"</script>\n"
		);
	}
	else
	{
		server.sendContent(bmp_conf());
	}
	server.sendContent("<script>TLN.append_line_numbers('f1');</script>");
	if (FILESYSTEM.exists("/config.txt"))
	{
		File cnffile = FILESYSTEM.open("/config.txt", "r");
		server.sendContent("<hr><p style='font-family:monospace'>");
		while (cnffile.available())
		{
			String confs = cnffile.readStringUntil('\n');
			server.sendContent(confs + "<br>");
		}
		cnffile.close();
		server.sendContent("</p>");
	}
	server.sendContent(
		"</body>\n</html>"
	);
}

void handleBuf()
{
	server.setContentLength(CONTENT_LENGTH_UNKNOWN);
	server.send(200, texthtml, "<html>\n"
		"<head>\n<style>\n"
		"td {width:5px;height:5px;}\n"
		"body {font-family: monospace;}\n"
		"</style>\n</head>\n"
		"<body>\n"
	);
	if (server.args() == 1)
	{
		String fname = urldecode(server.arg(0));
		if (fname.endsWith(exgif))
		{
			gifheader gf = gif_load(fname, true);
			server.sendContent("w " + String(bmpw) + " h " + String(bmph) + " f " + String(gf.fsz) + "<br>\n");
		}
		if (fname.endsWith(exbmp))
		{
			struct bmpheader bm = bmp_load(fname, true);
			server.sendContent("w " + String(bmpw) + " h " + String(bmph) + " f " + String(bm.size) + "<br>\n");
		}
	}
	server.sendContent(
	"<canvas id=canvas style='border:1px solid black'></canvas>\n"
	"<script>\nvar w = " + String(bmpw, DEC) + ", h = " + String(bmph, DEC) + ";\n"
		"var canvas = document.getElementById('canvas');\n"
		"var ctx = canvas.getContext('2d');\n"
		"canvas.width = w;\n"
		"canvas.height = h;\n"
	"</script>\n");
	
	server.sendContent("<script>\nvar img = [\n");
	int px = 0;
	char colbuf[1024];
	int idxbuf = 0;
	for (int i = 0; i < bmph; i++)
	{
		for (int j = 0; j < bmpw; j++)
		{
			sprintf(colbuf + idxbuf,"'%02X%02X%02X',", *(p + px * 3 + 2),*(p + px * 3 + 1),*(p + px * 3));
			px++;
			idxbuf += 9;
			if(idxbuf >= 950)
			{
				server.sendContent(colbuf);
				memset(colbuf, 0, 1024);
				idxbuf = 0;
			}
		}
		sprintf(colbuf + idxbuf, "\n");
		idxbuf++;
	}
	if(idxbuf != 0)
	{
		server.sendContent(colbuf);
		memset(colbuf, 0, 1024);
		idxbuf = 0;
	}
	server.sendContent("];\n"
	"for (var i = 0; i < h; i++)\n"
	"\tfor (var j = 0; j < w; j++){\n"
	"\t\tctx.fillStyle = '#' + img[i * w + j];\n"
	"\t\tctx.fillRect(j, i, 1, 1);\n"
	"\t}\n"
	"</script>"
	"</body></html>");
}

void handleReq()
{
	if (server.args() == 3 && server.argName(0) == "ssid" && server.argName(1) == "p" && server.argName(2) == "i")
	{
		int idx = server.arg(2).toInt();
		if (idx == 1)
		{
			ssid[1] = server.arg(0);
			pass[1] = server.arg(1);
			json_save();
			if(server.arg(0) == "spiffs")
				server.send(200, textplain, "ssid 1 reset ok");
			else
				server.send(200, textplain, "Saved 1 ssid:" + ssid[1] + " pass: <span title='" + pass[1] + "'>*****</span>");
		}
		else if (idx == 2)
		{
			ssid[2] = server.arg(0);
			pass[2] = server.arg(1);
			json_save();
			if(server.arg(0) == "spiffs")
				server.send(200, textplain, "ssid 2 reset ok");
			else
				server.send(200, textplain, "Saved 2 ssid:" + ssid[2] + " pass: <span title='" + pass[2] + "'>*****</span>");
		} else {
			server.send(200, textplain, "не выбрана точка 1 или 2");
		}
		return;
	}
	String ans = get_answ(server.argName(0), server.arg(0));
	server.send(200, textplain, ans);
	
	if (ans == F("restart"))
	{
		led_clear();
		delay(500);
		ESP.restart();
	}
}

static const char content_prog[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<title>poihd prog</title><meta charset='UTF-8'>
<script src='prog.js'></script>
<style>img,input {margin-right: 5px;}</style>
</head>
<body>
<a href='/'>main</a>   
<a href='/files'>files</a>   
<a href='/prog'>prog</a>
<br><br>

mp3 <input type='file' onchange='ws.loadBlob(this.files[0]);'>
<div id='wf'></div>
<input type='button' value='play' onClick='ws.play(0)'><input type='button' value='pause' onClick='ws.playPause()'>
zoom <input type='button' value='200x' onClick='ws.zoom(200);'><input type='button' value='1x' onClick='ws.zoom(1)'>
prog <input type='button' value='load' onClick='lp()'><input type='button' value='generate' onClick='genp()'>
pics <input type='button' value='clear' onClick='if(confirm("del all?")) ws.clearRegions()'><label><input type='checkbox' id='delrc'>del by click</label>
<div style='width:100%;overflow:scroll;'><table id='pcs'></table></div><br>

<select id='progs' onchange='loadf(this.value)'></select>
<br>
<form method='POST'>
name <input name='progn' id='progn' maxlength=25 onkeyup='this.value = this.value.replace(/[^A-Za-z0-9_.]/g, "")'><br>
<textarea name='prog' id='prog' rows=20 cols=30></textarea><br>
<input type='button' value='save' onclick='savep()'>
</form>

<script>
function loads(v){
	var a = document.getElementById('progs');
	for(var i = 0; i < a.length; i++) if(a[i].value == v) return;
	a.innerHTML += '<option>' + v + '</option>';
}
async function loadf(v){
	if (v=='0') v = document.getElementById('progs').value;
	if (!v) return;
	document.getElementById('progn').value = v;
	var a = await fetch('prog_'+v+'.txt');
	var b = await a.text();
	document.getElementById('prog').value = b;
	if (document.getElementById('progs').value != document.getElementById('progn').value)
		document.getElementById('progs').value = document.getElementById('progn').value;
}
fetch('/pics').then((r) => {return r.text();}).then((d) => {
	var f = d.split('\n'), rs = '';
	for (var i = 0; i < f.length; i++) rs += '<td><img src="' + f[i] + '" ondblclick="addr(this.src)"></td>';
	document.getElementById('pcs').innerHTML = '<tr>' + rs + '</tr>';
});
var ws = WaveSurfer.create({
	container: '#wf',
	plugins: [
		WaveSurfer.regions.create({})
	]
});
ws.on('region-click', function(region) {if(document.getElementById('delrc').checked) region.remove();});

var rg = 0, rgs = {}, p = document.getElementById('prog');
function lp(){
	ws.clearRegions();
	var t = p.value;
	var s = t.split('\n');
	var pr = [];
	for(var i = 0; i < s.length-1; i++){
		var r = s[i].split(' ');
		if(r[0].indexOf('stop') == 0){
			pr.push(r[0]);
		} else {
			pr.push(r[0]);
			pr.push(r[1]);
		}
	}
	var prev = 0;
	for(var i=0; i < pr.length; i+=2){
		ws.addRegion({
			id: 'reg'+rg,
			start: (prev/1000),
			end: (prev/1000)+0.5
		});
		prev += parseInt(pr[i+1]);
		var nregs = [].filter.call(document.getElementsByTagName('region'), el => el.dataset['id'] == 'reg'+rg);
		var stopImg = "url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAcAAAAgCAIAAABRmfUKAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAABYSURBVChTtYzBDcAgDAMzSJ/sv1l3IMiREZYL/XDKg3NM4sD7tJwSQF8W2iLmAvEXtL67MEGLUylQB7dST3a1Tl8Wt9IkXSMPiua7PAafKaYcqAOf/iCiA+ltOHHSEMWvAAAAAElFTkSuQmCC')";
		nregs[0].style.background = pr[i].indexOf("stop") == 0 ? stopImg : "url('"+pr[i]+"')";
		nregs[0].style.backgroundRepeat = 'repeat-x';
		rgs['reg'+rg] = pr[i];
		rg++;
	}
}
function sof(a,b) {if (a[0] === b[0]) return 0; else return (a[0] < b[0]) ? -1 : 1;}
function genp()
{
	p.value = '';
	var out = '', pics = [];
	for(var k in ws.regions.list){
		var r = ws.regions.list[k];
		pics.push([parseInt(r.start*1000), (rgs[r.id].split('/').pop())]);
	}
	pics.sort(sof);
	pics.push([parseInt(ws.getDuration()*1000), '/kek']);
	for(var i = 0; i < pics.length - 1; i++)
		if(pics[i][1].indexOf("/stop") == 0)
			out += "stop\n";
		else
			out += pics[i][1] + ' ' + (pics[i+1][0] - pics[i][0]) + "\n";
	p.value = out;
}
function ltime(){
	var t = 0;
	for(var k in ws.regions.list){
		var r = ws.regions.list[k];
		if( r.end > t ) t = r.end;
	}
	return t;
}
function addr(p){
	ws.addRegion({
		id: 'reg'+rg,
		start: ltime(),
		end: ltime()+0.5
	});
	var nregs = [].filter.call(document.getElementsByTagName('region'), el => el.dataset['id'] == 'reg'+rg);
	nregs[0].style.background = "url('"+p+"')";
	nregs[0].style.backgroundRepeat = 'repeat-x';
	rgs['reg'+rg] = p;
	rg++;
}
async function savep(){
	var fd=new FormData(document.forms[0]);
	var p = /^[a-zA-z0-9_]+$/;
	if(!p.test(fd.get('progn'))){alert('wrong prog name');return;}
	let r=await fetch('/prog',{method:'POST',body:fd});
	let t=await r.text();
	loads(document.getElementById('progn').value);
	alert(t);
}
)=====";

void handleProg()
{
	if (server.args() > 0)
	{
		String fname = ("/prog_") + server.arg(0).substring(0,20) + extxt;
		if (server.arg(0).startsWith("prog") && server.arg(0).endsWith(extxt))
		{
			fname = server.arg(0);
			if (fname.length() > 30) fname = fname.substring(0,25) + extxt;
		}
		File prgfile = FILESYSTEM.open(fname, "w");
		prgfile.print(server.arg(1));
		prgfile.close();
		server.send(200, textplain, ("saved file: ") + fname + ("\r\n") + server.arg(1));
		return;
	}
	server.setContentLength(CONTENT_LENGTH_UNKNOWN);
	server.send(200, texthtml, content_prog);
	File root = FILESYSTEM.open("/");
	File file = root.openNextFile();
	while (file)
	{
		String f = file.name();
		if (f.startsWith("prog") && f.endsWith(extxt))
		{
			server.sendContent(("loads('") + f.substring(5, f.length() - 4) + ("');\n"));
		}
		file = root.openNextFile();
	}
	server.sendContent(("loadf('") + (server.args() > 0 ? server.arg(0) : "0") + ("');\n"));
	server.sendContent(("</script>\n"));
	if (server.args() > 0) server.sendContent(("saved ") + String(server.arg(1).length(), DEC) + (" bytes<br>"));
	server.sendContent(("</body>\n</html>\n"));
}

bool handleFileRead(String path)
{
	path = urldecode(path);
	if (FILESYSTEM.exists(path))
	{
		File file = FILESYSTEM.open(path, "r");
		server.streamFile(file, "application/octet-stream");
		file.close();
		return true;
	}
	return false;
}

void handleNotFound()
{
	if (!handleFileRead(server.uri()))
	{
		server.send(404, texthtml, "<a href='/'>Not found</a>");
	}
}

File fsUploadFile;

void handleFileUpload()
{
	HTTPUpload & upload = server.upload();
	static bool odd = false;
	if (upload.status == UPLOAD_FILE_START)
	{
		String filename = upload.filename;
		int dot = filename.lastIndexOf(".");
		String name =  filename.substring(0, dot);
		String ext = filename.substring(dot);
		ext.toLowerCase();
		filename = name.substring(0,25) + ext;
		filename.replace("&","_");
		if (!filename.startsWith("/"))
		{
			filename = "/" + filename;
		}
		fsUploadFile = FILESYSTEM.open(filename, "w");
		filename = String();
		//led_clear();
		//led_show();
	}
	else if (upload.status == UPLOAD_FILE_WRITE)
	{
		if (fsUploadFile)
		{
			fsUploadFile.write(upload.buf, upload.currentSize);
			odd = !odd;
			//led_setpx(1, odd ? blue : black);
			//led_setpx(2, odd ? black: blue);
			//led_show();
		}
	}
	else if (upload.status == UPLOAD_FILE_END)
	{
		if (fsUploadFile)
		{
			fsUploadFile.close();
			//led_setpx(1, green);
			//led_setpx(2, black);
			//led_show();
			state.calcmax = true;
			server.sendHeader("Access-Control-Allow-Origin", "*");
			server.send(200, texthtml, upload.filename + " ok<script>setTimeout(function(){location.href='/files';},1000);</script>");
			Serial.println("uploaded " + upload.filename);
		}
	}
}

void handleFileDelete()
{
	if (server.args() == 0)
	{
		return server.send(500, textplain, "BAD ARGS");
	}
	String path = server.arg(0);
	Serial.print("handleFileDelete: ");
	Serial.println(path);
	if (!FILESYSTEM.exists(path)) {
		return server.send(404, textplain, "FileNotFound");
	}
	else
	{
		FILESYSTEM.remove(path);
		state.calcmax = true;
		server.send(200, textplain, "deleted " + path);
	}
}

void http_begin()
{
	server.on("/", handleRoot);
	server.on("/req", handleReq);
	server.on("/files", handleFiles);
	server.on("/filesap", handleFiles);
	server.on(F("/prog"), handleProg);
	server.on("/pics", handlePics);
	server.on("/config", handleConfig);
	server.on("/buf", handleBuf);
	server.on("/load", HTTP_POST, []() {
		server.send(200, texthtml, " ");
	}, handleFileUpload);
	server.on("/del", handleFileDelete);
	server.on("/tln.min.css", []() {
		server.setContentLength(tln_min_css_gz_len);
		server.sendHeader("Content-Encoding", "gzip");
		server.send(200, "text/css", "");
		server.sendContent(tln_min_css_gz, tln_min_css_gz_len);
	});
	server.on("/tln.min.js", []() {
		server.setContentLength(tln_min_js_gz_len);
		server.sendHeader("Content-Encoding", "gzip");
		server.send(200, "application/javascript", "");
		server.sendContent(tln_min_js_gz, tln_min_js_gz_len);
	});
	server.on("/prog.js", []() {
		server.setContentLength(prog_js_gz_len);
		server.sendHeader("Content-Encoding", "gzip");
		server.send(200, "application/javascript", "");
		server.sendContent(prog_js_gz, prog_js_gz_len);
	});
	httpUpdater.setup(&server);
	server.onNotFound(handleNotFound);
	server.begin();
	Serial.println("HTTP server started");
	ftpSrv.begin("Welcome");
	Serial.println("FTP server started");
}

void http_poll()
{
	server.handleClient();
}

void ftp_poll()
{
	ftpSrv.handleFTP();
}
