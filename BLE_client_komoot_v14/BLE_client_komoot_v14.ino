/**
 * A BLE client example that is rich in capabilities.
 * There is a lot new capabilities implemented.
 * author unknown
 * updated by chegewara
 */
#include "symbols.h"
#include "BLEDevice.h"




#include "ic_nav_arrow_finish.h"
#include "ic_nav_arrow_fork_left.h"
#include "ic_nav_arrow_fork_right.h"
#include "ic_nav_arrow_goto_start.h"
#include "ic_nav_arrow_keep_going.h"
#include "ic_nav_arrow_keep_left.h"
#include "ic_nav_arrow_keep_right.h"
#include "ic_nav_arrow_start.h"
#include "ic_nav_arrow_turn_hard_left.h"
#include "ic_nav_arrow_turn_hard_right.h"
#include "ic_nav_arrow_turn_left.h"
#include "ic_nav_arrow_turn_right.h"
#include "ic_nav_arrow_uturn.h"
#include "ic_nav_outof_route.h"
#include "ic_nav_roundabout_ccw1_1.h"
#include "ic_nav_roundabout_ccw1_2.h"
#include "ic_nav_roundabout_ccw1_3.h"
#include "ic_nav_roundabout_ccw2_2.h"
#include "ic_nav_roundabout_ccw2_3.h"
#include "ic_nav_roundabout_ccw3_3.h"
#include "ic_nav_roundabout_cw1_1.h"
#include "ic_nav_roundabout_cw1_2.h"
#include "ic_nav_roundabout_cw1_3.h"
#include "ic_nav_roundabout_cw2_2.h"
#include "ic_nav_roundabout_cw2_3.h"
#include "ic_nav_roundabout_cw3_3.h"
#include "ic_nav_roundabout_exit_ccw.h"
#include "ic_nav_roundabout_exit_cw.h"
#include "ic_nav_roundabout_fallback.h"








#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library
TFT_eSPI tft = TFT_eSPI();       // Invoke custom library
#define TFT_GREY 0x5AEB
#define lightblue 0x01E9
#define darkred 0xA041
#include "Orbitron_Medium_20.h"
#define blue 0x5D9B

int backlight[5] = {10,30,60,120,220};
byte b=4;
const int pwmFreq = 5000;
const int pwmResolution = 8;
const int pwmLedChannelTFT = 0;

const int battPin = 35; // A2=2 A6=34
unsigned int raw=0;
float volt=0.0;

// ESP32 ADV is a bit non-linear
const float vScale1 = 225.0; // divider for higher voltage range
const float vScale2 = 245.0; // divider for lower voltage range



// The remote service we wish to connect to.
static BLEUUID serviceUUID("71C1E128-D92F-4FA8-A2B2-0F171DB3436C");
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID("503DD605-9BCB-4F6E-B235-270A57483026");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;


std::string street = "Start";
std::string str1 = "";





static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    //Serial.println("Notify callback for characteristic (there is a new message!!) ");
    //Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    //Serial.print(" of data length ");
    //Serial.println(length);
    //Serial.print("pData data: ");
    //Serial.println((char*)pData);
}






class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};





bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());

    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");



    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");



    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");




    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }

    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
    return true;
}









/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class   MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks










void setup() {

  pinMode(0,INPUT_PULLUP);
  pinMode(35,INPUT);

  
  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(true);  // do not ask me. But without it the pictures are not displayed.
  //tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE,TFT_BLACK);  tft.setTextSize(1);
  ledcSetup(pwmLedChannelTFT, pwmFreq, pwmResolution);
  ledcAttachPin(TFT_BL, pwmLedChannelTFT);
  ledcWrite(pwmLedChannelTFT, backlight[b]);

  Serial.println("Screen black");    
  tft.fillRect(0,0,240,135,TFT_BLACK);

  Serial.println("Welcome message.");    
  tft.setTextSize(2);
  tft.setCursor(2, 40);
  tft.println("Komoot Reader V1.0");
  tft.setCursor(2, 100);
  tft.println("Starting..... ");

  
  
  
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
} // End of setup.









