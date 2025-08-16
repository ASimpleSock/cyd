/*******************************************************************
    A sketch to fetch a image from the internet and store it in memory

    Useful for smaller images

    Parts:
    ESP32 D1 Mini stlye Dev board* - http://s.click.aliexpress.com/e/C6ds4my
    (or any ESP32 board)

    *  = Affilate

    If you find what I do useful and would like to support me,
    please consider becoming a sponsor on Github
    https://github.com/sponsors/witnessmenow/


    Written by Brian Lough
    YouTube: https://www.youtube.com/brianlough
    Tindie: https://www.tindie.com/stores/brianlough/
    Twitter: https://twitter.com/witnessmenow
 *******************************************************************/

// ----------------------------
// Standard Libraries
// ----------------------------

#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

// Wifi network station credentials
#define WIFI_SSID "Joy500"
#define WIFI_PASSWORD "iisaharshii"

WiFiClient client;

#include <JPEGDEC.h>
// Library for decoding Jpegs from the API responses
//
// Can be installed from the library manager (Search for "JPEGDEC")
// https://github.com/bitbank2/JPEGDEC

#include <TFT_eSPI.h>
// A library for writing to the LCD
// NOTE: This library requires you to add config file to it
// steps are described here: https://github.com/witnessmenow/Spotify-Diy-Thing#display-config

// Can be installed from the library manager (Search for "TFT_eSPI")
// https://github.com/Bodmer/TFT_eSPI

TFT_eSPI tft = TFT_eSPI();
JPEGDEC jpeg;

// This next function will be called during decoding of the jpeg file to
// render each block to the Matrix.  If you use a different display
// you will need to adapt this function to suit.
int JPEGDraw(JPEGDRAW *pDraw)
{
  // Stop further decoding as image is running off bottom of screen
  if (pDraw->y >= tft.height())
    return 0;

  tft.pushImage(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pDraw->pPixels);
  return 1;
}

bool getImage(const char *imageUrl)
{
    HTTPClient http;
    http.begin(imageUrl);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK)
    {
        WiFiClient *stream = http.getStreamPtr();
        int len = http.getSize(); // May be -1 if unknown length
        if (len <= 0)
        {
            Serial.println("Invalid content length");
            http.end();
            return false;
        }

        // Allocate buffer for JPEG
        uint8_t *imageFile = (uint8_t *)malloc(len);
        if (!imageFile)
        {
            Serial.println("Malloc failed!");
            http.end();
            return false;
        }

        int index = 0;
        while (http.connected() && (index < len))
        {
            size_t available = stream->available();
            if (available)
            {
                int c = stream->readBytes(imageFile + index, available);
                index += c;
            }
            delay(1); // Yield to keep WiFi alive
        }

        Serial.printf("Got Image: %d bytes\n", index);

        // Decode using JPEGDEC
        if (jpeg.openRAM(imageFile, index, JPEGDraw) == 1)
        {
            jpeg.setPixelType(1);
            jpeg.decode(0, 0, 0);
            jpeg.close();
        }
        else
        {
            Serial.println("JPEG open failed");
        }

        free(imageFile);
        http.end();
        return true;
    }
    else
    {
        Serial.printf("HTTP GET failed, code: %d\n", httpCode);
        http.end();
        return false;
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println();

    // attempt to connect to Wifi network:
    Serial.print("Connecting to Wifi SSID ");
    Serial.print(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);



    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }
    Serial.print("\nWiFi connected. IP address: ");
    Serial.println(WiFi.localIP());

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.fillScreen(TFT_BLACK);
    tft.writecommand(ILI9341_GAMMASET); //Gamma curve selected
    tft.writedata(2);
    delay(120);
    tft.writecommand(ILI9341_GAMMASET); //Gamma curve selected
    tft.writedata(1);
}

void loop()
{
    int returnStatus = getImage("http://cyd.asimplesock.com/live.jpg");
    Serial.print("returnStatus: ");
    Serial.println(returnStatus);
    delay(1000);
}   