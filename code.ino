#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// WiFi settings
const char* ssid = "SID";
const char* password = "PASSWORD";

// Screen settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET 16
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Buttons and LED
const int buttonReset = D3;
const int buttonNext = D4;
const int buttonPrev = D5;
const int buzzer = D6;
const int ledPin = D7;

// Webserver
ESP8266WebServer server(80);

// Message variables and password
std::vector<String> messages;
int messageIndex = -1;
const int maxMessages = 5;
const String passwordToSend = "1234";

void setup() {
  pinMode(buttonReset, INPUT_PULLUP);
  pinMode(buttonNext, INPUT_PULLUP);
  pinMode(buttonPrev, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  pinMode(ledPin, OUTPUT);

  Wire.begin(4, 5);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(5, 10);
  display.println("Waiting!");
  display.display();
  delay(1000); 
  display.startscrollleft(0x00, 0x0F);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/sendMessage", HTTP_POST, handleMessage);
  server.begin();
}

void loop() {
  server.handleClient();
  if (!digitalRead(buttonReset)) {
    resetMessages();
    delay(1000);
  }
  if (!digitalRead(buttonNext)) {
    nextMessage();
    delay(1000);
  }
  if (!digitalRead(buttonPrev)) {
    prevMessage();
    delay(1000);
  }
}

void handleRoot() {
  String html = "<h2>Please use only English characters</h2>";
  html += "<form action='/sendMessage' method='post'>";
  html += "Message (max 20 characters):  <input type='text' name='message' maxlength='20'>";
  html += " Password: <input type='password' name='password'>";
  html += "<input type='submit' value='Send'>";
  html += "</form>";
  
  html += "<footer><p style='text-align:center; font-size:small;'>";
  html += "Copyright - Anders Isaksson - SA7BNB 2024";
  html += "</p></footer>";
  server.send(200, "text/html", html);
}

void handleMessage() {
  if (server.hasArg("password") && server.arg("password") == passwordToSend) {
    if (server.hasArg("message")) {
      String newMessage = server.arg("message");
      if (newMessage.length() > 20) {
        server.send(200, "text/html", "<p>Error: Message too long!.</p><a href='/'>Back</a>");
        return;
      }
      if (messages.size() >= maxMessages) {
        messages.erase(messages.begin());
      }
      messages.push_back(newMessage);
      messageIndex = messages.size() - 1;
      displayMessage();
      alertNewMessage();
      server.send(200, "text/html", "<p>Message Sent!</p><a href='/'>Back</a>");
    } else {
      server.send(200, "text/html", "<p>Error: No Message Sent!.</p><a href='/'>Back</a>");
    }
  } else {
    server.send(403, "text/html", "<p>Wrong Password!.</p><a href='/'>Back</a>");
  }
}

void displayMessage() {
  display.clearDisplay();
  display.setTextSize(1);
  if (messageIndex >= 0 && messageIndex < messages.size()) {
    display.setCursor(5, 20);
    display.println(messages[messageIndex]);
  }
    display.display();
  delay(1);
  display.startscrollleft(0x00, 0x0F);
}

void nextMessage() {
  if (messageIndex < messages.size() - 1) {
    messageIndex++;
    display.setCursor(5, 20);
    display.setTextSize(1);
    displayMessage();
    delay(1); 
    display.startscrollleft(0x00, 0x0F);
  }
}

void prevMessage() {
  if (messageIndex > 0) {
    messageIndex--;
    display.setCursor(5, 20);
    display.setTextSize(1);
    displayMessage();
    delay(1); 
    display.startscrollleft(0x00, 0x0F);
  }
}

void resetMessages() {
  messageIndex = -1;
  display.clearDisplay();
  display.display();
  digitalWrite(ledPin, LOW);
}

void alertNewMessage() {
  digitalWrite(buzzer, HIGH);
  delay(1000);
  digitalWrite(buzzer, LOW);
  delay(1000);
  digitalWrite(ledPin, HIGH);
}