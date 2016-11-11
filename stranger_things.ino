#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "FastLED.h"

// WIFI settings
#define WIFI_SSID "STRANGER WIFI"

// Network settings
#define DNS_PORT 53
#define WEB_PORT 80

IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(WEB_PORT );

#define PAGE \
"<!DOCTYPE html>"\
"<html>"\
"<head>"\
  "<title>Stranger Lights</title>"\
  "<meta name='viewport' content='initial-scale=1, maximum-scale=1'>"\
"</head>"\
"<body style='text-align: center;'>"\
  "<h3>STRANGER LIGHTS</h3>"\
  "<form>"\
    "<p>Enter your message below!</p>"\
    "<input type='text' name='message'><br>"\
    "<input type='submit' value='Send via the Upside Down'>"\
  "</form>"\
"</body>"\
"</html>"


// the milliseconds to give each letter
#define MILLIS_PER_LETTER 1000

// number of LEDs in the strip
#define NUM_LEDS 50

// the data pin the green wire from the LEDs are connected to
#define DATA_PIN 4

// an array to keep track of the LEDs
CRGB leds[NUM_LEDS];

// the message we will display
String message;

// the time we received the message
unsigned long received;

// the default millis per letter
//int millis_per_letter;

// we'll use all 26 letters of the alphabet
#define NUM_LETTERS 26

// the LED number (start counting from 0) that we light up to show our message
const int LETTER_LEDS[NUM_LETTERS] = {
 /*A*/  1
,/*B*/  2
,/*C*/  3
,/*D*/  4
,/*E*/  5
,/*F*/  6
,/*G*/  7
,/*H*/  8
,/*I*/  9
,/*J*/  10
,/*K*/  11
,/*L*/  12
,/*M*/  13
,/*N*/  14
,/*O*/  15
,/*P*/  16
,/*Q*/  17
,/*R*/  18
,/*S*/  19
,/*T*/  20
,/*U*/  21
,/*V*/  22
,/*W*/  23
,/*X*/  24
,/*Y*/  25
,/*Z*/  26
};

// how many colors to cycle through for the lights
#define NUM_COLORS 5

void setup() {
  // send print statements at 9600 baud
  Serial.begin(9600);

  // initialize the LEDS
  FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);

  // set them all to be off
  fill_solid(leds, NUM_LEDS, CRGB(0, 0, 0));
  FastLED.show();

  // create the wifi network
  Serial.print("Creating the Network");
  Serial.println(WIFI_AP);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(WIFI_SSID);

  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", apIP);

  // replay to all requests with same HTML
  webServer.onNotFound([]() {
    Serial.println(webServer.uri());
    Serial.print("message: ");
    String data = webServer.arg("message");
    if(data.length()){
      webServer.send(200, "text/html", PAGE);
      data.toLowerCase();
      Serial.println(data);
    
      // remember the message and the time it came in
      message = data;
      received = millis();
    }
    webServer.send(200, "text/html", PAGE);
  });
  webServer.begin();

  // this message will show until it is overwritten
  message = "abcdefghijklmnopqrstuvwxyz";
  received = millis();
}


void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();

  // how many milliseconds have elapsed since the last message came in
  unsigned long elapsed = millis() - received;

  // assuming MILLIS_PER_LETTER, what letter (index) ofthe message should we be on?
  int index = (elapsed/MILLIS_PER_LETTER)%message.length();

  // get the character letter we should print
  char letter = message.charAt(index);

  // if the character is between 'a' and 'z' (no numbers, spaces, or punctuations)
  if(letter >= 'a' && letter <= 'z'){
    // how bright to make this LED from 0 to 1, this is what makes them fade in and out
    // it calculates what percent we are completed with the letter, and makes it fade in from 0-50% and fade out from 50-100%
    // the formula can be visualized here: https://www.desmos.com/calculator/5qk8imeny4
    float brightness = 1-abs((2*(elapsed%MILLIS_PER_LETTER)/((float)MILLIS_PER_LETTER))-1);
    uint8_t value = 255 * brightness;
    
    // get the LED number the letter should be in, assuming our array starts at 'a' and ends at 'z'
    int letter_index = letter-'a';
    int led = LETTER_LEDS[letter_index];

    // get a rotation of colors, so that every NUM_COLORS lights, it loops
    // e.g. white, magenta, cyan, blue, yellow
    uint8_t hue = letter_index%NUM_COLORS;
    if (hue == 0) {
      leds[led] = CRGB(255, 220, 90);
    }
    else if (hue == 1) {
      leds[led] = CRGB(255, 0, 170);
    }
    else if (hue == 2) {
      leds[led] = CRGB(0, 255, 255);
    }
    else if (hue == 3) {
      leds[led] = CRGB(30, 15, 255);
    }
    else if (hue == 4) {
      leds[led] = CRGB(255, 100, 0);
    }
    
    FastLED.show();
    // set it to black so we don't have to remember the last LED we turned on
    leds[led] = CRGB::Black;
    
    Serial.print(letter);
    Serial.print("\t!");
    Serial.print(led);
    Serial.print("\t=");
    Serial.print(brightness);
    Serial.print("\t@");
    Serial.print(elapsed);
     Serial.print("\th");
    Serial.print(hue);
    Serial.println();
  } else {
    // if the letter wasn't a-z then, we just turn off all the leds
    FastLED.show();
  }
}
