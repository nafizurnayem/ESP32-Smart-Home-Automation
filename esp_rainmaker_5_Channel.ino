//Define the Node Name
char nodeName[] = "Nafizs-Smart-Home";

//Define the Device Names
char deviceName_1[] = "Light1";  // First light/switch
char deviceName_2[] = "Light2";  // Second light/switch
char deviceName_3[] = "Fan";     // Fan

//Update the HEX code of IR Remote buttons 0xHEX_CODE
#define IRButton1   0x1FEC13E  // Button for Light 1

const char *service_name = "NAF-TECH";
const char *pop = "NAF12321";

// Define the Chip Id
uint32_t espChipId = 5;

// Define the GPIO connected with Relays and switches
static uint8_t RelayPin1 = 23;  //D23 - Light 1
static uint8_t RelayPin2 = 22;  //D22 - Light 2

static uint8_t SwitchPin1 = 13;  //D13 - Switch for Light 1
static uint8_t SwitchPin2 = 12;  //D12 - Switch for Light 2

static uint8_t FanRelay1 = 21;  //D18 - Fan Relay 1
static uint8_t FanRelay2 = 19;  //D5  - Fan Relay 2
static uint8_t FanRelay3 = 18;  //D25 - Fan Relay 3

static uint8_t FanSwitchUp = 33;    //D33 - Fan Speed Increase
static uint8_t FanSwitchDown = 32;  //D32 - Fan Speed Decrease

static uint8_t gpio_reset = 0;    //Press BOOT to reset WiFi Details
static uint8_t wifiLed = 2;       //D2  - WiFi LED
static uint8_t IRRECV_PIN = 35;   //D35 - IR receiver pin
static uint8_t DHTPIN = 16;       //RX2 pin connected with DHT

#define DHTTYPE DHT11  // DHT 11
int currSpeed = 0;

// Relay State
bool toggleState_1 = LOW;  //State for Light 1
bool toggleState_2 = LOW;  //State for Light 2
bool toggleState_3 = LOW;  //State for Fan

float temperature1 = 0;
float humidity1 = 0;
int wifiFlag = 0;

IRrecv irrecv(IRRECV_PIN);
decode_results results;
DHT dht(DHTPIN, DHTTYPE);

ButtonConfig config1;
AceButton button1(&config1);
ButtonConfig config2;
AceButton button2(&config2);
ButtonConfig config3;
AceButton button3(&config3);
ButtonConfig config4;
AceButton button4(&config4);

void handleEvent1(AceButton*, uint8_t, uint8_t);
void handleEvent2(AceButton*, uint8_t, uint8_t);
void handleEvent3(AceButton*, uint8_t, uint8_t);
void handleEvent4(AceButton*, uint8_t, uint8_t);

//The framework provides some standard device types like switch, lightbulb, fan, temperature sensor.
static Switch my_light1(deviceName_1, &RelayPin1);
static Switch my_light2(deviceName_2, &RelayPin2);
static Fan my_fan(deviceName_3);
static TemperatureSensor temperature("Temperature");
static TemperatureSensor humidity("Humidity");

void sysProvEvent(arduino_event_t *sys_event) {
  switch (sys_event->event_id) {
    case ARDUINO_EVENT_PROV_START:
#if CONFIG_IDF_TARGET_ESP32
      Serial.printf("\nProvisioning Started with name \"%s\" and PoP \"%s\" on BLE\n", service_name, pop);
      printQR(service_name, pop, "ble");
#else
      Serial.printf("\nProvisioning Started with name \"%s\" and PoP \"%s\" on SoftAP\n", service_name, pop);
      printQR(service_name, pop, "softap");
#endif
      break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      Serial.printf("\nConnected to Wi-Fi!\n");
      digitalWrite(wifiLed, true);
      break;
  }
}

