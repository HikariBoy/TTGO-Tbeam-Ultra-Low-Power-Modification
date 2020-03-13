/*
  Evaluation of Current consumption BEFORE and AFTER TTGO TBeam PCB modification
  Modifications undertaken by Adrian Keating AT The University of Westerna Australia 
  March 2020
*/
#include <string.h>
#include <ctype.h>
#include <WiFi.h>
#include <LoRa.h>
#include<esp_deep_sleep.h>

HardwareSerial ss(1);
// defines the pins used by the transceiver module
#define SS 18   // GPIO18 - SX1278's CS   - LoRa radio chip select
#define RST 23   // GPIO23 - SX1278's RESET   - LoRa radio reset
#define DI0 26   // GPIO26 - SX1278's IRQ   - IRQ pin

#define GPS_TX 12
#define GPS_RX 15
#define BUILTIN_LED 14
#define ME6211_EN 21
#define READ_DELAY 10000
#define SET_FLASHON true   // Use this flag if you are unsure the TTGO is responding during wake periods
#define STARUP_DELAY_SEC 6
#define SET_RTC_OFF true
#define SET_WIFI_OFF true
#define SET_BLUETOOTH_OFF true
#define SET_GPS_OFF true
#define SET_LORA_OFF true
#define SET_ALLGPIO_OFF true
#define SET_ME6211_GPIO_CNTRL true

#define BAND 433E6   // EU   -Australia [EVAL only]

// stores the data on the RTC memory so that it will not be deleted during the deep sleep
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR int ResetCounter = 0;   // count loops

// deep sleep
#define uS_TO_S_FACTOR 1000000  // conversion factor for micro seconds to seconds 
#define TIME_TO_SLEEP  10        // time ESP32 will go to sleep (in seconds)   - 99 for (about) 1% duty cycle  


// see http://forum.espruino.com/conversations/332295/
uint8_t GPS_CFG_ANT_default[] = {0xB5, 0x62,  0x06,  0x13, 0x04,  0x00, 0x1B,  0x00, 0x8B, 0xA9, 0x6C, 0xF8};
uint8_t GPS_CFG_ANT_NO_SUPPLY[] = {0xB5,  0x62,  0x06,  0x13, 0x04,  0x00, 0x1A,  0x00, 0x8B, 0xA9, 0x6B, 0xF4};
uint8_t GPS_CFG_ANT_POLL[] = {0xB5, 0x62, 0x06, 0x13, 0x00, 0x00,  0x19,  0x51};
uint8_t CFG_PM2_POLL[] = {0xB5, 0x62, 0x06,  0x3B,  0x00,  0x00,  0x41,  0xC9};
// after changing CFG-PM2 you must call the following
uint8_t Power_save[] =  {0xB5, 0x62, 0x06, 0x11, 0x02, 0x00, 0x08, 0x01, 0x22, 0x92};
uint8_t GPSfirmwareID[] = {0xB5, 0x62, 0x0A, 0x04, 0x00, 0x00, 0x0E, 0x34};
// GPSoff  TURNS off the SERIAL STREAMING OF gps DATA
uint8_t GPSoff[] = {0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, 0x00, 0x00, 0x08, 0x00, 0x16, 0x74};
// GPSoffall shoudl turn off ALL but does not
uint8_t GPSoffall[] = {0xB5, 0x62, 0x02, 0x41, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x4D, 0x3B};
uint8_t GPSon[] = {0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, 0x00, 0x00, 0x09, 0x00, 0x17, 0x76};
// see http://forum.espruino.com/conversations/332295/

void ShutDownGPS() {
  ss.begin(9600, SERIAL_8N1, GPS_TX, GPS_RX);
  delay(200);
  ss.write(GPS_CFG_ANT_NO_SUPPLY, sizeof(GPS_CFG_ANT_NO_SUPPLY)); delay(50);
  ss.write(CFG_PM2_POLL, sizeof(CFG_PM2_POLL)); delay(50);
  ss.write(Power_save, sizeof(Power_save)); delay(50);
  ss.write(GPSoff, sizeof(GPSoff)); delay(50);
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);       // Initialize LED pin
  digitalWrite(BUILTIN_LED, LOW);
  pinMode(ME6211_EN, OUTPUT);
  digitalWrite(ME6211_EN, HIGH);  // Enable power to GPS, LoRA and USB controller
  // LoRa transceiver module setup
  ResetCounter++;
  LoRa.setPins(SS, RST, DI0);   // overrides the default CS, reset, and IRQ pins used by the library
  // initializes the transceiver module with a specified frequency
  while (!LoRa.begin(BAND)) {   // LoRa.begin returns 1 on success, 0 on failure
    delay(50);
  }
  delay(READ_DELAY);//10
  if (SET_GPS_OFF) ShutDownGPS();
  delay(READ_DELAY); //20
  if (SET_WIFI_OFF) {
    WiFi.mode(WIFI_OFF);
    delay(50);
  }
  delay(READ_DELAY); //30
  if (SET_BLUETOOTH_OFF) {
    btStop();
    delay(50);
  }
  delay(READ_DELAY);//40
  if (SET_LORA_OFF) {
    LoRa.sleep();
    delay(50);
  }
  delay(READ_DELAY);//50
  if (SET_ME6211_GPIO_CNTRL) digitalWrite(ME6211_EN, LOW);
  delay(READ_DELAY);//60
  if (SET_ME6211_GPIO_CNTRL) digitalWrite(ME6211_EN, HIGH);
  delay(100);
  if (SET_GPS_OFF) ShutDownGPS();  // You must shut down the GPS and LoRa again after cycling  ME6211/EN
  if (SET_LORA_OFF) {
    LoRa.sleep();
    delay(50);
  }

  if (SET_ALLGPIO_OFF) {
    pinMode(5, OUTPUT); digitalWrite(5 , LOW);
    pinMode(19, OUTPUT); digitalWrite(19, LOW);
    pinMode(27, OUTPUT); digitalWrite(27 , LOW);
    pinMode(33, OUTPUT); digitalWrite(33 , LOW); // HPDIO1=IO33
    pinMode(32, OUTPUT); digitalWrite(32, LOW); //  HPDIO2=IO32
  }
  delay(READ_DELAY); //70

  if (SET_FLASHON)digitalWrite(BUILTIN_LED, HIGH);
  delay(READ_DELAY);//80
  if (SET_FLASHON)   digitalWrite(BUILTIN_LED, LOW);
  delay(READ_DELAY); //90

  // deep sleep
  // Recommended not to force digitalWrite(ME6211_EN, LOW) before shutdown unless you need to for low power operating when connected to solar recharge
  /*if (SET_ME6211_GPIO_CNTRL) {
    delay(30000);  // second delay to allow upload
    digitalWrite(ME6211_EN, LOW);
    delay(50);} // Kills power to GPS, LoRA and USB controller
    }
  */
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  delay(READ_DELAY); //100
  // See details on HIBERNATION, https://esp32.com/viewtopic.php?t=3083
  if (SET_RTC_OFF) {
    esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);  //ESP_PD_DOMAIN_RTC_FAST_MEM=2
    esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);  //ESP_PD_DOMAIN_RTC_SLOW_MEM=1
    esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);  //ESP_PD_DOMAIN_RTC_PERIPH=0
    esp_deep_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);
    esp_deep_sleep_pd_config(ESP_PD_DOMAIN_MAX, ESP_PD_OPTION_OFF);
  }
  delay(READ_DELAY);//110
  esp_deep_sleep_start();   // enters deep sleep with the configured wakeup options
}

void loop() {
}




