#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

#define LCD_CS  2
#define LCD_RST 3
#define LCD_DC  4
#define LCD_BACKLIGHT 10

#define LED_BLUE  6
#define LED_RED  9

#define BTN_LEFT  A0
#define BTN_UP  A1
#define BTN_DOWN  A2
#define BTN_RIGHT  A3
#define BTN_BACK 7
#define BTN_ACCEPT 8

#define MAX_BULLETS 16
#define MAX_INVADERS  32

#define PLAYER_WIDTH 5
#define PLAYER_HEIGHT 4

#define BULLET_WIDTH  2
#define BULLET_HEIGHT 2

#define INVADER_WIDTH 3
#define INVADER_HEIGHT 3

#define GAME_SPEED  50

Adafruit_PCD8544 display = Adafruit_PCD8544(LCD_DC, LCD_CS, LCD_RST);

int bulletsX[MAX_BULLETS];
int bulletsY[MAX_BULLETS];

int invadersX[MAX_INVADERS];
int invadersY[MAX_INVADERS];

// init
int rootX = 0;
int rootY = 0;

int playerX = 0;
int playerY = (display.height() - PLAYER_HEIGHT) / 2;

int maxInvaders = 5;
int invadersDensity = 5;

int energy = 3;
int score = 0;

int shakeScreenTimeout = 0;
int shakeScreenIntensivity = 0;

boolean isHit = false;
boolean isContact = false;

boolean isGameOver = false;

void setup() {
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN , INPUT_PULLUP);
  pinMode(BTN_ACCEPT, INPUT_PULLUP);
  pinMode(BTN_BACK, INPUT_PULLUP);

  pinMode(LCD_BACKLIGHT, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  digitalWrite(LCD_BACKLIGHT, HIGH);

  for (int i = 0; i < MAX_BULLETS; i++) {
    bulletsX[i] = 0;
    bulletsY[i] = 0;
  }

  for (int i = 0; i < MAX_INVADERS; i++) {
    invadersX[i] = 0;
    invadersY[i] = 0;
  }

  display.begin();
  display.setContrast(60);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.clearDisplay();
  display.display();
}

bool isButtonDown(int pin) {
  if (digitalRead(pin) == LOW) {
    return true;
  }

  return false;
}

boolean checkForCollision(int rect1x, int rect1y, int rect1w, int rect1h,
                          int rect2x, int rect2y, int rect2w, int rect2h) {
  if (rect1x < rect2x + rect2w &&
      rect1x + rect1w > rect2x &&
      rect1y < rect2y + rect2h &&
      rect1h + rect1y > rect2y) {
    return true;
  }

  return false;
}

