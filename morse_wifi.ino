//192.168.4.1
#include <string.h>
#define Touch T0
#define TOUCH_THRESHOLD 25
#define BUZZ 17
#include <Adafruit_SSD1306.h>
#include <WiFi.h> // header we need to include for WiFi functions
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>
#define NEOP 25
Adafruit_NeoPixel pixel = Adafruit_NeoPixel(1, NEOP, NEO_GRB+NEO_KHZ800);
Adafruit_SSD1306 lcd(128, 64);

const char* ssid = "Saniya"; // !!! CHANGE THIS to your name or sth. unique
const char* pass = "Saniya161001"; // !!! NEEDS TO BE AT LEAST 8 CHARACTER LONG
WebServer server(80);  // define server object, at port 80

char str[300] = "" ; 
char words[300] = "";
char dot = '.';
char dash = '-';
char spaces = ' ';

char morse[][36] = {
{'.','-'},
{'-','.','.','.'},
{'-','.','-','.'},
{'-','.','.'},
{'.'},
{'.','.','-','.'},
{'-','-','.'},
{'.','.','.','.'},
{'.','.'},
{'.','-','-','-'},
{'-','.','-'},
{'.','-','.','.'},
{'-','-'},
{'-','.'},
{'-','-','-'},
{'.','-','-','.'},
{'-','-','.','-'},
{'.','-','.'},
{'.','.','.'},
{'-'},
{'.','.','-'},
{'.','.','.','-'},
{'.','-','-'},
{'-','.','.','-'},
{'-','.','-','-'},
{'-','-','.','.'},
{'-','-','-','-','-'},
{'.','-','-','-','-'},
{'.','.','-','-','-'},
{'.','.','.','-','-'},
{'.','.','.','.','-'},
{'.','.','.','.','.'},
{'-','.','.','.','.'},
{'-','-','.','.','.'},
{'-','-','-','.','.'},
{'-','-','-','-','.'}};

char alphaNumeric[37] = {'a', 'b' , 'c' , 'd' , 'e', 'f' , 'g' , 'h' , 'i' , 'j' , 'k' , 'l' , 'm' , 'n' , 'o' , 'p' , 'q' , 'r' , 's' , 't' , 'u' , 'v' , 'w' , 'x' , 'y' , 'z' , '0' , '1' , '2' , '3' , '4' , '5' , '6' , '7' , '8' , '9' , '\0'};
unsigned long start_time = 0, end_time = 0, start_space = 0, end_space = 0, curr = 0;
int doCalculation = 0, pressed = 0, flag = 0, doSpace = 0;

void setup() {
  Serial.flush();
  Serial.begin(9600);
  pinMode(Touch, INPUT);
  lcd.begin(SSD1306_SWITCHCAPVCC, 0x3C); 
  lcd.clearDisplay(); 
  lcd.setTextColor(WHITE); 
  lcd.setCursor(0,0);
  lcd.display();  
  pixel.begin();
  pixel.setBrightness(128);
  WiFi.mode(WIFI_AP); // start ESP in AP mode
  WiFi.softAP(ssid, pass); // configure ssid and (optionally) password 
  server.on("/", on_home);  // home callback function
  server.on("/inline", [](){
    server.send(200, "text/html", "<h1>Inline callback works too!</h1>");
  });
  server.begin();  // starts server
  ledcSetup(0, 5000, 8);
  ledcAttachPin(BUZZ, 0);
}

void on_home() {
  String output = "";
  output.concat("<h1>");
  output.concat(words);
  output.concat("</h1>");
  server.send(200, "text/html", output);
}

void BlinkPixel(int input){
  while(input < TOUCH_THRESHOLD){
    pixel.setPixelColor(0, pixel.Color(255,0,255));
    ledcWriteTone(0, 200);
    delay(10);
    pixel.show();
    input = touchRead(T0);
  }
   pixel.setPixelColor(0, pixel.Color(0,0,0));
   ledcWriteTone(0, 0);
   pixel.show();
}

char *MorseToChar(char inp[]){
  char *temp = NULL;
  for(int i = 0; i < 36; i++){
    for(int j = 0; j < strlen(morse[i]); j++){
      if(strcmp(inp , morse[i]) == 0){
        temp = &alphaNumeric[i];
        return temp;
      }
    }
  }
  temp = &alphaNumeric[36];
  return temp;
}


void loop() {
  int currTime = millis();
  // Based on state, do things 
  if (touchRead(T0) > TOUCH_THRESHOLD && pressed) {
    start_space = currTime;
    end_time = currTime;
    doCalculation = 1;
    pressed = 0;
  }
  else if ((curr = touchRead(T0)) < TOUCH_THRESHOLD && !pressed) {
    BlinkPixel(curr);
    end_space = currTime;
    start_time = currTime;
    pressed = 1;
    doSpace = 1;
  }
  
  if (doCalculation) {
    char *temp = NULL;
    int time_interval = end_time - start_time;
    if (time_interval > 0) {
      temp = (time_interval <= 800) ? &dot : &dash; 
      strncat(str , temp , 1);
    }
    Serial.println(str);
    doCalculation = 0;
  }
  else if (doSpace) {
    int time_space = end_space - start_space;
    if (time_space > 0) {
        if(time_space >= 7000){
          strncat(words , MorseToChar(str),1);
          strncat(words , &spaces,1);
          strcpy(str , "");
        }
        else if(time_space >= 3000){
          strncat(words , MorseToChar(str),1);
          strcpy(str , "");
        }
    }
    lcd.clearDisplay();
    lcd.setCursor(0,0);
    lcd.print(words);
    lcd.display();
    doSpace = 0;
  }
  server.handleClient();  // handle client requests, must call frequently
}