// This is the Arduino main loop function.
void loop() {

  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.

  if (doConnect == true) {

    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
      if (millis()/1000 > 20) {    // no connect after 0 sec. We switch everything off.
        Serial.println(millis()/1000);
        Serial.println("Switch screen off.");
        digitalWrite(4, LOW); // switch off screen completely
        tft.writecommand(TFT_DISPOFF);
        tft.writecommand(TFT_SLPIN);
        Serial.println("Switch ESP32 off.");
        esp_deep_sleep_start();  // gpo to sleep
        // will never go beyond those lines.... 
      }
    }

    
    doConnect = false;
  }




  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.

  
  if (connected) {
    String newValue = "Time since boot: " + String(millis()/1000);
    Serial.println("Update # :" + newValue);
    // Set the characteristic's value to be the array of bytes that is actually a string.
    // pRemoteCharacteristic->writeValue(newValue.c_str(), newValue.length());
    // Read the latest value of the characteristic.
    std::string value = pRemoteCharacteristic->readValue();
    //Identifier (UInt32)  -> 4 bytes
    //Direction Arrow (UInt8)  -> 1 bytes
    //Street (UTF8 string)  -> 4 bytes
    //Distance (UInt32)  -> 4 bytes
    tft.fillRect(0,0,240,135,TFT_BLACK);
    tft.setFreeFont(&Orbitron_Medium_20);
    tft.setTextSize(1);

    street = value.substr(9);
    //Serial.print ("Street: ");
    //Serial.println (street.c_str());

    Serial.print (value[0], HEX);
    Serial.print (value[1], HEX);
    Serial.print (value[2], HEX);
    Serial.print (value[3], HEX);

    //tft.setCursor(2, 30);
    //tft.setTextSize(1);
    //tft.println(value[4],DEC);
    
    //Serial.println ("====================");
    double dist = int(value[5])+int(value[6])*256+int(value[7])*65536;
    
    if (dist > 1000) { 
      dist = dist / 1000;
    } else { 
      if (dist > 200) { 
        dist = int(dist / 50) * 50;  // round down
      } else // 10m steps
        dist = int(dist / 10) * 10;  // round down
    }
    
    Serial.println (value[5],DEC);
    tft.setCursor(25, 50);
    tft.setTextSize(2);
    //tft.println(value[5],DEC);
    tft.println(int(dist));

    //Serial.println (value[6],DEC);
    //Serial.println (value[7,DEC]);
    //Serial.println (value[8,DEC]);
    //Serial.println (""); 
    //Serial.println ("=== ");
    Serial.println(value.substr(9).c_str());

    tft.setCursor(2, 95);
    tft.setTextSize(1);
    tft.println(value.substr(9).c_str());
    //tft.pushImage(2, 31,  48, 48, ic_nav_arrow_finish_bits);

    const int sym_num = int(value[4]);
    Serial.println ("---");
    Serial.println (sym_num);
    
    switch (sym_num) {
        case 1:
          tft.pushImage(180, 10,  60, 60, ic_nav_arrow_keep_going);
          break;
        case 2:
          tft.pushImage(180, 10,  60, 60, ic_nav_arrow_start);
          break;
        case 3:
          tft.pushImage(180, 10,  60, 60, ic_nav_arrow_finish);
          break;
        case 4:
          tft.pushImage(180, 10,  60, 60, ic_nav_arrow_keep_left);
          break;
        case 5:
          tft.pushImage(180, 10,  60, 60, ic_nav_arrow_turn_left);
          break;
        case 6:
          tft.pushImage(180, 10,  60, 60, ic_nav_arrow_turn_hard_left);
          break;
        case 7:
          tft.pushImage(180, 10,  60, 60, ic_nav_arrow_turn_hard_right);
          break;
        case 8:
          tft.pushImage(180, 10,  60, 60, ic_nav_arrow_turn_right);
          break;
        case 9:
          tft.pushImage(180, 10,  60, 60, ic_nav_arrow_keep_right);
          break;
        case 10:
          tft.pushImage(180, 10,  60, 60, ic_nav_arrow_fork_right);
          break;
        case 11:
          tft.pushImage(180, 10,  60, 60, ic_nav_arrow_fork_left);
          break;
        case 12:
          tft.pushImage(180, 10,  60, 60, ic_nav_arrow_uturn);
          break;
        case 13:
          tft.pushImage(180, 10,  60, 60, ic_nav_outof_route);
          break;
        case 14:
          tft.pushImage(180, 10,  60, 60, ic_nav_outof_route);
          break;
        case 15:
          tft.pushImage(180, 10,  60, 60, ic_nav_roundabout_exit_cw);
          break;
        case 16:
          tft.pushImage(180, 10,  60, 60, ic_nav_roundabout_exit_ccw);
          break;
        case 17:
          tft.pushImage(180, 10,  60, 60, ic_nav_roundabout_ccw1_1);
          break;
        case 18:
          tft.pushImage(180, 10,  60, 60, ic_nav_roundabout_ccw1_2);
          break;
        case 19:
          tft.pushImage(180, 10,  60, 60, ic_nav_roundabout_ccw1_3);
          break;
        case 20:
          tft.pushImage(180, 10,  60, 60, ic_nav_roundabout_ccw2_2);
          break;
        case 21:
          tft.pushImage(180, 10,  60, 60, ic_nav_roundabout_ccw2_3);
          break;
        case 22:
          tft.pushImage(180, 10,  60, 60, ic_nav_roundabout_ccw3_3);
          break;
        case 23:
          tft.pushImage(180, 10,  60, 60, ic_nav_roundabout_cw1_1);
          break;
        case 24:
          tft.pushImage(180, 10,  60, 60, ic_nav_roundabout_cw1_2);
          break;
        case 25:
          tft.pushImage(180, 10,  60, 60, ic_nav_roundabout_cw1_3);
          break;
        case 26:
          tft.pushImage(180, 10,  60, 60, ic_nav_roundabout_cw2_2);
          break;
        case 27:
          tft.pushImage(180, 10,  60, 60, ic_nav_roundabout_cw2_3);
          break;
        case 28:
          tft.pushImage(180, 10,  60, 60, ic_nav_roundabout_cw3_3);
          break;
        case 29:
          tft.pushImage(180, 10,  60, 60, ic_nav_roundabout_fallback);
          break;
        case 30:
          tft.pushImage(180, 10,  60, 60,  ic_nav_outof_route);
          break;
        
        default:
          tft.pushImage(180, 10,  60, 60, ic_nav_arrow_goto_start);
        break;
    }

    
    
    
    Serial.println ("---");

    
    //Serial.println(value.substr(5, 8).c_str());
    Serial.println ("====================");
    Serial.println (" ");
    //Serial.print (value[9]);
    //Serial.print (value[10]);

    Serial.println ("");    
    Serial.println ("=== ");    

    raw  = analogRead(battPin);
    volt = raw / vScale2;
    double volt1= int(volt * 10);
    volt1 = volt1 / 10;
    Serial.println(raw);
    Serial.println(volt);
    Serial.println(volt1, 1);    
    Serial.println("................");

    tft.setCursor(155, 135);
    tft.setTextSize(1);
    tft.println(volt1, 1);
    tft.setCursor(210, 135);
    tft.println('V');

   
    if (volt < 3.1) { //sleep below 3.1 V
      Serial.print("Switching off now... Low battery");
      delay(999);
      //esp_wifi_stop();
      Serial.println(millis()/1000);
      Serial.println("Switch screen off.");
      digitalWrite(4, LOW); // switch off screen completely
      tft.writecommand(TFT_DISPOFF);
      tft.writecommand(TFT_SLPIN);
      Serial.println("Switch ESP32 off.");
      esp_deep_sleep_start();  // gpo to sleep
      // will never go beyond those lines.... 
    }




    
    
  } else {
      Serial.print("Not connected.");
      if (millis()/1000 > 20) {    // no connect after 0 sec. We switch everything off.
        Serial.println(millis()/1000);

        tft.setTextSize(2);
        tft.setCursor(2, 40);
        tft.println("Switching off...");
        tft.setCursor(2, 100);
        tft.println("See you soon. ");
        delay(5000);

        
        Serial.println("Switch screen off.");
        digitalWrite(4, LOW); // switch off screen completely
        tft.writecommand(TFT_DISPOFF);
        tft.writecommand(TFT_SLPIN);
        Serial.println("Switch ESP32 off.");
        esp_deep_sleep_start();  // gpo to sleep
        // will never go beyond those lines.... 
      }


      

      if(doScan){
        Serial.print('################# new scan required or go to sleep ########### ' );
        BLEDevice::getScan()->start(0);  // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
      }
  }
  
  delay(1500); // Delay a second between loops.
} // End of loop
