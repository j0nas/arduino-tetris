#include <Adafruit_ST7735.h>

#include "config.h"
#include "shapes.h"

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
byte currentShape = 0;
byte currentRotation = 0;

short yOffset = -4;
short xOffset = 0;
short lastY = -4;
short lastX = 0;

unsigned short level = 300;
unsigned int score = 0;
unsigned long stamp = 0;
unsigned long lastDown = 0;

uint16_t grid[BOARD_WIDTH][BOARD_HEIGHT];
uint16_t shapeColors[SHAPE_COUNT];

uint16_t getCurrentShapeColor() {
  return shapeColors[currentShape];
}

void fillBlock(byte x, byte y, uint16_t color) {
  grid[x][y] = color;
  tft.fillRect(1 + BOARD_OFFSET_X + (x * BLOCK_SIZE), 1 + BOARD_OFFSET_Y + (y * BLOCK_SIZE), BLOCK_SIZE - 1, BLOCK_SIZE - 1, color);
}

bool hittingBottom() {
  for (int i = 3; i != 0; i--) {
    for (int j = 3; j != 0; j--) {
      if (bitRead(shapes[currentShape][currentRotation][i], j) == 1) {
        return (i + 1 + yOffset) >= BOARD_HEIGHT;
      }
    }
  }
}

void nextShape() {
  yOffset = -4;
  xOffset = 0;
  currentRotation = 0;
  currentShape = random(SHAPE_COUNT);
}

void tftPrint(int x, int y, String message) {
  tft.setCursor(x, y);
  tft.print(message);
}

void redrawScore() {
  tft.fillRect(SCORE_X, SCORE_Y + 10, 40, 10, COLOR_BLACK);
  tftPrint(SCORE_X, SCORE_Y + 10, String(score));
}

void gameOver() {
  tftPrint(GAMEOVER_X, GAMEOVER_Y, "GAME");
  tftPrint(GAMEOVER_X, GAMEOVER_Y + 10, "OVER");

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

      
      redrawScore();
      score = 0;
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
      if (bitRead(p[i], j) == 1) {
        if (grid[xOffset + x][max(0, yOffset + i + 1)] != COLOR_BLACK) {
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
  if (hittingBottom() || isShapeColliding()) {
    checkForTetris();
    nextShape();
  }
}

void checkForTetris() {
  bool foundScore = false;
  byte rowsPastFirstMatching = 0;
  for (byte row = 0; row < BOARD_HEIGHT; row++) {
    // Tiny optimization: no need to scan more than four rows for line clears
    if (foundScore && (++rowsPastFirstMatching > 4)) {
      return;
    }

    for (byte col = 0; col < BOARD_WIDTH; col++) {
      if (grid[col][row] == COLOR_BLACK) {
        break;
      }

      // If detected full line
      if (col == (BOARD_WIDTH - 1)) {
        score += LINE_SCORE_VALUE;
        foundScore = true;

        for (byte i = 0; i < BOARD_WIDTH; i++) {
          fillBlock(i, row, COLOR_BLACK);
        }

        for (byte r = row; r > 1; r--) {
          for (byte c = 0; c < BOARD_WIDTH; c++) {
            swap(grid[c][r], grid[c][r - 1]);
            fillBlock(c, r, grid[c][r]);
          }
        }

      }
    }
  }

  if (foundScore) {
    redrawScore();
  }
}

void gravity(bool apply) {
  static byte lastXoffset = 0;

  for (byte k = 0; k < 2; k++) {
    for (byte i = 0; i < 4; i++) {
      byte x = 0;
      for (short j = 3; j != -1; j--) {
        if (bitRead(shapes[currentShape][currentRotation][i], j) == 1) {
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
  static byte lastRotation = currentRotation;
  static byte lastWidth = 0;

  if (currentShape == lastShape && lastRotation == currentRotation && lastWidth != 0) {
    // If shape hasn't changed, return cached value.
    return lastWidth;
  } else {
    lastWidth = 0;
    lastShape = currentShape;
    lastRotation = currentRotation;
  }

  for (byte i = 0; i < 4; i++) {
    byte x = 0;
    for (short j = 3; j != -1; j--) {
      if (bitRead(shapes[currentShape][currentRotation][i], j) == 1) {
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
  byte predictedShapePositions[4];
  for (byte i = 0; i < 4; i++) {
    byte shiftedPiece = (left ? shapes[currentShape][currentRotation][i] >> 1 : shapes[currentShape][currentRotation][i] << 1) & B00001111;
    predictedShapePositions[i] = shapes[currentShape][currentRotation][i] - (shiftedPiece & shapes[currentShape][currentRotation][i]);
  }

  for (byte i = 0; i < 4; i++) {
    byte x = 0;
    for (short j = 3; j != -1; j--) {
      if ((yOffset + i) >= 0) {
        if ((bitRead(predictedShapePositions[i], j) == 1) && (grid[xOffset + x + (left ? -1 : 1)][yOffset + i] != COLOR_BLACK)) {
          return false;
        }
      }

      x++;
    }
  }

  return true;
}

void rotate() {
  if (shapeRotations[currentShape] == 1) {
    return;
  }

  for (byte k = 0; k < 1; k++) {
    for (byte i = 0; i < 4; i++) {
      int x = 0;
      for (short j = 3; j != -1; j--) {
        if (bitRead(shapes[currentShape][currentRotation][i], j) == 1) {
          fillBlock(xOffset + x, yOffset + i, k == 0 ? COLOR_BLACK : getCurrentShapeColor());
        }

        x++;
      }
    }

    currentRotation = ((shapeRotations[currentShape] - 1) == currentRotation ? 0 : currentRotation + 1);
  }
}

void joystickMovement() {
  int joyX = analogRead(JOY_X);
  int joyY = analogRead(JOY_Y);
  unsigned long now = millis();

  static unsigned long lastMove = millis();
  static short lastYoffset = yOffset;
  static bool hasClicked = false;

  // left
  if (joyX < 50 && xOffset > 0 && (now - lastMove) > MOVE_DELAY) {
    if (canMove(true)) {
      lastMove = now;
      xOffset--;
    }
  }

  // right
  if (joyX > 900 && xOffset < (BOARD_WIDTH - getShapeWidth()) && (now - lastMove) > MOVE_DELAY) {
    if (canMove(false)) {
      lastMove = now;
      xOffset++;
    }
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
    rotate();
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

  tftPrint(SCORE_X, SCORE_Y, "SCORE");
  tftPrint(SCORE_X, SCORE_Y + 10, "0");

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