void write_callback(Device *device, Param *param, const param_val_t val, void *priv_data, write_ctx_t *ctx) {
  const char *device_name = device->getDeviceName();
  const char *param_name = param->getParamName();

  if (strcmp(device_name, deviceName_1) == 0) {
    if (strcmp(param_name, "Power") == 0) {
      toggleState_1 = val.val.b;
      digitalWrite(RelayPin1, !toggleState_1);
      pref.putBool("Relay1", toggleState_1);
      param->updateAndReport(val);
    }
  } else if (strcmp(device_name, deviceName_2) == 0) {
    if (strcmp(param_name, "Power") == 0) {
      toggleState_2 = val.val.b;
      digitalWrite(RelayPin2, !toggleState_2);
      pref.putBool("Relay2", toggleState_2);
      param->updateAndReport(val);
    }
  } else if (strcmp(device_name, deviceName_3) == 0) {
    if (strcmp(param_name, "Power") == 0) {
      toggleState_3 = val.val.b;
      if (toggleState_3 == 1) {
        fanSpeedControl(currSpeed);
      } else {
        fanSpeedControl(0);
      }
      pref.putBool("FanPower", toggleState_3);
      param->updateAndReport(val);
    } else if (strcmp(param_name, "MySpeed") == 0) {
      currSpeed = val.val.i;
      if (toggleState_3 == 1) {
        fanSpeedControl(currSpeed);
      }
      pref.putInt("FanSpeed", currSpeed);
      param->updateAndReport(val);
    }
  }
}

void readSensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();  // or dht.readTemperature(true) for Fahrenheit

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  } else {
    humidity1 = h;
    temperature1 = t;
  }
}

void sendSensor() {
  readSensor();
  temperature.updateAndReportParam("Temperature", temperature1);
  humidity.updateAndReportParam("Temperature", humidity1);
}

void irremote() {
  if (irrecv.decode(&results)) {
    switch (results.value) {
      case IRButton1:
        toggleState_1 = !toggleState_1;
        digitalWrite(RelayPin1, !toggleState_1);
        pref.putBool("Relay1", toggleState_1);
        my_light1.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_1);
        break;
      case IRButton2:
        toggleState_2 = !toggleState_2;
        digitalWrite(RelayPin2, !toggleState_2);
        pref.putBool("Relay2", toggleState_2);
        my_light2.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_2);
        break;
      case IRFanUp:
        if (currSpeed < 4) {
          if (toggleState_3 == 0) {
            toggleState_3 = 1;  // Turn on the fan if it's off
          }
          currSpeed++;
          fanSpeedControl(currSpeed);
          pref.putInt("FanSpeed", currSpeed);
          pref.putBool("FanPower", toggleState_3);
          my_fan.updateAndReportParam("MySpeed", currSpeed);
          my_fan.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_3);
        }
        break;
      case IRFanDown:
        if (currSpeed > 0) {
          currSpeed--;
          fanSpeedControl(currSpeed);
          pref.putInt("FanSpeed", currSpeed);
          if (currSpeed == 0) {
            toggleState_3 = 0;  // Turn off the fan if speed reaches 0
            pref.putBool("FanPower", toggleState_3);
            my_fan.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_3);
          }
          my_fan.updateAndReportParam("MySpeed", currSpeed);
        }
        break;
      case IRAllOn:
        toggleState_1 = 1;
        toggleState_2 = 1;
        toggleState_3 = 1;
        digitalWrite(RelayPin1, LOW);
        digitalWrite(RelayPin2, LOW);
        fanSpeedControl(currSpeed);
        pref.putBool("Relay1", toggleState_1);
        pref.putBool("Relay2", toggleState_2);
        pref.putBool("FanPower", toggleState_3);
        my_light1.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_1);
        my_light2.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_2);
        my_fan.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_3);
        break;
      case IRAllOff:
        toggleState_1 = 0;
        toggleState_2 = 0;
        toggleState_3 = 0;
        digitalWrite(RelayPin1, HIGH);
        digitalWrite(RelayPin2, HIGH);
        fanSpeedControl(0);
        pref.putBool("Relay1", toggleState_1);
        pref.putBool("Relay2", toggleState_2);
        pref.putBool("FanPower", toggleState_3);
        my_light1.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_1);
        my_light2.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_2);
        my_fan.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_3);
        break;
      default:
        break;
    }
    irrecv.resume();
  }
}

void allSwitchOff() {
  toggleState_1 = 0;
  toggleState_2 = 0;
  toggleState_3 = 0;
  digitalWrite(RelayPin1, HIGH);
  digitalWrite(RelayPin2, HIGH);
  fanSpeedControl(0);
  pref.putBool("Relay1", toggleState_1);
  pref.putBool("Relay2", toggleState_2);
  pref.putBool("FanPower", toggleState_3);
  my_light1.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_1);
  my_light2.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_2);
  my_fan.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_3);
}

