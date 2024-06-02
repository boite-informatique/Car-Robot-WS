/*
 * WebSocketClient.ino
 *
 *  Created on: 24.05.2015
 *
 */

#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>

#include <WebSocketsClient.h>

#include "esp_camera.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems

// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define MOTOR_1_PIN_1    14
#define MOTOR_1_PIN_2    15
#define MOTOR_2_PIN_1    12
#define MOTOR_2_PIN_2    13
#define FLASH_PIN         4
#define SERVO_PIN         2


// CustomServo.ino

#define MIN_PULSE_WIDTH       544     // minimum pulse width
#define MAX_PULSE_WIDTH      2400     // maximum pulse width
#define DEFAULT_PULSE_WIDTH  1500     // default pulse width

class CustomServo {
public:
    CustomServo();
    void attach(int pin);
    void detach();
    void write(int angle);
    void writeMicroseconds(int pulseWidth);
    int read();
    int readMicroseconds();
    bool attached();

private:
    int servoPin;
    int pulseWidth;
    bool isAttached;
};

CustomServo::CustomServo() : servoPin(-1), pulseWidth(DEFAULT_PULSE_WIDTH), isAttached(false) {}

void CustomServo::attach(int pin) {
    servoPin = pin;
    isAttached = true;
}

void CustomServo::detach() {
    isAttached = false;
}

void CustomServo::write(int angle) {
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;
    pulseWidth = map(angle, 0, 180, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH);
    writeMicroseconds(pulseWidth);
}

void CustomServo::writeMicroseconds(int pulseWidth) {
    if (pulseWidth < MIN_PULSE_WIDTH) pulseWidth = MIN_PULSE_WIDTH;
    if (pulseWidth > MAX_PULSE_WIDTH) pulseWidth = MAX_PULSE_WIDTH;
    this->pulseWidth = pulseWidth;
    if (isAttached) {
        digitalWrite(servoPin, HIGH);
        delayMicroseconds(pulseWidth);
        digitalWrite(servoPin, LOW);
    }
}

int CustomServo::read() {
    return map(pulseWidth, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH, 0, 180);
}

int CustomServo::readMicroseconds() {
    return pulseWidth;
}

bool CustomServo::attached() {
    return isAttached;
}

/* servo lib end */



const char* WIFI_SSID = "cnx";
const char* WIFI_PASSWORD = "niggdo08";
const char* WS_SERVER_URL = "192.168.32.2";

unsigned long photoPreviousMillis = 0;
const long photoInterval = 100;

unsigned long servoLeftPreviousMillis = 0;
unsigned long servoRightPreviousMillis = 0;
const long servoInterval = 15;
bool servoRight = false;
bool servoLeft = false;

WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
CustomServo myServo;

void hexdump(const void *mem, uint32_t len, uint8_t cols = 16) {
	const uint8_t* src = (const uint8_t*) mem;
	Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
	for(uint32_t i = 0; i < len; i++) {
		if(i % cols == 0) {
			Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
		}
		Serial.printf("%02X ", *src);
		src++;
	}
	Serial.printf("\n");
}

