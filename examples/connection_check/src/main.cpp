#include <M5Stack.h>

HardwareSerial sim7080(2);

// 状態
bool ok_module = false;
bool ok_sim = false;
bool ok_signal = false;
bool ok_network = false;
bool ok_time = false;

// Signal値
int signalValue = -1;

// 時刻
String currentTime = "--:--:--";

// AT送信
String sendCmd(const char* cmd, int wait = 1000) {
  String res = "";

  Serial.print(">> ");
  Serial.println(cmd);

  sim7080.println(cmd);
  delay(wait);

  while (sim7080.available()) {
    char c = sim7080.read();
    Serial.write(c);
    res += c;
  }

  Serial.println("----------------------------");
  return res;
}

// CSQパース
int parseCSQ(String res) {
  int idx = res.indexOf("+CSQ:");
  if (idx < 0) return -1;

  int comma = res.indexOf(",", idx);
  if (comma < 0) return -1;

  String val = res.substring(idx + 6, comma);
  val.trim();

  return val.toInt();
}

// 表示
void drawAll() {
  M5.Lcd.clear();
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextSize(2);

  M5.Lcd.println("SIM7080 Check\n");

  M5.Lcd.printf("Module : %s\n", ok_module ? "OK" : "NG");
  M5.Lcd.printf("SIM    : %s\n", ok_sim ? "OK" : "NG");

  if (signalValue >= 0) {
    M5.Lcd.printf("Signal : %s (RSSI=%d)\n",
      ok_signal ? "OK" : "NG", signalValue);
  } else {
    M5.Lcd.printf("Signal : %s\n",
      ok_signal ? "OK" : "NG");
  }

  M5.Lcd.printf("Network: %s\n", ok_network ? "OK" : "NG");
  M5.Lcd.printf("Time   : %s\n", ok_time ? "OK" : "NG");

  M5.Lcd.printf("\nTime: %s", currentTime.c_str());
}

// ボーレート自動判定
bool checkModuleWithBaud() {
  String res;

  // まず115200で試す
  sim7080.begin(115200, SERIAL_8N1, 22, 21);
  delay(1000);

  res = sendCmd("AT");
  if (res.indexOf("OK") >= 0) {
    Serial.println("Use baud: 115200");
    return true;
  }

  // ダメなら9600
  sim7080.begin(9600, SERIAL_8N1, 22, 21);
  delay(1000);

  res = sendCmd("AT");
  if (res.indexOf("OK") >= 0) {
    Serial.println("Detected baud: 9600 -> switching to 115200");

    // 115200に変更
    sendCmd("AT+IPR=115200", 1000);

    // 再接続
    sim7080.begin(115200, SERIAL_8N1, 22, 21);
    delay(1000);

    // 再確認
    res = sendCmd("AT");
    return (res.indexOf("OK") >= 0);
  }

  return false;
}

void setup() {
  M5.begin();
  Serial.begin(115200);

  delay(5000);

  drawAll(); // 初期NG表示

  // --- Baud判定 ---
  ok_module = checkModuleWithBaud();
  drawAll();
  delay(500);

  if (!ok_module) {
    Serial.println("Module not responding!");
    return;
  }

  // --- SIM ---
  String res = sendCmd("AT+CPIN?");
  ok_sim = res.indexOf("READY") >= 0;
  drawAll();
  delay(500);

  // --- Signal ---
  res = sendCmd("AT+CSQ");
  ok_signal = res.indexOf("+CSQ") >= 0;
  signalValue = parseCSQ(res);
  drawAll();
  delay(500);

  // --- Network ---
  sendCmd("AT+CGATT=1", 5000);
  res = sendCmd("AT+CEREG?");
  ok_network = (res.indexOf("0,1") >= 0 || res.indexOf("0,5") >= 0);
  drawAll();
  delay(500);

  // --- Time ---
  sendCmd("AT+CTZU=1");
  res = sendCmd("AT+CCLK?");
  ok_time = res.indexOf("+CCLK") >= 0;

  int p = res.indexOf("\"");
  if (p >= 0) {
    currentTime = res.substring(p + 1, p + 20);
  }

  drawAll();

  Serial.println("=== Check Complete ===");
}

void loop() {
  String res = sendCmd("AT+CCLK?", 500);

  int p = res.indexOf("\"");
  if (p >= 0) {
    currentTime = res.substring(p + 1, p + 20);
  }

  drawAll();

  delay(1000);
}