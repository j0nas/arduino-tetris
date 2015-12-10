
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// TFT display and SD card will share the hardware SPI interface.
// Hardware SPI pins are specific to the Arduino board type and
// cannot be remapped to alternate pins.  For Arduino Uno,
// Duemilanove, etc., pin 11 = MOSI, pin 12 = MISO, pin 13 = SCK.
// #define SD_CS    4  // Chip select line for SD card
#define TFT_CS  10  // Chip select line for TFT display
#define TFT_DC   9  // Data/command line for TFT
#define TFT_RST  8  // Reset line for TFT (or connect to +5V)

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

void drawText(uint16_t color) {
  tft.setCursor(0, 0);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print("Lorem ipsum dolar sit amet.");
}

// Define colors to loosen coupling to screen implementation
#define COLOR_GRAY      tft.Color565(112, 112, 112)
#define COLOR_WHITE     ST7735_WHITE
#define COLOR_BLACK     ST7735_BLACK
#define COLOR_RED       ST7735_RED
#define COLOR_BLUE      ST7735_BLUE
#define COLOR_GREEN     ST7735_GREEN

#define BOARD_COLOR      COLOR_GRAY
#define BOARD_WIDTH     10
#define BOARD_HEIGHT    20
#define BOARD_OFFSET_X  2
#define BOARD_OFFSET_Y  17

#define MIN(X, Y)       (((X) < (Y)) ? (X) : (Y))
#define BLOCK_SIZE      MIN((tft.width() - 1) / BOARD_WIDTH, (tft.height() - 1) / BOARD_HEIGHT)

uint16_t grid[BOARD_WIDTH][BOARD_HEIGHT];

void fillBlock(byte x, byte y, uint16_t color) {
  grid[x][y] = color;
  tft.fillRect(1 + BOARD_OFFSET_X + (x * BLOCK_SIZE), 1 + BOARD_OFFSET_Y + (y * BLOCK_SIZE), BLOCK_SIZE - 1, BLOCK_SIZE - 1, color);
}

/*
  void _swap(uint16_t *a, uint16_t *b) {
  uint16_t tmp = *b;
   b = *a;
   a = tmp;
  }*/

byte shapes[2][4] = {
  {
    B1100,
    B1100,
    B0000,
    B0000
  }, {
    B1000,
    B1000,
    B1000,
    B1000
  }
};

int currentShape = 0;
byte getNthBit(byte c, byte n) {
  return ((c & (1 << n)) >> n);
}

bool isShapeHittingBottom(byte shapeNumber, byte offset) { // TODO: compensate for orientation
  byte shapeLength = 0;
  switch (shapeNumber) {
    case 0:
      shapeLength = 2;
      break;
    case 1:
      shapeLength = 4;
      break;
  }

  return (shapeLength + offset) >= BOARD_HEIGHT;
}

void gravity() {
  static bool hitBottom = false;
  static int offset = 0;
  if (hitBottom) {
    return;
  }

  for (byte i = 0; i < 4; i++) {
    int x = 0;
    for (byte j = 3; j != 0; j--) {
      if (getNthBit(shapes[currentShape][i], j) == 1) {
        if (offset > 0 && i == 0) {
          fillBlock(x, offset - 1 + i, COLOR_BLACK);
        }

        fillBlock(x++, offset + i, COLOR_RED);
      }
    }
  }

  if (isShapeHittingBottom(currentShape, offset++)) {
    hitBottom = true;
    offset = 0;
  }
}

void drawGrid() {
  for (int i = 0; i < BOARD_HEIGHT + 1; i++) {
    tft.drawFastHLine(BOARD_OFFSET_X, BOARD_OFFSET_Y + (i * BLOCK_SIZE), (BLOCK_SIZE * BOARD_WIDTH), BOARD_COLOR);
  }

  // Vertical lines
  for (int i = 0; i < BOARD_WIDTH + 1; i++) {
    tft.drawFastVLine(BOARD_OFFSET_X + (i * BLOCK_SIZE), BOARD_OFFSET_Y, (BLOCK_SIZE * BOARD_HEIGHT), BOARD_COLOR);
  }
}


void setup()
{
  tft.initR(INITR_BLACKTAB);
  delay(500);
  tft.fillScreen(ST7735_BLACK);
  tft.print("Orientation\nverification");
  delay(1000);
  drawGrid();
}

void loop()
{
  delay(700);
  gravity();
}

