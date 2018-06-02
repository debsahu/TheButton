#include <ESP8266WiFi.h>
#include <HARestAPI.h>         //https://github.com/debsahu/HARestAPI
//#include "secret.h";

uint8_t holdPin = 0;  // defines GPIO 0 as the hold pin (will hold CH_PD high untill we power down).
uint8_t pirPin = 12;  // defines GPIO 12 as the PIR read pin (reads the state of the PIR output).
uint8_t pir = 1;      // sets the PIR record (pir) to 1 (it must have been we woke up).
uint8_t ledPin = 4;   // GPIO4 HIGH = ON, LOW = OFF

//Comment/Uncomment if HA is using SSL
WiFiClientSecure sclient;
HARestAPI ha(sclient);

//Comment/Uncomment if HA is not using SSL
//WiFiClient client;
//HARestAPI ha(client);

#ifndef SECRET
const char* ssid = "WiFi SSID";
const char* password = "WiFi Password";
const char* ha_ip = "192.168.0.xxx";
uint16_t ha_port = 8123; // Could be 443 is using SSL
const char* ha_pwd = "HA_PASSWORD";
String fingerprint = "35 85 74 EF 67 35 A7 CE 40 69 50 F3 C0 F6 80 CF 80 3B 2E 19";
#endif

void wifi_connect() {
  // start static IP config
  IPAddress ip(192, 168, 1, 161);     // where xx is the desired IP Address
  IPAddress gateway(192, 168, 1, 1);  // set gateway to match your network
  Serial.print(F("Setting static ip to : "));
  Serial.println(ip);
  IPAddress subnet(255, 255, 255, 0); // set subnet mask to match your network
  IPAddress dns(192, 168, 1, 1);      // set dns of your network
  WiFi.config(ip, gateway, subnet, dns);
  // end of static IP config
  Serial.print(F("connecting to "));
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());
}

void setup() {
  pinMode(holdPin, OUTPUT);     // sets GPIO 0 to output
  digitalWrite(holdPin, HIGH);  // sets GPIO 0 to high (this holds CH_PD high even if the PIR output goes low)
  pinMode(pirPin, INPUT);       // sets GPIO 12 to an input so we can read the PIR output state
  //pinMode(LED_BUILTIN, OUTPUT); // BuiltIn LED
  pinMode(ledPin, OUTPUT);      // Key LED ON
  digitalWrite(ledPin, HIGH);   // LED ON
  
  Serial.begin(115200);
  Serial.println();

  wifi_connect();
  
  ha.setHAServer(ha_ip, ha_port);
  ha.setHAPassword(ha_pwd);
  ha.setFingerPrint(fingerprint); // Only used if HA is running SSL

  ha.setURL("/api/services/light/toggle");
  ha.sendHAComponent("light.fairy_light");
  
  WiFi.disconnect();
}

void loop() {
  if((pir) == 0){  // if (pir) == 0, which its not first time through as we set it to "1" above
    Serial.println();
    Serial.println(F("OFF"));
    //digitalWrite(LED_BUILTIN, HIGH); // BuiltIn LED OFF
    digitalWrite(ledPin, LOW);       // LED OFF
    digitalWrite(holdPin, LOW);  // set GPIO 0 low this takes CH_PD & powers down the ESP
  }else{                 // if (pir) == 0 is not true
    //digitalWrite(LED_BUILTIN, LOW);  // BuiltIn LED ON
    digitalWrite(ledPin, HIGH);      // LED ON
    Serial.println(F("ON"));
    while(digitalRead(pirPin) == 1){  // read GPIO 12, while GPIO 12 = 1 is true, wait (delay below) & read again, when GPIO 2 = 1 is false skip delay & move on out of "while loop"
      delay(50);
      Serial.print(".");
    }
    pir = 0;             // set the value of (pir) to 0
    delay(20);           // wait 20 msec
  }
}