void allSwitchOn() {
  toggleState_1 = 1;
  toggleState_2 = 1;
  toggleState_3 = 1;
  digitalWrite(RelayPin1, LOW);
  digitalWrite(RelayPin2, LOW);
  fanSpeedControl(currSpeed);
  pref.putBool("Relay1", toggleState_1);
  pref.putBool("Relay2", toggleState_2);
  pref.putBool("FanPower", toggleState_3);
  my_light1.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_1);
  my_light2.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_2);
  my_fan.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_3);
}

void getRelayState() {
  toggleState_1 = pref.getBool("Relay1", 0);
  digitalWrite(RelayPin1, !toggleState_1);
  my_light1.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_1);
  delay(200);

  toggleState_2 = pref.getBool("Relay2", 0);
  digitalWrite(RelayPin2, !toggleState_2);
  my_light2.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_2);
  delay(200);

  currSpeed = pref.getInt("FanSpeed", 0);
  my_fan.updateAndReportParam("MySpeed", currSpeed);
  delay(200);

  toggleState_3 = pref.getBool("FanPower", 0);
  if (toggleState_3 == 1) {
    if (currSpeed == 0) {
      currSpeed++;
    }
    fanSpeedControl(currSpeed);
  }
  my_fan.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_3);
  delay(200);
}

void setup() {
  Serial.begin(115200);
  pref.begin("RelayState", false);

  pinMode(RelayPin1, OUTPUT);
  pinMode(RelayPin2, OUTPUT);
  pinMode(FanRelay1, OUTPUT);
  pinMode(FanRelay2, OUTPUT);
  pinMode(FanRelay3, OUTPUT);
  pinMode(wifiLed, OUTPUT);

  pinMode(SwitchPin1, INPUT_PULLUP);
  pinMode(SwitchPin2, INPUT_PULLUP);
  pinMode(FanSwitchUp, INPUT_PULLUP);
  pinMode(FanSwitchDown, INPUT_PULLUP);
  pinMode(gpio_reset, INPUT);

  digitalWrite(RelayPin1, !toggleState_1);
  digitalWrite(RelayPin2, !toggleState_2);
  digitalWrite(FanRelay1, HIGH);
  digitalWrite(FanRelay2, HIGH);
  digitalWrite(FanRelay3, HIGH);
  digitalWrite(wifiLed, LOW);

  config1.setEventHandler(button1Handler);
  config2.setEventHandler(button2Handler);
  config3.setEventHandler(button3Handler);
  config4.setEventHandler(button4Handler);

  button1.init(SwitchPin1);
  button2.init(SwitchPin2);
  button3.init(FanSwitchUp);
  button4.init(FanSwitchDown);

  irrecv.enableIRIn();
  dht.begin();

  Node my_node;
  my_node = RMaker.initNode(nodeName);

  my_light1.addCb(write_callback);
  my_light2.addCb(write_callback);
  my_fan.addCb(write_callback);

  Param speed("MySpeed", ESP_RMAKER_PARAM_RANGE, value(0), PROP_FLAG_READ | PROP_FLAG_WRITE);
  speed.addBounds(value(0), value(4), value(1));
  speed.addUIType(ESP_RMAKER_UI_SLIDER);
  my_fan.addParam(speed);

  my_node.addDevice(my_light1);
  my_node.addDevice(my_light2);
  my_node.addDevice(my_fan);
  my_node.addDevice(temperature);
  my_node.addDevice(humidity);

  Timer.setInterval(30000);

  RMaker.enableOTA(OTA_USING_PARAMS);
  RMaker.enableTZService();
  RMaker.enableSchedule();

  Serial.printf("\nStarting ESP-RainMaker\n");
  RMaker.start();

  WiFi.onEvent(sysProvEvent);

#if CONFIG_IDF_TARGET_ESP32
  WiFiProv.beginProvision(WIFI_PROV_SCHEME_BLE, WIFI_PROV_SCHEME_HANDLER_FREE_BTDM, WIFI_PROV_SECURITY_1, pop, service_name);
#else
  WiFiProv.beginProvision(WIFI_PROV_SCHEME_SOFTAP, WIFI_PROV_SCHEME_HANDLER_NONE, WIFI_PROV_SECURITY_1, pop, service_name);
#endif

  delay(200);
  getRelayState();
}

