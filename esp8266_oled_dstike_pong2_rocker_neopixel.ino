# MIT License
# Copyright (c) 2025 Benb0jangles
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_NeoPixel.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define I2C_ADDRESS 0x3C

// Pin definitions
#define SDA_PIN 5   // GPIO5 (D1)
#define SCL_PIN 4   // GPIO4 (D2)
#define BUTTON_UP 12    // GPIO12 for Up
#define BUTTON_DOWN 13  // GPIO13 for Down
#define NEOPIXEL_PIN 15
#define NUM_LEDS 1

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Game Variables
float ball_x = 64, ball_y = 32;   // Ball start position
float ball_dx = 3, ball_dy = 3;   // Ball movement direction
const int paddle_speed = 3;
const int paddle_height = 16;
const int paddle_width = 3;

// Paddle positions
int paddle1_x = 5;
int paddle1_y = 24;
int paddle2_x = 120;
int paddle2_y = 24;

// Scores
int score_p1 = 0;
int score_p2 = 0;

// LED Effect
bool ledActive = false;
unsigned long effectStartTime = 0;
const uint16_t EFFECT_DURATION = 500; // 0.5 second effect

void setup() {
  Serial.begin(74880);
  Serial.println("\nESP8266 Pong Starting...");
  
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  
  Wire.begin(SDA_PIN, SCL_PIN);
  
  delay(250);
  display.begin(I2C_ADDRESS, true);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  
  strip.begin();
  strip.setBrightness(50);
  strip.show();
}

void loop() {
  display.clearDisplay();
  
  // Game drawing and logic remains the same
  display.fillCircle(ball_x, ball_y, 2, SH110X_WHITE);
  display.fillRect(paddle1_x, paddle1_y, paddle_width, paddle_height, SH110X_WHITE);
  display.fillRect(paddle2_x, paddle2_y, paddle_width, paddle_height, SH110X_WHITE);
  
  ball_x += ball_dx;
  ball_y += ball_dy;
  
  if (ball_y <= 0 || ball_y >= SCREEN_HEIGHT - 3) {
    ball_dy *= -1;
  }
  
  if (ball_x >= paddle1_x && ball_x <= paddle1_x + paddle_width && 
      ball_y >= paddle1_y && ball_y <= paddle1_y + paddle_height) {
    ball_dx *= -1;
  }
  
  if (ball_x >= paddle2_x && ball_x <= paddle2_x + paddle_width && 
      ball_y >= paddle2_y && ball_y <= paddle2_y + paddle_height) {
    ball_dx *= -1;
  }
  
  // Modified scoring triggers
  if (ball_x < 0) {  // Player 2 scores - no LED effect
    ball_x = 64;
    ball_y = 32;
    score_p2++;
  }
  if (ball_x > SCREEN_WIDTH) {  // Player 1 scores - trigger LED
    ball_x = 64;
    ball_y = 32;
    score_p1++;
    ledActive = true;
    effectStartTime = millis();
  }
  
  // Score display
  display.setCursor(30, 5);
  display.print("P1: ");
  display.print(score_p1);
  
  display.setCursor(80, 5);
  display.print("P2: ");
  display.print(score_p2);
  
  // Controls
  if (digitalRead(BUTTON_UP) == LOW) {
    paddle1_y -= paddle_speed;
  }
  if (digitalRead(BUTTON_DOWN) == LOW) {
    paddle1_y += paddle_speed;
  }
  
  // AI
  if (ball_y > paddle2_y + paddle_height / 2) {
    paddle2_y += 2;
  } else if (ball_y < paddle2_y + paddle_height / 2) {
    paddle2_y -= 2;
  }
  
  // Paddle limits
  paddle1_y = max(0, min(SCREEN_HEIGHT - paddle_height, paddle1_y));
  paddle2_y = max(0, min(SCREEN_HEIGHT - paddle_height, paddle2_y));
  
  display.display();

  // Modified NeoPixel effect (1 second, only for P1)
  if (ledActive) {
    unsigned long currentTime = millis();
    unsigned long elapsed = currentTime - effectStartTime;
    
    if (elapsed < EFFECT_DURATION) {
      // Faster color transition (full cycle in 1 second)
      uint16_t hue = (elapsed * 65535UL) / EFFECT_DURATION;
      uint32_t color = strip.ColorHSV(hue, 255, 128);
      strip.setPixelColor(0, color);
      strip.show();
    } else {
      strip.setPixelColor(0, 0);
      strip.show();
      ledActive = false;
    }
  }

  delay(50);
}
