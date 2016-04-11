/*

  NTP RGB Clock v1.0

  Get the time from a Network Time Protocol (NTP) time server.
  Display on a Neopixel 8 RGB leds strip.
  This code is in the public domain.

*/

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <Adafruit_NeoPixel.h>


char ssid[] = "SmartLive";  //  your network SSID (name)
char pass[] = "";           // your network password
int deuxpoints = 1;         // color of two points (Hh it's h, heures)

unsigned int localPort = 2390;      // local port to listen for UDP packets

IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "0.fr.pool.ntp.org"; //or "time.nist.gov";

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

#define PIXEL_PIN   4    // Digital IO pin connected to the NeoPixels.
#define PIXEL_COUNT 8    // Juste 8 leds <HH : MM : SS>


static unsigned long lastTick = 0;
static unsigned long lastNTP = 0;

int secondes; // seconds
int minutes;  // minutes
int heures;   // hours
int r1;       // red 1
int v1;       // green 1
int b1;       // blue 1
int r2;       // red 2
int v2;       // green 2
int b2;       // blue 2
int oo = 0;   // ticks tic tac
int GMT = 2;  // GMT+2 Paris

// Parameter 1 = number of pixels in strip,  neopixel stick has 8
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream, correct for neopixel stick
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip), correct for neopixel stick
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  strip.begin();
  strip.setBrightness(32); // Not too bright
  strip.show(); // Initialize all pixels to 'off'

  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    rainbowCycle (10);
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());


  WiFi.hostByName(ntpServerName, timeServerIP);
  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  delay(1000);

  int cb = udp.parsePacket();
  if (cb) {
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    const unsigned long seventyYears = 2208988800UL;
    unsigned long epoch = secsSince1900 - seventyYears;

    heures = ((epoch  % 86400L) / 3600) + GMT;
    minutes = ((epoch % 3600) / 60);
    secondes = epoch % 60;
  }


}

void loop()
{

  if (millis() - lastNTP >= 60000) { // query each minute

    lastNTP = millis();

    WiFi.hostByName(ntpServerName, timeServerIP);
    sendNTPpacket(timeServerIP); // send an NTP packet to a time server
    // wait to see if a reply is available
    delay(400);

    int cb = udp.parsePacket();
    if (cb) {
      udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
      unsigned long secsSince1900 = highWord << 16 | lowWord;
      const unsigned long seventyYears = 2208988800UL;
      unsigned long epoch = secsSince1900 - seventyYears;
      heures = ((epoch  % 86400L) / 3600) + GMT;
      minutes = ((epoch % 3600) / 60);
      secondes = epoch % 60;
    }

  }
  if (millis() - lastTick >= 1000) {
    lastTick = millis();
    if (oo == 1) {
      oo = 0;
    }
    else
    {
      oo = 1;
    }
    secondes++;
    disp(6, secondes); // display seconds
    vide(5, oo);       // point between min and sec
    disp(3, minutes);  // display minutes
    vide(2, oo);       // point between hour and min
    disp(0, heures);   // display hours

  }

  // Time management
  if (secondes > 59) {
    secondes = 0;
    minutes++;
  }
  if (minutes > 59) {
    minutes = 0;
    heures++;
  }
  if (heures > 23) {
    heures = 0;
    minutes = 0;
  }


}

unsigned long sendNTPpacket(IPAddress& address)
{
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}


void vide(int n, int c)
{
  if (c != 1) {
    strip.setPixelColor(n, strip.getPixelColor(deuxpoints));
    strip.show();
  }
  else
  {
    strip.setPixelColor(n, 0);        //turn pixel off
    strip.show();
  }
}

void disp(int m, int n)
{
  strip.setPixelColor(m, Wheel((( (n / 10) * 256 / strip.numPixels()) + n * 4) & 255));
  strip.setPixelColor(m + 1, Wheel((( (n % 10) * 256 / strip.numPixels()) + n * 4) & 255));
  strip.show();

}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}


