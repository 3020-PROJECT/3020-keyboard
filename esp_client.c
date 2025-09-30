#include <WiFi.h>
#include <ESP32Video.h>
#include <Ressources/CodePage437_8x8.h>

// ========== WiFi Config ==========
const char* ssid     = "MAKERS_IOT";
const char* password = "1337@IOT";
const uint16_t serverPort = 1337;

// ========== VGA pins ==========
#define RED   14
#define GREEN 19
#define BLUE  27
#define HSYNC 32
#define VSYNC 33

#define NATIVE_TEXT_MODE 0

// ========== Screen params ==========
#define FONT_HIGHT 8
#define FONT_WIDTH 8
#define COLS 90   // 720 / 8
#define ROWS 50   // 400 / 8


char screen[ROWS][COLS] = {0};

// globals
VGA1BitI videodisplay;

int16_t cursor_x = 0, cursor_y = 0;
WiFiServer server(serverPort);
WiFiClient client;

// ---------- Helper functions ----------
void setCursor(int16_t x, int16_t y) {
  cursor_x = x;
  cursor_y = y;
  # if NATIVE_TEXT_MODE
    videodisplay.setCursor(cursor_x, cursor_y);
  # else
    videodisplay.setCursor(cursor_x * FONT_WIDTH, cursor_y * FONT_HIGHT + FONT_HIGHT);
  # endif
}

void putChar(char c){
  screen[cursor_y][cursor_x] = c;
  videodisplay.print(c);
  moveCursorNext();
}

void redrawScreen(void){
  videodisplay.clear();
  for (int y = 0; y < ROWS; y++){
    videodisplay.setCursor(0, y * FONT_HIGHT + FONT_HIGHT);
    for (int x = 0; x < COLS; x++){
      if (screen[y][x])
        videodisplay.print(screen[y][x]);
    }
  }
  setCursor(cursor_x, cursor_y);
}

void moveCursorNext() {
  cursor_x++;
  if (cursor_x >= COLS) {
    cursor_x = 0;
    cursor_y++;
  }
  if (cursor_y >= ROWS) {
    videodisplay.clear();
    cursor_x = 0;
    cursor_y = 0;
  }
  setCursor(cursor_x, cursor_y);
}

void handleChar(char c) {
  switch (c) {
    case '\n':
      cursor_x = 0;
      cursor_y++;
      if (cursor_y >= ROWS) {
        videodisplay.clear();
        cursor_x = 0;
        cursor_y = 0;
      }
      setCursor(cursor_x, cursor_y);
      break;
    case '\b':  // Backspace
      if (cursor_x > 0)
        cursor_x--;
      else if (cursor_y > 0){
        cursor_y--;
        cursor_x = COLS - 1;
      }
      setCursor(cursor_x, cursor_y);
      screen[cursor_y][cursor_x] = 0;
      redrawScreen();
      break;
    default:
      putChar(c);
      break;
  }
}

void printCentered(const char* s, int16_t row) {
  setCursor((COLS / 2) - (strlen(s) / 2), row);
  videodisplay.print(s);
}

void drawBoxAroundText(const char* s, int16_t row) {
  int len = strlen(s);
  int startX = (COLS / 2) - (len / 2) - 2;
  int endX = (COLS / 2) + (len / 2) + 2;
  int startY = row - 1;
  int endY = row + 1;

  // Top and bottom
  for (int x = startX; x <= endX; x++) {
    setCursor(x, startY); videodisplay.print("-");
    setCursor(x, endY);   videodisplay.print("-");
  }
  // Left and right
  for (int y = startY; y <= endY; y++) {
    setCursor(startX, y); videodisplay.print("|");
    setCursor(endX, y);   videodisplay.print("|");
  }
}

void showWelcomeScreen() {
  videodisplay.clear();
  char buf[64];
  snprintf(buf, sizeof(buf), "connect via %s:%d", WiFi.localIP().toString().c_str(), serverPort);

  drawBoxAroundText("Welcome to ESP-MONITOR", ROWS/2 - 2);
  printCentered("Welcome to ESP-MONITOR", ROWS/2 - 2);
  drawBoxAroundText(buf, ROWS/2 + 2);
  printCentered(buf, ROWS/2 + 2);

  cursor_x = 0;
  cursor_y = ROWS/2 + 5;
  setCursor(cursor_x, cursor_y);
}

// ========== Setup VGA ==========
void initVGA(void) {
  videodisplay.setFont(CodePage437_8x8);
  videodisplay.init(VGAMode::MODE720x400, RED, GREEN, BLUE, HSYNC, VSYNC);
  videodisplay.clear();
}

// ========== Setup ==========
void setup() {
  // WiFi connect
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
    delay(500);
  // init server
  server.begin();
  // Initial screen
  Serial.begin(115200);
  initVGA();
  showWelcomeScreen();
}

// ========== Loop ==========
void loop() {
  // if client disconnected
  if (client && !client.connected()) {
    client.stop();
    showWelcomeScreen();
  }

  // Accept new client if none is connected
  if (!client || !client.connected()) {
    client = server.available();
    if (!client) {
      // No client connected, show welcome screen
      delay(1000);
      return;
    }
    videodisplay.clear();
    cursor_x = 0;
    cursor_y = 0;
    setCursor(cursor_x, cursor_y);
  }

  // Handle client input
  while (client.available()) {
    char c = client.read();
    handleChar(c);
  }
}
