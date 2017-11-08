#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class Display
{
  Adafruit_SSD1306 *display;
public:
  Display()
  {
    display = nullptr;
  }
  ~Display()
  {
    if (display)
    {
      delete display;
      display = nullptr;
    }
  }
  void begin(int T, int address, int oledReset = 4)
  {
    if (display)
    {
      delete display;
      display = nullptr;
    }
    display = new Adafruit_SSD1306(oledReset);
    display->begin(T, address);
  }
  void print(String data, int posX = 0, int posY = 0, char fontSize = 1, char color = WHITE)
  {
    display->setTextSize(fontSize);
    display->setTextColor(color);
    display->setCursor(posX, posY);
    display->println(data);
  }
  void xPrint(String data, int posX = 0, int posY = 0, char fontSize = 1, char color = WHITE)
  {
    display->clearDisplay();
    display->setTextSize(fontSize);
    display->setTextColor(color);
    display->setCursor(posX, posY);
    display->println(data);
    display->display();
  }
  void pagePrint(String Header, String l1 = "", String l2 = "")
  {
    clearBuffer();
    print(Header, 0, 11, 2);
    print(l1, 0, 29, 2);
    flushBuffer();
  }
  
  void clearBuffer()
  {
    display->clearDisplay();
  }
  void flushBuffer()
  {
    display->display();
  }
};


String serialReader()
{
  String dt = "";
  while(Serial.available() != 0)
  {
    dt += (char)Serial.read();
    delay(5);
  }
  return dt;
}

Display *dis;
void setup() {
  Serial.begin(9600);
  dis = new Display();
  dis->begin(SSD1306_SWITCHCAPVCC, 0x3C);
}

void loop() {
  String dt = serialReader();
  if(dt != "")
    dis->pagePrint(dt);
}