void loop() {
  display.clearDisplay();

  if (isGameOver) {
    display.setCursor(13, 20);
    digitalWrite(LED_RED, HIGH);
    display.print("GAME OVER!");
  }

  if (shakeScreenTimeout > 0) {
    rootX = random(0, shakeScreenIntensivity);
    rootY = random(0, shakeScreenIntensivity);

    if (isHit) {
      digitalWrite(LED_BLUE, HIGH);
    }

    if (isContact) {
      digitalWrite(LED_RED, HIGH);
    }

    shakeScreenTimeout--;

    if (shakeScreenTimeout == 0) {
      rootX = 0;
      rootY = 0;

      isHit = false;
      isContact = false;

      digitalWrite(LED_BLUE, LOW);
      digitalWrite(LED_RED, LOW);
    }
  }

  if (!isGameOver) {
    // check for contact
    for (int j = 0; j < MAX_INVADERS; j++) {
      if (invadersX[j] == 0) {
        continue;
      }

      if (checkForCollision(invadersX[j], invadersY[j], INVADER_WIDTH, INVADER_HEIGHT,
                            playerX, playerY, PLAYER_WIDTH, PLAYER_HEIGHT) == true) {
        invadersX[j] = 0;
        invadersY[j] = 0;

        isContact = true;

        energy--;

        if (energy == 0) {
          isGameOver = true;
        }

        shakeScreenTimeout = 20;
        shakeScreenIntensivity = 10;
      }
    }

    // check for hit
    for (int i = 0; i < MAX_BULLETS; i++) {
      for (int j = 0; j < MAX_INVADERS; j++) {
        // collision
        if (invadersX[j] == 0 || bulletsX[i] == 0) {
          continue;
        }

        if (checkForCollision(invadersX[j], invadersY[j], INVADER_WIDTH, INVADER_HEIGHT,
                              bulletsX[i], bulletsY[i], BULLET_WIDTH, BULLET_HEIGHT) == true) {
          bulletsX[i] = 0;
          bulletsY[i] = 0;

          invadersX[j] = 0;
          invadersY[j] = 0;

          shakeScreenTimeout = 5;
          shakeScreenIntensivity = 3;

          isHit = true;

          score++;

          if (score > 0) {
            if (score % 10 == 0 && maxInvaders < 32) {
              maxInvaders += 4;
            }

            if (score > 0 && score % 20 == 0 && invadersDensity > 3) {
              invadersDensity--;
            }
          }
        }
      }
    }

    // draw player
    display.fillRect(rootX + playerX, rootY + playerY, 2, PLAYER_HEIGHT, BLACK);
    display.fillRect(rootX + playerX + 2, rootY + playerY + 1, PLAYER_HEIGHT - 1, 2, BLACK);
  }

  // draw bullets
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bulletsX[i] == 0) {
      continue;
    }

    bulletsX[i]++;

    if (bulletsX[i] > display.width()) {
      bulletsX[i] = 0;
      bulletsY[i] = 0;
      continue;
    }

    display.fillRect(rootX + bulletsX[i], rootY + bulletsY[i], BULLET_WIDTH, BULLET_HEIGHT, BLACK);
  }

  // generate invaders
  if (random(0, invadersDensity) == 1) {
    int posY = random(3, display.height() - 3);
    int posX = display.width() - 3;

    for (int i = 0; i < maxInvaders; i++) {
      if (invadersX[i] == 0) {
        invadersX[i] = posX;
        invadersY[i] = posY;
        break;
      }
    }
  }

  // draw invaders
  for (int i = 0; i < maxInvaders; i++) {
    if (invadersX[i] > 0) {
      invadersX[i]--;
      display.fillRect(rootX + invadersX[i], rootY + invadersY[i], INVADER_HEIGHT, INVADER_WIDTH, BLACK);
    }
  }

  // draw ui
  display.setCursor(rootX, rootY + display.height() - 8);

  for (int i = 0; i < energy; i++) {
    display.write(3);
  }

  display.print(score);
  display.display();

  if (!isGameOver) {
    // actions
    if (isButtonDown(BTN_UP)) {
      if (playerY > 0) {
        playerY--;
      }
    }

    if (isButtonDown(BTN_DOWN)) {
      if (playerY < display.height() - PLAYER_HEIGHT) {
        playerY++;
      }
    }

    if (isButtonDown(BTN_LEFT)) {
      if (playerX > 0) {
        playerX--;
      }
    }

    if (isButtonDown(BTN_RIGHT)) {
      if (playerX < display.width() - PLAYER_WIDTH) {
        playerX++;
      }
    }

    if (isButtonDown(BTN_ACCEPT)) {
      for (int i = 0; i < MAX_BULLETS; i++) {
        if (bulletsX[i] == 0) {
          bulletsX[i] = playerX + 3;
          bulletsY[i] = playerY + 1;
          break;
        }
      }
    }
  } else {
    if (isButtonDown(BTN_BACK)) {
      for (int i = 0; i < MAX_BULLETS; i++) {
        bulletsX[i] = 0;
        bulletsY[i] = 0;
      }
    
      for (int i = 0; i < MAX_INVADERS; i++) {
        invadersX[i] = 0;
        invadersY[i] = 0;
      }
      
      playerX = 0;
      playerY = (display.height() - PLAYER_HEIGHT) / 2;

      energy = 3;
      score = 0;

      maxInvaders = 5;
      invadersDensity = 5;

      isGameOver = false;

      digitalWrite(LED_RED, LOW);
    }
  }

  delay(GAME_SPEED);
}