void send_photo() {
  if(!webSocket.isConnected())
    return;
  camera_fb_t * fb = NULL;
  
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    return;
  }
  webSocket.sendBIN(fb->buf, fb->len);
  esp_camera_fb_return(fb);
}
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  char payloadStr[length + 1];
	switch(type) {
		case WStype_DISCONNECTED:
			Serial.printf("[WSc] Disconnected!\n");
			break;
		case WStype_CONNECTED:
			Serial.printf("[WSc] Connected to url: %s\n", payload);

			break;
		case WStype_TEXT:
			memcpy(payloadStr, payload, length);
			payloadStr[length] = '\0';

      if(!strcmp(payloadStr, "forward")) {
        Serial.println("Forward");
        digitalWrite(MOTOR_1_PIN_1, 1);
        digitalWrite(MOTOR_1_PIN_2, 0);
        digitalWrite(MOTOR_2_PIN_1, 1);
        digitalWrite(MOTOR_2_PIN_2, 0);
      }
      else if(!strcmp(payloadStr, "left")) {
        Serial.println("Left");
        digitalWrite(MOTOR_1_PIN_1, 0);
        digitalWrite(MOTOR_1_PIN_2, 1);
        digitalWrite(MOTOR_2_PIN_1, 1);
        digitalWrite(MOTOR_2_PIN_2, 0);
      }
      else if(!strcmp(payloadStr, "right")) {
        Serial.println("Right");
        digitalWrite(MOTOR_1_PIN_1, 1);
        digitalWrite(MOTOR_1_PIN_2, 0);
        digitalWrite(MOTOR_2_PIN_1, 0);
        digitalWrite(MOTOR_2_PIN_2, 1);
      }
      else if(!strcmp(payloadStr, "backward")) {
        Serial.println("Backward");
        digitalWrite(MOTOR_1_PIN_1, 0);
        digitalWrite(MOTOR_1_PIN_2, 1);
        digitalWrite(MOTOR_2_PIN_1, 0);
        digitalWrite(MOTOR_2_PIN_2, 1);
      }
      else if(!strcmp(payloadStr, "stop")) {
        Serial.println("Stop");
        digitalWrite(MOTOR_1_PIN_1, 0);
        digitalWrite(MOTOR_1_PIN_2, 0);
        digitalWrite(MOTOR_2_PIN_1, 0);
        digitalWrite(MOTOR_2_PIN_2, 0);
      }
      else if(!strcmp(payloadStr, "servoleftOn")) {
        Serial.println("servo left");
        // myServo.write(myServo.read() - 6); 
        servoRight = false;
        servoLeft = true;
      } 
      else if(!strcmp(payloadStr, "servorightOn")) {
        Serial.println("servo right");
        // myServo.write(myServo.read() + 6);
        servoLeft = false;
        servoRight = true;
      } else if(!strcmp(payloadStr, "servoleftOff")) {
        Serial.println("servo left off");
        // myServo.write(myServo.read() + 6); 
        servoLeft = false;
      }
      else if(!strcmp(payloadStr, "servorightOff")) {
        Serial.println("servo right off");
        // myServo.write(myServo.read() + 6); 
        servoRight = false;
      } else if(!strcmp(payloadStr, "flashon")) {
        Serial.println("flash on°");
        digitalWrite(FLASH_PIN, HIGH);
      } else if(!strcmp(payloadStr, "flashoff")) {
        Serial.println("flash off°");
        digitalWrite(FLASH_PIN, LOW);
      }

			// send message to server
			// webSocket.sendTXT("message here");
			break;
		case WStype_BIN:
			Serial.printf("[WSc] get binary length: %u\n", length);
			hexdump(payload, length);

			// send data to server
			// webSocket.sendBIN(payload, length);
			break;
		case WStype_ERROR:			
		case WStype_FRAGMENT_TEXT_START:
		case WStype_FRAGMENT_BIN_START:
		case WStype_FRAGMENT:
		case WStype_FRAGMENT_FIN:
			break;
	}

}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  pinMode(MOTOR_1_PIN_1, OUTPUT);
  pinMode(MOTOR_1_PIN_2, OUTPUT);
  pinMode(MOTOR_2_PIN_1, OUTPUT);
  pinMode(MOTOR_2_PIN_2, OUTPUT);
  pinMode(FLASH_PIN, OUTPUT);
  pinMode(SERVO_PIN, OUTPUT);
  myServo.attach(SERVO_PIN);

	Serial.begin(115200);
  Serial.flush();

  /* camera start */
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  /* camera end */


	WiFiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

	//WiFi.disconnect();
	while(WiFiMulti.run() != WL_CONNECTED) {
		delay(100);
    Serial.print(".");
	}
  
  Serial.println("");
  Serial.println("Connected to WiFi");
	// server address, port and URL
	webSocket.begin(WS_SERVER_URL, 3000, "/");

	// event handler
	webSocket.onEvent(webSocketEvent);

	// use HTTP Basic Authorization this is optional remove if not needed
	webSocket.setAuthorization("user", "Password");

	// try ever 5000 again if connection has failed
	webSocket.setReconnectInterval(5000);

}

void loop() {
	webSocket.loop();
  unsigned long currentMillis = millis();
    if (currentMillis - photoPreviousMillis >= photoInterval) {
        photoPreviousMillis = currentMillis;
        send_photo();  // Call send_photo() every 100 ms
    }

    if (servoLeft && currentMillis - servoLeftPreviousMillis >= servoInterval) {
        servoLeftPreviousMillis = currentMillis;
        int current = myServo.read();
        myServo.write(current - 3); 
    } else if (servoRight && currentMillis - servoRightPreviousMillis >= servoInterval) {
        servoRightPreviousMillis = currentMillis;
        int current = myServo.read();
        myServo.write(current + 3); 
    }
}
