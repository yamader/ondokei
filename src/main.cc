#include <Arduino.h>
#include <DHT.h>
#include <HTTPClient.h>
#include <WiFi.h>

constexpr auto dht11_pin = 9;
constexpr auto ssid = "hogehoge";
constexpr auto pass = "hogehogefugafuga";
constexpr auto endpoint = "https://example.net/endpoint";

DHT dht{dht11_pin, DHT11};

auto json(float t, float h, float hi) -> String {
  return "{\"t\":" + String(t) + ",\"h\":" + String(h) +
         ",\"hi\":" + String(hi) + "}";
}

auto setup() -> void {
  Serial.begin(9600);
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
}

auto loop() -> void {
  auto t = dht.readTemperature();
  auto h = dht.readHumidity();
  auto hi = dht.computeHeatIndex(t, h, false);
  auto s = json(t, h, hi);

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
