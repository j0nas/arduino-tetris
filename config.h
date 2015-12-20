#ifndef config_h
#define config_h

// TFT display and SD card will share the hardware SPI interface.
// Hardware SPI pins are specific to the Arduino board type and
// cannot be remapped to alternate pins.  For Arduino Uno,
// Duemilanove, etc., pin 11 = MOSI, pin 12 = MISO, pin 13 = SCK.
// #define SD_CS    4  // Chip select line for SD card
#define TFT_CS            10  // Chip select line for TFT display
#define TFT_DC            9   // Data/command line for TFT
#define TFT_RST           8   // Reset line for TFT (or connect to +5V)
#define SD_CS             4

#define JOY_X             A0
#define JOY_Y             A1
#define JOY_BTN           2

#define BUZZER            7

#define BOARD_COLOR       COLOR_GRAY
#define BACKGROUND_COLOR  COLOR_BLACK

#define BOARD_WIDTH       10
#define BOARD_HEIGHT      20
#define BOARD_OFFSET_X    2
#define BOARD_OFFSET_Y    17

#define GAMEOVER_X        90
#define GAMEOVER_Y        20

#define SCORE_X           GAMEOVER_X
#define SCORE_Y           GAMEOVER_Y + 50
#define LINE_SCORE_VALUE  100

#define MIN(X, Y)         (((X) < (Y)) ? (X) : (Y))
#define BLOCK_SIZE        MIN((tft.width() - 1) / BOARD_WIDTH, (tft.height() - 1) / BOARD_HEIGHT)

#define SHAPE_COUNT       7

#define SHAPE_I           0
#define SHAPE_J           1
#define SHAPE_L           2
#define SHAPE_O           3
#define SHAPE_S           4
#define SHAPE_T           5
#define SHAPE_Z           6

#define COLOR_GRAY        tft.Color565(33, 33, 33)
#define COLOR_WHITE       ST7735_WHITE
#define COLOR_BLACK       ST7735_BLACK
#define COLOR_CYAN        ST7735_CYAN
#define COLOR_YELLOW      ST7735_YELLOW
#define COLOR_BLUE        ST7735_BLUE
#define COLOR_ORANGE      tft.Color565(255, 165, 0)
#define COLOR_LIME        tft.Color565(204, 255, 0)
#define COLOR_PURPLE      tft.Color565(128,0,128)
#define COLOR_RED         ST7735_RED

#define SHAPE_I_COLOR     COLOR_CYAN
#define SHAPE_J_COLOR     COLOR_BLUE
#define SHAPE_L_COLOR     COLOR_ORANGE
#define SHAPE_O_COLOR     COLOR_YELLOW
#define SHAPE_S_COLOR     COLOR_LIME
#define SHAPE_T_COLOR     COLOR_PURPLE
#define SHAPE_Z_COLOR     COLOR_RED

#define FILE_NAME         "highscores.txt"

#define MOVE_DELAY        50
#define DOWN_DELAY        150

#define NEXTSHAPE_X       GAMEOVER_X
#define NEXTSHAPE_Y       SCORE_Y + 50
#endif
