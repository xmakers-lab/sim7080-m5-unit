#include <M5Stack.h>

#define TINY_GSM_MODEM_SIM7080
#include <TinyGsmClient.h>

// ===== UART =====
HardwareSerial SerialAT(2);
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

// ===== 通信設定（ここを変えれば他サービスにも対応）=====
const char APN[]    = "soracom.io";
const char HOST[]   = "harvest.soracom.io";
const int  PORT     = 80;
const char PATH[]   = "/";

// ===== 表示設定 =====
int logLine = 0;
const int LINE_H = 20;
const int START_Y = 40;

// =====================
// 表示処理
// =====================
void initDisplay() {
  M5.Lcd.clear();
  M5.Lcd.setTextSize(2);

  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println("SORACOM_HTTP");
  M5.Lcd.println("----------------");

  logLine = 0;
}

void logMsg(String msg) {
  Serial.println(msg);

  int y = START_Y + logLine * LINE_H;

  M5.Lcd.fillRect(0, y, 320, LINE_H, BLACK);
  M5.Lcd.setCursor(0, y);
  M5.Lcd.print(msg);

  logLine++;
  if (logLine > 8) logLine = 0;
}

// =====================
// ボーレート自動判定
// =====================
bool testAT() {
  String res = "";
  SerialAT.println("AT");
  delay(1000);

  while (SerialAT.available()) {
    res += (char)SerialAT.read();
  }

  return res.indexOf("OK") >= 0;
}

void initModemBaud() {

  logMsg("TRY 115200");
  SerialAT.begin(115200, SERIAL_8N1, 22, 21);
  delay(2000);

  if (testAT()) {
    logMsg("115200 OK");
    return;
  }

  logMsg("TRY 9600");
  SerialAT.begin(9600, SERIAL_8N1, 22, 21);
  delay(2000);

  if (testAT()) {
    logMsg("9600 OK");

    SerialAT.println("AT+IPR=115200");
    delay(1000);

    SerialAT.begin(115200, SERIAL_8N1, 22, 21);
    delay(2000);

    if (testAT()) {
      logMsg("SWITCH OK");
      return;
    }
  }

  logMsg("MODEM ERROR");
}

// =====================
// ネット接続
// =====================
bool connectNetwork() {

  logMsg("WAIT NET");
  if (!modem.waitForNetwork()) {
    logMsg("NET FAIL");
    return false;
  }

  logMsg("NET OK");

  logMsg("GPRS");
  if (!modem.gprsConnect(APN)) {
    logMsg("GPRS FAIL");
    return false;
  }

  logMsg("GPRS OK");

  return true;
}

// =====================
// センサ読み取り（ここを書き換えると実機対応）
// =====================
String readSensor() {

  // 仮データ（温度）
  float temp = 15.0 + (float)random(0, 150) / 10.0;

  String json = "{\"temp\":";
  json += String(temp, 1);
  json += "}";

  return json;
}

// =====================
// HTTP送信（汎用）
// =====================
bool sendHttp(String host, int port, String path, String json) {

  logMsg("CONNECT");

  if (!client.connect(host.c_str(), port)) {
    logMsg("CONNECT FAIL");
    return false;
  }

  // HTTP POST
  client.println("POST " + path + " HTTP/1.1");
  client.println("Host: " + host);
  client.println("Content-Type: application/json");
  client.print("Content-Length: ");
  client.println(json.length());
  client.println();
  client.println(json);

  logMsg("SEND");

  // レスポンス取得
  String response = "";
  long timeout = millis();

  while (client.connected() && millis() - timeout < 5000) {
    while (client.available()) {
      char c = client.read();
      Serial.write(c);
      response += c;
    }
  }

  client.stop();

  // 成功判定（HTTP 2xxを成功とみなす）
  int idx = response.indexOf("HTTP/1.1 ");
  if (idx < 0) return false;
  char c = response.charAt(idx + 9);
  return c == '2';
}

// =====================
// 時刻表示
// =====================
void showTime() {

  TinyGSMDateTimeFormat format;
  String dt = modem.getGSMDateTime(format);

  if (dt.length() >= 17) {
    String t =
      "20" + dt.substring(0,2) + "-" +
      dt.substring(3,5) + "-" +
      dt.substring(6,8) + " " +
      dt.substring(9,17);

    logMsg(t);
  } else {
    logMsg(dt);
  }
}

// =====================
// setup
// =====================
void setup() {
  M5.begin();
  Serial.begin(115200);

  initDisplay();
  logMsg("INIT");

  initModemBaud();
  modem.restart();

  if (!connectNetwork()) return;
}

// =====================
// loop
// =====================
void loop() {

  initDisplay();

  logMsg("SEND START");

  // 電波強度
  int rssi = modem.getSignalQuality();
  logMsg("RSSI: " + String(rssi));

  // センサ読み取り
  String json = readSensor();
  logMsg(json);

  // HTTP送信
  bool ok = sendHttp(HOST, PORT, PATH, json);

  if (ok) logMsg("SUCCESS");
  else    logMsg("FAILED");

  logMsg("END");

  // 時刻表示
  showTime();

  delay(300000);
}