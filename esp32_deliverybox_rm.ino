#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <HTTPClient.h>

const char* ssid = "";
const char* password = "";
const char* line_notify_token = "";
const char* line_notify_api = "https://notify-api.line.me/api/notify";


const int GPIOPin = 5;
const int send_time = 300000;

void send_message(String message) {
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("checked WiFi");
    HTTPClient http;

    http.begin(line_notify_api);
    http.addHeader("Content-type", "application/x-www-form-urlencoded");
    String header = "Bearer ";
    header += line_notify_token;
    Serial.println(header);
    http.addHeader("Authorization", header);
    String message_data = "message=";
    message_data += message;
    int httpRes = http.POST(message_data);
    if(httpRes <0) {
      Serial.println("An error occured!");
    } else {
      Serial.print("HTTP Response:");
      Serial.println(httpRes);
      Serial.println("200:Success, 400:Bad Request, 401:Invailed token");
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}

void setup() {
  //OTAetc_setup
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  // ArduinoOTA.setHostname("myesp32");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //main_setup
  pinMode(GPIOPin, INPUT_PULLUP);
}

void loop() {
  ArduinoOTA.handle();
  Serial.println("Waiting switch");
  boolean sended_message = false;
  while (digitalRead(GPIOPin) == HIGH) {}
  Serial.println("Box open");
  unsigned long time_start = millis();
  unsigned long time_long = 0;
  while (digitalRead(GPIOPin) == LOW) {
    delay(1000);
    time_long = millis() - time_start;
    if(time_long > send_time && sended_message == false) {
      //send_message over_time
      Serial.println("Over time!");
      String message_data = "\nBox is open(over10min)";
      send_message(message_data);
      sended_message = true;
    }
  }
  //send_message close_box
  Serial.println("Box closed");
  String message_data = "Box closed";
  send_message(message_data);
}