void loop() {
  if (digitalRead(gpio_reset) == LOW) {
    delay(100);
    int startTime = millis();
    while (digitalRead(gpio_reset) == LOW) delay(50);
    int endTime = millis();

    if ((endTime - startTime) > 10000) {
      RMakerFactoryReset(2);
    } else if ((endTime - startTime) > 3000) {
      RMakerWiFiReset(2);
    }
  }
  delay(100);

  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(wifiLed, false);
  } else {
    digitalWrite(wifiLed, true);
  }

  if (Timer.isReady()) {
    sendSensor();
    Timer.reset();
  }

  button1.check();
  button2.check();
  button3.check();
  button4.check();
  irremote();
}

void fanSpeedControl(int fanSpeed) {
  switch (fanSpeed) {
    case 0:
      digitalWrite(FanRelay1, HIGH);
      digitalWrite(FanRelay2, HIGH);
      digitalWrite(FanRelay3, HIGH);
      break;
    case 1:
      digitalWrite(FanRelay1, LOW);
      digitalWrite(FanRelay2, HIGH);
      digitalWrite(FanRelay3, HIGH);
      break;
    case 2:
      digitalWrite(FanRelay1, HIGH);
      digitalWrite(FanRelay2, LOW);
      digitalWrite(FanRelay3, HIGH);
      break;
    case 3:
      digitalWrite(FanRelay1, LOW);
      digitalWrite(FanRelay2, LOW);
      digitalWrite(FanRelay3, HIGH);
      break;
    case 4:
      digitalWrite(FanRelay1, HIGH);
      digitalWrite(FanRelay2, HIGH);
      digitalWrite(FanRelay3, LOW);
      break;
    default:
      break;
  }
}

void button1Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  switch (eventType) {
    case AceButton::kEventPressed:
      toggleState_1 = 1;
      digitalWrite(RelayPin1, LOW);
      pref.putBool("Relay1", toggleState_1);
      my_light1.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_1);
      break;
    case AceButton::kEventReleased:
      toggleState_1 = 0;
      digitalWrite(RelayPin1, HIGH);
      pref.putBool("Relay1", toggleState_1);
      my_light1.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_1);
      break;
  }
}

void button2Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  switch (eventType) {
    case AceButton::kEventPressed:
      toggleState_2 = 1;
      digitalWrite(RelayPin2, LOW);
      pref.putBool("Relay2", toggleState_2);
      my_light2.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_2);
      break;
    case AceButton::kEventReleased:
      toggleState_2 = 0;
      digitalWrite(RelayPin2, HIGH);
      pref.putBool("Relay2", toggleState_2);
      my_light2.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_2);
      break;
  }
}

void button3Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  if (eventType == AceButton::kEventPressed) {
    if (currSpeed < 4) {
      if (toggleState_3 == 0) {
        toggleState_3 = 1;  // Turn on the fan if it's off
      }
      currSpeed++;
      fanSpeedControl(currSpeed);
      pref.putInt("FanSpeed", currSpeed);
      pref.putBool("FanPower", toggleState_3);
      my_fan.updateAndReportParam("MySpeed", currSpeed);
      my_fan.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_3);
    }
  }
}

void button4Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  if (eventType == AceButton::kEventPressed) {
    if (currSpeed > 0) {
      currSpeed--;
      fanSpeedControl(currSpeed);
      pref.putInt("FanSpeed", currSpeed);
      if (currSpeed == 0) {
        toggleState_3 = 0;  // Turn off the fan if speed reaches 0
        pref.putBool("FanPower", toggleState_3);
        my_fan.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_3);
      }
      my_fan.updateAndReportParam("MySpeed", currSpeed);
    }
  }
}
#define IRButton2   0x1FECE31  // Button for Light 2
#define IRFanUp     0x1FE1CE3  // Fan Speed Up (Increase)
#define IRFanDown   0x1FE02FD  // Fan Speed Down (Decrease)
#define IRAllOn     0x1FE12ED  // Turn All Devices On
#define IRAllOff    0x1FE817E  // Turn All Devices Off

#include "RMaker.h"
#include "WiFi.h"
#include "WiFiProv.h"
#include <IRremote.h>
#include <DHT.h>
#include <SimpleTimer.h>
#include <Preferences.h>
#include <AceButton.h>
using namespace ace_button;

Preferences pref;
SimpleTimer Timer;
