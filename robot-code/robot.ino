#define TINY_GSM_MODEM_SIM800
#include <SoftwareSerial.h>
SoftwareSerial SerialAT(12, 13);
#if !defined(TINY_GSM_RX_BUFFER)
#define TINY_GSM_RX_BUFFER 650
#endif
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 115200
#define GSM_PIN ""
#include <Adafruit_PWMServoDriver.h>
#include <ArduinoHttpClient.h>
#include <TinyGsmClient.h>
#include <Wire.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

const char apn[] = "my apn";
const char gprsUser[] = "";
const char gprsPass[] = "";

const char server[] = "server";
const uint16_t port = 123;
HttpClient http(client, server, port);

enum class FirstDriver {
  Left_go = 4,
  Left_back = 5,
  Right_go = 6,
  Right_back = 7,
  Left_PWM = 5,
  Right_PWM = 3
};

enum class SecondDriver {
  Left_go = 8,
  Left_back = 9,
  Right_go = 10,
  Right_back = 11,
  Left_PWM = 9,
  Right_PWM = 6
};

enum class ThirdDriver {
  Left_go = 12,
  Left_back = 13,
  Right_go = 14,
  Right_back = 15,
  Left_PWM = 11,
  Right_PWM = 10
};

enum class IRArray {
  Left_sensor = 2,
  Left_center_sensor = 4,
  Rigth_center_sensor = 7,
  Right_sensor = 8
};

#define DC_FORWARD(driver, speed)                                              \
  pwm.setPWM(int(driver::Left_go), 4096, 0);                                   \
  pwm.setPWM(int(driver::Right_go), 4096, 0);                                  \
  pwm.setPWM(int(driver::Left_back), 0, 4096);                                 \
  pwm.setPWM(int(driver::Right_back), 0, 4096);                                \
  digitalWrite(int(driver::Right_PWM), speed);                                 \
  digitalWrite(int(driver::Left_PWM), speed);

#define DC_BACK(driver, speed)                                                 \
  pwm.setPWM(int(driver::Left_go), 0, 4096);                                   \
  pwm.setPWM(int(driver::Right_go), 0, 4096);                                  \
  pwm.setPWM(int(driver::Left_back), 4096, 0);                                 \
  pwm.setPWM(int(driver::Right_back), 4096, 0);                                \
  digitalWrite(int(driver::Right_PWM), speed);                                 \
  digitalWrite(int(driver::Left_PWM), speed);

#define DC_STOP(driver)                                                        \
  pwm.setPWM(int(driver::Left_go), 0, 4096);                                   \
  pwm.setPWM(int(driver::Right_go), 0, 4096);                                  \
  pwm.setPWM(int(driver::Left_back), 0, 4096);                                 \
  pwm.setPWM(int(driver::Right_back), 0, 4096);                                \
  digitalWrite(int(driver::Right_PWM), 0);                                     \
  digitalWrite(int(driver::Left_PWM), 0);

#define DC_LEFT(driver, speed)                                                 \
  pwm.setPWM(int(driver::Left_go), 0, 4096);                                   \
  pwm.setPWM(int(driver::Right_go), 4096, 0);                                  \
  pwm.setPWM(int(driver::Left_back), 4096, 0);                                 \
  pwm.setPWM(int(driver::Right_back), 0, 4096);                                \
  digitalWrite(int(driver::Right_PWM), speed);                                 \
  digitalWrite(int(driver::Left_PWM), speed);

#define DC_RIGHT(driver, speed)                                                \
  pwm.setPWM(int(driver::Left_go), 4096, 0);                                   \
  pwm.setPWM(int(driver::Right_go), 0, 4096);                                  \
  pwm.setPWM(int(driver::Left_back), 0, 4096);                                 \
  pwm.setPWM(int(driver::Right_back), 4096, 0);                                \
  digitalWrite(int(driver::Right_PWM), speed);                                 \
  digitalWrite(int(driver::Left_PWM), speed);

void setup() {
  Serial.begin(9600);

  TinyGsmAutoBaud(SerialAT, GSM_AUTOBAUD_MIN, GSM_AUTOBAUD_MAX);

  modem.restart();
  if (GSM_PIN && modem.getSimStatus() != SIM_ANTITHEFT_LOCKED) {
    modem.simUnlock(GSM_PIN);
  }

  // pwm shield but not really pwm
  pwm.begin();
  pwm.setPWMFreq(1000);

  pinMode(int(FirstDriver::Left_PWM), OUTPUT);
  pinMode(int(FirstDriver::Right_PWM), OUTPUT);

  pinMode(int(SecondDriver::Left_PWM), OUTPUT);
  pinMode(int(SecondDriver::Right_PWM), OUTPUT);

  pinMode(int(ThirdDriver::Left_PWM), OUTPUT);
  pinMode(int(ThirdDriver::Right_PWM), OUTPUT);

  pinMode(int(IRArray::Left_sensor), INPUT);
  pinMode(int(IRArray::Left_center_sensor), INPUT);
  pinMode(int(IRArray::Right_sensor), INPUT);
  pinMode(int(IRArray::Rigth_center_sensor), INPUT);

  Wire.setClock(400000);
}

void run(uint8_t speed) {
  DC_FORWARD(FirstDriver, speed)
  DC_FORWARD(SecondDriver, speed)
  DC_FORWARD(ThirdDriver, speed)
}

void back(uint8_t speed) {
  DC_BACK(FirstDriver, speed)
  DC_BACK(SecondDriver, speed)
  DC_BACK(ThirdDriver, speed)
}

void brake() {
  DC_STOP(FirstDriver)
  DC_STOP(SecondDriver)
  DC_STOP(ThirdDriver)
}

void left(uint8_t speed) {
  DC_LEFT(FirstDriver, speed)
  DC_LEFT(SecondDriver, speed)
  DC_LEFT(ThirdDriver, speed)
}

void right(uint8_t speed) {
  DC_RIGHT(FirstDriver, speed)
  DC_RIGHT(SecondDriver, speed)
  DC_RIGHT(ThirdDriver, speed)
}

void loop() {
  // if all four are bad - go back
  if (digitalRead(int(IRArray::Left_sensor)) &
      digitalRead(int(IRArray::Left_center_sensor)) &
      digitalRead(int(IRArray::Right_sensor)) &
      digitalRead(int(IRArray::Rigth_center_sensor)) == HIGH) {
    back(255);
    delay(3000);
    brake();
  }
  // GPRS connection parameters are usually set after network registration
  Serial.print(F("Connecting to "));
  Serial.print(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    Serial.println(" fail");
    delay(10000);
    return;
  }
  Serial.println(" success");

  if (modem.isGprsConnected()) {
    Serial.println("GPRS connected");
  }
  Serial.print(F("Performing HTTP GET request... "));
  int err = http.get("/get_next");
  if (err != 0) {
    Serial.println(F("failed to connect"));
    delay(10000);
    return;
  }

  int status = http.responseStatusCode();
  Serial.print(F("Response status code: "));
  Serial.println(status);
  if (!status) {
    delay(10000);
    return;
  }

  String body = http.responseBody();
  Serial.println(F("Response:"));
  switch (body.toInt()) {
  case 0:
    run(255);
    break;
  case 1:
    left(255);
    break;
  case 2:
    right(255);
    break;
  case 3:
    back(255);
    break;
  case 4:
    brake();
    break;
  }
}
