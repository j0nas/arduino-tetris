//#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// TFT display and SD card will share the hardware SPI interface.
// Hardware SPI pins are specific to the Arduino board type and
// cannot be remapped to alternate pins.  For Arduino Uno,
// Duemilanove, etc., pin 11 = MOSI, pin 12 = MISO, pin 13 = SCK.
// #define SD_CS    4  // Chip select line for SD card
#define TFT_CS  10  // Chip select line for TFT display
#define TFT_DC  9   // Data/command line for TFT
#define TFT_RST 8   // Reset line for TFT (or connect to +5V)

#define JOY_X   A0
#define JOY_Y   A1
#define JOY_BTN 2

// Define colors to loosen coupling to screen implementation
#define COLOR_GRAY      tft.Color565(66, 66, 66)
#define COLOR_WHITE     ST7735_WHITE
#define COLOR_BLACK     ST7735_BLACK
#define COLOR_CYAN      ST7735_CYAN
#define COLOR_YELLOW    ST7735_YELLOW
#define COLOR_BLUE      ST7735_BLUE
#define COLOR_ORANGE    tft.Color565(255, 165, 0)
#define COLOR_LIME      tft.Color565(204, 255, 0)
#define COLOR_PURPLE    tft.Color565(128,0,128)
#define COLOR_RED       ST7735_RED

#define BOARD_COLOR       COLOR_GRAY
#define BACKGROUND_COLOR  COLOR_BLACK

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

#define MOVE_DELAY 85
#define DOWN_DELAY 150

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
byte currentShape = 0;
byte currentRotation = 0;

short yOffset = -4;
short xOffset = 0;
short lastY = -4;
short lastX = 0;

short level = 300;
unsigned long stamp = 0;
unsigned long lastDown = 0;

uint16_t grid[BOARD_WIDTH][BOARD_HEIGHT];
uint16_t shapeColors[SHAPE_COUNT];
const byte shapes[SHAPE_COUNT][2][4] = {
  {
    {
      B1000,
      B1000,
      B1000,
      B1000
    },
    {
      B0000,
      B1111,
      B0000,
      B0000
    }
  },
  {
    {
      B0000,
      B0100,
      B0100,
      B1100
    }
  },
  {
    {
      B0000,
      B1000,
      B1000,
      B1100
    }
  },
  {
    {
      B0000,
      B0000,
      B1100,
      B1100
    }
  },
  {
    {
      B0000,
      B0000,
      B0110,
      B1100
    }
  },
  {
    {
      B0000,
      B0000,
      B0100,
      B1110
    }
  },
  {
    {
      B0000,
      B0000,
      B1100,
      B0110
    }
  }
};

uint16_t getCurrentShapeColor() {
  return shapeColors[currentShape];
}

void fillBlock(byte x, byte y, uint16_t color) {
  grid[x][y] = color;
  tft.fillRect(1 + BOARD_OFFSET_X + (x * BLOCK_SIZE), 1 + BOARD_OFFSET_Y + (y * BLOCK_SIZE), BLOCK_SIZE - 1, BLOCK_SIZE - 1, color);
}

byte getNthBit(byte c, byte n) {
  return ((c & (1 << n)) >> n);
}

bool yOffsetAtBottom() {
  for (int i = 3; i != 0; i--) {
    for (int j = 3; j != 0; j--) {
      if (getNthBit(shapes[currentShape][currentRotation][i], j) == 1) {
        return (i + 1 + yOffset) >= BOARD_HEIGHT;
      }
    }
  }
}

void nextShape() {
  yOffset = -4;
  xOffset = 0;
  currentRotation = 0;
  currentShape = SHAPE_I; //random(SHAPE_COUNT);
}

