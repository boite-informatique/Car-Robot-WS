// // CustomServo.ino

// #define MIN_PULSE_WIDTH       544     // minimum pulse width
// #define MAX_PULSE_WIDTH      2400     // maximum pulse width
// #define DEFAULT_PULSE_WIDTH  1500     // default pulse width

// class CustomServo {
// public:
//     CustomServo();
//     void attach(int pin);
//     void detach();
//     void write(int angle);
//     void writeMicroseconds(int pulseWidth);
//     int read();
//     int readMicroseconds();
//     bool attached();

// private:
//     int servoPin;
//     int pulseWidth;
//     bool isAttached;
// };

// CustomServo::CustomServo() : servoPin(-1), pulseWidth(DEFAULT_PULSE_WIDTH), isAttached(false) {}

// void CustomServo::attach(int pin) {
//     servoPin = pin;
//     pinMode(servoPin, OUTPUT);
//     isAttached = true;
// }

// void CustomServo::detach() {
//     isAttached = false;
// }

// void CustomServo::write(int angle) {
//     if (angle < 0) angle = 0;
//     if (angle > 180) angle = 180;
//     pulseWidth = map(angle, 0, 180, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH);
//     writeMicroseconds(pulseWidth);
// }

// void CustomServo::writeMicroseconds(int pulseWidth) {
//     if (pulseWidth < MIN_PULSE_WIDTH) pulseWidth = MIN_PULSE_WIDTH;
//     if (pulseWidth > MAX_PULSE_WIDTH) pulseWidth = MAX_PULSE_WIDTH;
//     this->pulseWidth = pulseWidth;
//     if (isAttached) {
//         digitalWrite(servoPin, HIGH);
//         delayMicroseconds(pulseWidth);
//         digitalWrite(servoPin, LOW);
//     }
// }

// int CustomServo::read() {
//     return map(pulseWidth, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH, 0, 180);
// }

// int CustomServo::readMicroseconds() {
//     return pulseWidth;
// }

// bool CustomServo::attached() {
//     return isAttached;
// }

// CustomServo myServo;

// // void setup() {
// //     myServo.attach(16);  // Attach the servo to pin 9
// // }

// // void loop() {
// //    for (int angle = 0; angle <= 45; angle++) {
// //         myServo.write(angle);
// //         delay(15);  // Wait 15ms for the servo to reach the position
// //     }
// //     delay(2000);
// //     for (int angle = 45; angle <= 90; angle++) {
// //         myServo.write(angle);
// //         delay(15);  // Wait 15ms for the servo to reach the position
// //     }
// //     delay(2000);
// // for (int angle = 90; angle <= 135; angle++) {
// //         myServo.write(angle);
// //         delay(15);  // Wait 15ms for the servo to reach the position
// //     }
// // delay(2000);
// // for (int angle = 135; angle <= 180; angle++) {
// //         myServo.write(angle);
// //         delay(15);  // Wait 15ms for the servo to reach the position
// //     }
// // delay(2000);
// //     // Sweep the servo from 180 to 0 degrees
// //     for (int angle = 180; angle >= 0; angle--) {
// //         myServo.write(angle);
// //         delay(15);  // Wait 15ms for the servo to reach the position
// //     }
// // }