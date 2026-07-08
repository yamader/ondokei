#include <Arduino.h>
#include <DHT.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <time.h>

constexpr auto baud = 9600;
constexpr auto dht11_pin = 9;
constexpr auto ssid = "hogehoge";
constexpr auto pass = "hogehogefugafuga";
constexpr auto endpoint = "https://example.net/endpoint";
constexpr auto ntp_server_1 = "ntp1.example.net";
constexpr auto ntp_server_2 = "ntp2.example.net";
constexpr auto ntp_server_3 = "ntp3.example.net";
constexpr auto gmt_offset = 3600 * 9;
constexpr auto daylight_offset = 0;
constexpr auto ntp_sync_period = 3600;

DHT dht{dht11_pin, DHT11};

auto json(float t, float h, float hi, time_t ts) -> String {
  return "{\"t\":" + String(t) + ",\"h\":" + String(h) +
         ",\"hi\":" + String(hi) + ",\"ts\":" + String(ts) + "}";
}

auto sync_time() -> void {
  configTime(gmt_offset, daylight_offset, ntp_server_1, ntp_server_2, ntp_server_3);
  Serial.print("syncing time: ");
  Serial.println(String{ntp_server_1} + ", " + String{ntp_server_2} + ", " + String{ntp_server_3});
  tm info;
  while (!getLocalTime(&info)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("synced");
}

auto setup() -> void {
  Serial.begin(baud);
  Serial.println("");

  dht.begin();

  WiFi.begin(ssid, pass);
  Serial.print("connecting to: ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("connected");

  Serial.print("IP addr: ");
  Serial.println(WiFi.localIP());

  sync_time();
}

auto loop() -> void {
  static auto old_ts = time(nullptr);
  auto ts = time(nullptr);
  if (ts - old_ts >= ntp_sync_period) {
    sync_time();
    old_ts = ts = time(nullptr);
  }

  auto t = dht.readTemperature();
  auto h = dht.readHumidity();
  auto hi = dht.computeHeatIndex(t, h, false);
  auto s = json(t, h, hi, ts);

  Serial.println(s);

  {
    HTTPClient http;

    http.begin(endpoint);
    http.addHeader("Content-Type", "application/json");
    http.POST(s);
    http.end();
  }

  delay(10000);
}