void gameOver() {
  tft.setCursor(90, 20);
  tft.print("GAME");
  tft.setCursor(90, 30);
  tft.print("OVER");
  while (true) {
    bool isClicked = (digitalRead(JOY_BTN) == LOW);
    if (isClicked) {
      delay(150);
      isClicked = (digitalRead(JOY_BTN) == LOW);
    }

    if (isClicked) {
      tft.fillRect(90, 20, 25, 20, COLOR_BLACK);
      for (byte i = 0; i < BOARD_WIDTH; i++) {
        for (byte j = 0; j < BOARD_HEIGHT; j++) {
          fillBlock(i, j, COLOR_BLACK);
        }
      }

      nextShape();
      break;
    }

    delay(100);
  }
}

bool isShapeColliding() {
  short p[4];
  short shiftedUp[4];

  for (byte i = 0; i < 3; i++) {
    shiftedUp[i] = shapes[currentShape][currentRotation][i + 1];
  }
  shiftedUp[3] = 0;

  for (byte i = 0; i < 4; i++) {
    p[i] = shapes[currentShape][currentRotation][i] - (shapes[currentShape][currentRotation][i] & shiftedUp[i]);
  }

  for (byte i = 0; i < 4; i++) {
    byte x = 0;
    for (short j = 3; j != -1; j--) {
      if (getNthBit(p[i], j) == 1) {
        if (grid[xOffset + x][yOffset + i + 1] != COLOR_BLACK) {
          if (yOffset < -1) {
            gameOver();
          }

          return true;
        }
      }

      x++;
    }
  }

  return false;
}

void detectCurrentShapeCollision() {
  if (yOffsetAtBottom() || isShapeColliding()) {
    nextShape();
  }
}

void gravity(bool apply) {
  static byte lastXoffset = 0;

  for (byte k = 0; k < 2; k++) {
    for (byte i = 0; i < 4; i++) {
      byte x = 0;
      for (short j = 3; j != -1; j--) {
        if (getNthBit(shapes[currentShape][currentRotation][i], j) == 1) {
          fillBlock((k == 0 ? lastXoffset : xOffset) + x, yOffset + i, k == 0 ? COLOR_BLACK : getCurrentShapeColor());
        }

        x++;
      }
    }

    if (k == 0 && apply) {
      yOffset++;
    }
  }

  if (xOffset != lastXoffset) {
    lastXoffset = xOffset;
  }
}

void drawGrid() {
  for (int i = 0; i < BOARD_HEIGHT + 1; i++) {
    tft.drawFastHLine(BOARD_OFFSET_X, BOARD_OFFSET_Y + (i * BLOCK_SIZE), (BLOCK_SIZE * BOARD_WIDTH), BOARD_COLOR);
  }

  for (int i = 0; i < BOARD_WIDTH + 1; i++) {
    tft.drawFastVLine(BOARD_OFFSET_X + (i * BLOCK_SIZE), BOARD_OFFSET_Y, (BLOCK_SIZE * BOARD_HEIGHT), BOARD_COLOR);
  }
}

byte getShapeWidth() {
  static byte lastShape = currentShape;
  static byte lastWidth = 0;
  
  if (currentShape == lastShape && lastWidth != 0) {
    // If shape hasn't changed, return cached value.
    return lastWidth;
  } else {
    lastWidth = 0;
    lastShape = currentShape;
  }
  
  for (byte i = 0; i < 4; i++) {
    byte x = 0;
    for (short j = 3; j != -1; j--) {
      if (getNthBit(shapes[currentShape][currentRotation][i], j) == 1) {
        if (j == 0) {
          // Found largest possible value.
          lastWidth = 4;
          return 4;
        }

        if ((x + 1) > lastWidth) {
          lastWidth = x + 1;
        }
      }

      x++;
    }
  }

  return lastWidth;
}

