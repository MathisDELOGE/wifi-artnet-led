#include <ArtnetWifi.h>
#include <FastLED.h>
#include "WiFi.h"
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WebConfig.h>

ArtnetWiFiReceiver artnet;
WebServer server;
WebConfig conf;

String params = "["
  "{"
  "'name':'ssid',"
  "'label':'Name des WLAN',"
  "'type':"+String(INPUTTEXT)+","
  "'default':''"
  "},"
  "{"
  "'name':'pwd',"
  "'label':'WLAN Passwort',"
  "'type':"+String(INPUTPASSWORD)+","
  "'default':''"
  "},"
  "]";

// FastLED
const int NUM_LEDS = 144;
const uint8_t PIN_LED_DATA = 4;
CRGB leds[NUM_LEDS];

const uint8_t UNIVERSE = 1;
const char *WIFI_SSID = "WifideJojo";
const char *WIFI_PASS = "Cestlewifidejojo";
IPAddress local_IP(192, 168, 0, 16);
IPAddress gateway(192, 168, 0, 254);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional

boolean initWiFi() {
    boolean connected = false;
    WiFi.mode(WIFI_STA);
    Serial.print("Connexion à ");
    Serial.print(conf.values[0]);
    if (conf.values[0] != "") {
      WiFi.begin(conf.values[0].c_str(),conf.values[1].c_str());
      uint8_t cnt = 0;
      while ((WiFi.status() != WL_CONNECTED) && (cnt<20)){
        delay(500);
        Serial.print(".");
        cnt++;
      }
      Serial.println();
      if (WiFi.status() == WL_CONNECTED) {
        Serial.print("IP-Adresse = ");
        Serial.println(WiFi.localIP());
        connected = true;
      }
    }
    if (!connected) {
          WiFi.mode(WIFI_AP);
          Serial.print("IP-Adresse = ");
          Serial.println(WiFi.localIP());
          WiFi.softAP(conf.getApName(),"",1);  
    }
    return connected;
}

void handleRoot() {
  conf.handleFormRequest(&server);
  if (server.hasArg("SAVE")) {
    uint8_t cnt = conf.getCount();
    Serial.println("*********** Configuration ************");
    for (uint8_t i = 0; i<cnt; i++) {
      Serial.print(conf.getName(i));
      Serial.print(" = ");
      Serial.println(conf.values[i]);
    }
    if (conf.getBool("switch")) Serial.printf("%s \n",
                                conf.getValue("ssid"));
  }
}

void setup()
{
  // Serial.begin(9600);
  // FastLED.addLeds<WS2812, PIN_LED_DATA, RGB>(leds, NUM_LEDS);  // GRB ordering is typical


  Serial.begin(9600);
  // Giving it a little time because the serial monitor doesn't
  // immediately attach. Want the firmware that's running to
  // appear on each upload.
  delay(2000);

  Serial.println();
  Serial.println("Running Firmware.");

  Serial.println(params);
  conf.setDescription(params);
  conf.readConfig();
  initWiFi();
  char dns[30];
  sprintf(dns,"%s.local",conf.getApName());
  if (MDNS.begin(dns)) {
    Serial.println("MDNS responder gestartet");
  }
  server.on("/",handleRoot);
  server.begin(80);

  // // Connect to Wifi.
  // Serial.println();
  // Serial.println();
  // Serial.print("Connecting to ");
  // Serial.println(WIFI_SSID);

  // // setup Ethernet/WiFi...
  // WiFi.mode(WIFI_STA);
  // if (!WiFi.config(local_IP, gateway, subnet)) {
  //   Serial.println("Static IP Failed to configure, fallback to DHCP");
  // }
  // WiFi.disconnect();
  // delay(100);

  // WiFi.begin(WIFI_SSID, WIFI_PASS);
  // while (WiFi.status() != WL_CONNECTED)
  // {
  //   // Check to see if connecting failed.
  //   // This is due to incorrect credentials
  //   if (WiFi.status() == WL_CONNECT_FAILED)
  //   {
  //     Serial.println("Failed to connect to WIFI. Please verify credentials: ");
  //     Serial.println();
  //     Serial.print("SSID: ");
  //     Serial.println(WIFI_SSID);
  //     Serial.print("Password: ");
  //     Serial.println(WIFI_PASS);
  //     Serial.println();
  //   }
  //   delay(5000);
  // }

  // Serial.println("");
  // Serial.println("WiFi connected");
  // Serial.println("IP address: ");
  // Serial.println(WiFi.localIP());

  // setup FastLED
  FastLED.addLeds<WS2812, PIN_LED_DATA>(leds, NUM_LEDS);

  artnet.begin(); // waiting for Art-Net in default port
  // artnet.begin(net, subnet); // optionally you can set net and subnet here

  // artnet.forward(1, leds, NUM_LEDS);

  // this can be achieved manually as follows
  artnet.subscribe(UNIVERSE, [](const uint8_t *data, const uint16_t size)
                   {
        // artnet data size per packet is 512 max
        // so there is max 170 pixel per packet (per universe)
        for (size_t pixel = 0; pixel < NUM_LEDS; ++pixel)
        {
          size_t idx = pixel * 3;
          leds[pixel].r = data[idx + 0];
          leds[pixel].g = data[idx + 1];
          leds[pixel].b = data[idx + 2];
        } });
}

void loop()
{
  // WebConfig
  server.handleClient();

  //Artnet
  artnet.parse();

  // FastLed
  FastLED.show();


  // // Turn the LED on, then pause
  // leds[0] = CRGB::Red;
  // Serial.println("RED");
  // FastLED.show();
  // delay(500);
  // // Now turn the LED off, then pause
  // leds[0] = CRGB::Black;
  // Serial.println("BLACK");
  // FastLED.show();
  // delay(500);
}
