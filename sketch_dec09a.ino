
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
#define COLOR_CYAN      ST7735_CYAN
#define COLOR_YELLOW    ST7735_YELLOW
#define COLOR_BLUE      ST7735_BLUE
#define COLOR_ORANGE    tft.Color565(255, 165, 0)
#define COLOR_LIME      tft.Color565(204, 255, 0)
#define COLOR_PURPLE    tft.Color565(128,0,128)
#define COLOR_RED       ST7735_RED

#define BOARD_COLOR      COLOR_GRAY
#define BOARD_WIDTH     10
#define BOARD_HEIGHT    20
#define BOARD_OFFSET_X  2
#define BOARD_OFFSET_Y  17

#define MIN(X, Y)       (((X) < (Y)) ? (X) : (Y))
#define BLOCK_SIZE      MIN((tft.width() - 1) / BOARD_WIDTH, (tft.height() - 1) / BOARD_HEIGHT)

#define SHAPE_COUNT     7

#define SHAPE_I         0
#define SHAPE_J         1
#define SHAPE_L         2
#define SHAPE_O         3
#define SHAPE_S         4
#define SHAPE_T         5
#define SHAPE_Z         6

#define SHAPE_I_COLOR   COLOR_CYAN
#define SHAPE_J_COLOR   COLOR_BLUE
#define SHAPE_L_COLOR   COLOR_ORANGE
#define SHAPE_O_COLOR   COLOR_YELLOW
#define SHAPE_S_COLOR   COLOR_LIME
#define SHAPE_T_COLOR   COLOR_PURPLE
#define SHAPE_Z_COLOR   COLOR_RED

int currentShape = 0;
uint16_t grid[BOARD_WIDTH][BOARD_HEIGHT];
uint16_t shapeColors[SHAPE_COUNT];
byte shapes[SHAPE_COUNT][4] = {
  {
    B0100,
    B0100,
    B0100,
    B0100
  }, {
    B0000,
    B0010,
    B0010,
    B0110
  }, {
    B0000,
    B0100,
    B0100,
    B0110
  }, {
    B1100,
    B1100,
    B0000,
    B0000
  }, {
    B0000,
    B0000,
    B0110,
    B1100
  }, {
    B0000,
    B0000,
    B0010,
    B0111
  }, {
    B0000,
    B0000,
    B0110,
    B0011
  }
};

uint16_t getCurrentShapeColor() {
  return shapeColors[currentShape];
}

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

byte getNthBit(byte c, byte n) {
  return ((c & (1 << n)) >> n);
}

byte getShapeHeight(byte shapeNumber) {
  byte shapeLength = 0;

  switch (shapeNumber) {
    case SHAPE_I:
      shapeLength = 4;
      break;
    case SHAPE_J:
    case SHAPE_L:
      shapeLength = 4;
      break;
    case SHAPE_O:
    case SHAPE_S:
    case SHAPE_T:
    case SHAPE_Z:
      shapeLength = 3;
      break;
  };

  return shapeLength;
}

int currentShapeXpos = 0;
bool isShapeHittingBottom(byte shapeNumber, byte shapeXpos, byte offset) { // TODO: compensate for orientation
  return false;// grid[shapeXpos][offset + getShapeHeight(currentShape)] != COLOR_BLACK || (getShapeHeight(currentShape) + offset) >= BOARD_HEIGHT;
}



int offset = 0;

void detectCurrentShapeCollision() {
  if (isShapeHittingBottom(currentShape, currentShapeXpos, offset - 1)) {
    offset = 0;
    currentShape = random(SHAPE_COUNT);
  }
}


void repaintShapeTrail(int shape) {
  bool columnsChecked[4] = {false, false, false, false};

  for (byte i = 0; i < 4; i++) {
    byte x = 0;
    for (byte j = 0; j != 4; j++) {
      x++;
      if (columnsChecked[x - 1]) {
        continue;
      }
      
      if (getNthBit(shapes[shape][i], j - 1) == 1) {
        fillBlock(x - 2, offset - 2 + i, COLOR_BLACK);
        columnsChecked[x - 1] = true;
      }
    }
  }
}

void gravity() {
  for (byte i = 0; i < 4; i++) {
    int x = 0;
    // TODO: set currentShapeXpos
    for (byte j = 0; j != 4; j++) {
      if (getNthBit(shapes[currentShape][i], j) == 1) {
        fillBlock(x, offset + i, getCurrentShapeColor());
      }
      x++;
    }
  }

  offset++;
  repaintShapeTrail(currentShape);
}

void drawGrid() {
  for (int i = 0; i < BOARD_HEIGHT + 1; i++) {
    tft.drawFastHLine(BOARD_OFFSET_X, BOARD_OFFSET_Y + (i * BLOCK_SIZE), (BLOCK_SIZE * BOARD_WIDTH), BOARD_COLOR);
  }

  for (int i = 0; i < BOARD_WIDTH + 1; i++) {
    tft.drawFastVLine(BOARD_OFFSET_X + (i * BLOCK_SIZE), BOARD_OFFSET_Y, (BLOCK_SIZE * BOARD_HEIGHT), BOARD_COLOR);
  }
}


void setup()
{
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST7735_BLACK);
  tft.print("Orientation\nverification");
  randomSeed(analogRead(0));

  shapeColors[SHAPE_I] = SHAPE_I_COLOR;
  shapeColors[SHAPE_J] = SHAPE_J_COLOR;
  shapeColors[SHAPE_L] = SHAPE_L_COLOR;
  shapeColors[SHAPE_O] = SHAPE_O_COLOR;
  shapeColors[SHAPE_S] = SHAPE_S_COLOR;
  shapeColors[SHAPE_T] = SHAPE_T_COLOR;
  shapeColors[SHAPE_Z] = SHAPE_Z_COLOR;

  currentShape = SHAPE_I;

  drawGrid();
}

void loop()
{
  delay(800);
  gravity();
  detectCurrentShapeCollision();
}