bool canMove(bool left) {
  short predictedShapePositions[4];
  for (byte i = 0; i < 4; i++) {
    predictedShapePositions[i] =
      left ?
      ((shapes[currentShape][currentRotation][i] >> 1) - (shapes[currentShape][currentRotation][i] & (shapes[currentShape][currentRotation][i] >> 1))) :
      ((shapes[currentShape][currentRotation][i] << 1) - (shapes[currentShape][currentRotation][i] & (shapes[currentShape][currentRotation][i] << 1)));
  }

  for (byte i = 0; i < 4; i++) {
    byte x = 0;
    for (byte j = 0; j != 4; j++) {
      if ((getNthBit(predictedShapePositions[i], j) == 1) && (grid[xOffset + x][yOffset + i] != COLOR_BLACK)) {
        return false;
      }

      x++;
    }
  }

  return true;
}

void joystickMovement() {
  int joyX = analogRead(JOY_X);
  int joyY = analogRead(JOY_Y);
  unsigned long now = millis();

  static unsigned long lastMove = millis();
  static short lastYoffset = yOffset;
  static bool hasClicked = false;

  // left
  if (joyX == 0 && xOffset > 0 && (now - lastMove) > MOVE_DELAY) {
    //if (canMove(true)) {
      lastMove = now;
      xOffset--;
    //}
  }

  // right
  if (joyX > 900 && xOffset < (BOARD_WIDTH - getShapeWidth()) && (now - lastMove) > MOVE_DELAY) {
    //if (canMove(false)) {
      lastMove = now;
      xOffset++;
    //}
  }

  // down
  if (yOffset < lastYoffset && !(joyY > 490 && joyY < 520)) {
    return;
  }

  lastYoffset = yOffset;
  if (joyY > 1000 && lastDown - now > DOWN_DELAY) {
    stamp -= level;
    lastDown = now;
  }

  // click
  bool isClicked = (digitalRead(JOY_BTN) == LOW);
  if (isClicked) {
    delay(50);
    isClicked = (digitalRead(JOY_BTN) == LOW);
  }

  hasClicked = hasClicked && isClicked;
  if (!hasClicked && isClicked) {
    hasClicked = true;

    for (byte k = 0; k < 1; k++) {
      for (byte i = 0; i < 4; i++) {
        int x = 0;
        for (short j = 3; j != -1; j--) {
          if (getNthBit(shapes[currentShape][currentRotation][i], j) == 1) {
            fillBlock(xOffset + x, yOffset + i, k == 0 ? COLOR_BLACK : getCurrentShapeColor());
          }

          x++;
        }
      }

      currentRotation = currentRotation == 0 ? 1 : 0;
    }
  }
}

void setup() {
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST7735_BLACK);

  pinMode(JOY_BTN, INPUT_PULLUP);
  pinMode(JOY_X, INPUT);
  pinMode(JOY_Y, INPUT);

  randomSeed(analogRead(A2));

  shapeColors[SHAPE_I] = SHAPE_I_COLOR;
  shapeColors[SHAPE_J] = SHAPE_J_COLOR;
  shapeColors[SHAPE_L] = SHAPE_L_COLOR;
  shapeColors[SHAPE_O] = SHAPE_O_COLOR;
  shapeColors[SHAPE_S] = SHAPE_S_COLOR;
  shapeColors[SHAPE_T] = SHAPE_T_COLOR;
  shapeColors[SHAPE_Z] = SHAPE_Z_COLOR;

  for (byte i = 0; i < BOARD_WIDTH; i++) {
    for (byte j = 0; j < BOARD_HEIGHT; j++) {
      fillBlock(i, j, COLOR_BLACK);
    }
  }

  Serial.begin(9600);
  while (!Serial);

  drawGrid();
  nextShape();
  stamp = millis();
}

void loop() {
  if ((millis() - stamp) > level) {
    stamp = millis();
    gravity(true);
  }

  if ((lastX != xOffset) || (lastY != yOffset)) {
    gravity(false);
    detectCurrentShapeCollision();
    lastX = xOffset;
    lastY = yOffset;
  }

  joystickMovement();
}

