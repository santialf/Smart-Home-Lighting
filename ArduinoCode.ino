#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
// Provide the token generation process info.
#include <addons/TokenHelper.h>
// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>
/* 1. Define the WiFi credentials */
#define WIFI_SSID "NOS-50E6"
#define WIFI_PASSWORD "A3JXKPC2"
/* 2. Define the API Key */
#define API_KEY "AIzaSyA4O7vDE2Acqfeu57iXKWP55SmzsAMR83w"
/* 3. Define the RTDB URL */
#define DATABASE_URL "registration-f031a-default-rtdb.europe-west1.firebasedatabase.app" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "zemor.morgado@gmail.com"
#define USER_PASSWORD "1234567"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int R1counter = 0;
int R2counter = 0;

int photoPin1 = 34;
const byte led_gpioR1 = 25;//23 25
const byte led_gpioG1 = 32;//22 32
const byte led_gpioB1 = 33;//21 33

int photoPin2 = 35;
const byte led_gpioR2 = 23;//23 25
const byte led_gpioG2 = 22;//22 32
const byte led_gpioB2 = 21;//21 33

#include <Arduino.h>
#include <HC_SR04.h>
// Sensor / Arduino Pin mapping
//             |  Echo  |  Trigger
// Sensor 1:   |   2    |     8     OUTSIDE/LEFT BIG BOX
// Sensor 2:   |   3    |     8     INSIDE/LEFT SMALL BOX
// Sensor 3:   |   4    |     8     OUTSIDE/RIGHT SMALL BOX
// Sensor 4:   |   5    |    11     INSIDE/RIGHT BIG BOX

// Sensor / Arduino Pin mapping
//             |  Echo  |  Trigger
// Sensor 1:   |   4  |    15
// Sensor 2:   |   16 |    15
// Sensor 3:   |   17 |    15
// Sensor 4:   |   2  |    5
HC_SR04_BASE *Slaves[] = { new HC_SR04<13>(), new HC_SR04<14>(), new HC_SR04<2>(5) };
HC_SR04<4> sonicMaster(15, Slaves, 3);


void setup()
{
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");
  unsigned long ms = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  // Limit the size of response payload to be collected in FirebaseData
  fbdo.setResponseSize(2048);

  Firebase.begin(&config, &auth);

  // Comment or pass false value when WiFi reconnection will control by your code or third party library
  Firebase.reconnectWiFi(true);

  Firebase.setDoubleDigits(5);

  config.timeout.serverResponse = 10 * 1000;

  ledcAttachPin(led_gpioR1, 0); // assign a led pins to a channel
  ledcAttachPin(led_gpioG1, 1); // assign a led pins to a channel
  ledcAttachPin(led_gpioB1, 2);

  ledcAttachPin(led_gpioR2, 3); // assign a led pins to a channel
  ledcAttachPin(led_gpioG2, 4); // assign a led pins to a channel
  ledcAttachPin(led_gpioB2, 5);

  ledcSetup(0, 4000, 8); // 12 kHz PWM, 8-bit resolution
  ledcSetup(1, 4000, 8); // 12 kHz PWM, 8-bit resolution
  ledcSetup(2, 4000, 8);

  ledcSetup(3, 4000, 8); // 12 kHz PWM, 8-bit resolution
  ledcSetup(4, 4000, 8); // 12 kHz PWM, 8-bit resolution
  ledcSetup(5, 4000, 8);

  // begin all sensors as asynchron, they can still be used synchron
  // in case one of echo pin doesn't suport interrupt the function returns false
  sonicMaster.beginAsync();
}
bool autmode;

//Room 1 variables
int outsider1 = 0;
int insider1 = 0;

int Red11 = 0;
int Green11 = 0;
int Blue11 = 0;

int Red12 = 0;
int Green12 = 0;
int Blue12 = 0;

int Red13 = 0;
int Green13 = 0;
int Blue13 = 0;

int Red14 = 0;
int Green14 = 0;
int Blue14 = 0;

int colorN1 = 0;
int tempcolor1 = 0;
int tempRed1 = 0;
int tempGreen1 = 0;
int tempBlue1 = 0;
int speed1;
bool switch1;
bool autoswitch1 = true;


//Room 2 variables
int outsider2 = 0;
int insider2 = 0;

int Red21 = 0;
int Green21 = 0;
int Blue21 = 0;

int Red22 = 0;
int Green22 = 0;
int Blue22 = 0;

int Red23 = 0;
int Green23 = 0;
int Blue23 = 0;

int Red24 = 0;
int Green24 = 0;
int Blue24 = 0;

int colorN2 = 0;
int tempcolor2 = 0;
int tempRed2 = 0;
int tempGreen2 = 0;
int tempBlue2 = 0;
int speed2;
bool switch2;
bool autoswitch2 = true;

// main loop function
void loop()
{
  sonicMaster.startAsync(200000);
  while(!sonicMaster.isFinished())
  {
    // Do something usefulle while measureing
    // all echo pins which doesnt support interrupt will have a 0 result
    // Firebase.ready() should be called repeatedly to handle authentication tasks.

  
    
 
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();

    FirebaseJson BedRoom;
    FirebaseJson LivingRoom;

    Firebase.RTDB.getBool(&fbdo, F("/Auto-Mode"), &autmode);

    if (autmode){
      autoswitch1 = true;
      autoswitch2 = true;
    }
    else{
      autoswitch1 = false;
      autoswitch2 = false;
    }

    Firebase.RTDB.getJSON(&fbdo, "/BedRoom", &BedRoom);
    Firebase.RTDB.getJSON(&fbdo, "/LivingRoom", &LivingRoom);

    FirebaseJsonData result;

    //BEDROOM VARIABLES
    BedRoom.get(result, "Red1");
    if (result.success) Red21 = result.to<int>();

    BedRoom.get(result, "Red2"); 
    if (result.success) Red22 = result.to<int>();

    BedRoom.get(result, "Red3");
    if (result.success) Red23 = result.to<int>();

    BedRoom.get(result, "Red4");
    if (result.success) Red24 = result.to<int>();

    BedRoom.get(result, "Green1");
    if (result.success) Green21 = result.to<int>();

    BedRoom.get(result, "Green2");
    if (result.success) Green22 = result.to<int>();

    BedRoom.get(result, "Green3");
    if (result.success) Green23 = result.to<int>();

    BedRoom.get(result, "Green4");
    if (result.success) Green24 = result.to<int>();
 
    BedRoom.get(result, "Blue1");
    if (result.success) Blue21 = result.to<int>();
 
    BedRoom.get(result, "Blue2");
    if (result.success) Blue22 = result.to<int>();

    BedRoom.get(result, "Blue3");
    if (result.success) Blue23 = result.to<int>();
 
    BedRoom.get(result, "Blue4");
    if (result.success) Blue24 = result.to<int>();

    BedRoom.get(result, "ColorCount");
    if (result.success) colorN2 = result.to<int>();
   
    BedRoom.get(result, "DynamicSpeed");
    if (result.success) speed2 = result.to<int>();
 
    BedRoom.get(result, "Switch");
    if (result.success) switch2 = result.to<bool>();

    //LIVING ROOM VARIABLES
    LivingRoom.get(result, "Red1");
    if (result.success) Red11 = result.to<int>();

    LivingRoom.get(result, "Red2"); 
    if (result.success) Red12 = result.to<int>();

    LivingRoom.get(result, "Red3");
    if (result.success) Red13 = result.to<int>();

    LivingRoom.get(result, "Red4");
    if (result.success) Red14 = result.to<int>();

    LivingRoom.get(result, "Green1");
    if (result.success) Green11 = result.to<int>();

    LivingRoom.get(result, "Green2");
    if (result.success) Green12 = result.to<int>();

    LivingRoom.get(result, "Green3");
    if (result.success) Green13 = result.to<int>();

    LivingRoom.get(result, "Green4");
    if (result.success) Green14 = result.to<int>();
 
    LivingRoom.get(result, "Blue1");
    if (result.success) Blue11 = result.to<int>();
 
    LivingRoom.get(result, "Blue2");
    if (result.success) Blue12 = result.to<int>();

    LivingRoom.get(result, "Blue3");
    if (result.success) Blue13 = result.to<int>();
 
    LivingRoom.get(result, "Blue4");
    if (result.success) Blue14 = result.to<int>();

    LivingRoom.get(result, "ColorCount");
    if (result.success) colorN1 = result.to<int>();
   
    LivingRoom.get(result, "DynamicSpeed");
    if (result.success) speed1 = result.to<int>();
 
    LivingRoom.get(result, "Switch");
    if (result.success) switch1 = result.to<bool>();
  }
  }
  if (!autmode){
//ROOM 1 COLOR SHIFT CODE

if ((tempRed1 != Red11 || tempGreen1 != Green11 || tempBlue1 != Blue11) && tempcolor1 == 0){
      if (tempRed1 < Red11 && (tempRed1 + speed1) < Red11) {tempRed1 += speed1;}
      if (tempRed1 < Red11 && (tempRed1 + speed1) > Red11) {tempRed1 = Red11;}

      if (tempRed1 > Red11 && (tempRed1 - speed1) > Red11) {tempRed1 -= speed1;}
      if (tempRed1 > Red11 && (tempRed1 - speed1) < Red11) {tempRed1 = Red11;}


      if (tempGreen1 < Green11 && (tempGreen1 + speed1) < Green11) {tempGreen1 += speed1;}
      if (tempGreen1 < Green11 && (tempGreen1 + speed1) > Green11) {tempGreen1 = Green11;}

      if (tempGreen1 > Green11 && (tempGreen1 - speed1) > Green11) {tempGreen1 -= speed1;}
      if (tempGreen1 > Green11 && (tempGreen1 - speed1) < Green11) {tempGreen1 = Green11;}


      if (tempBlue1 < Blue11 && (tempBlue1 + speed1) < Blue11) {tempBlue1 += speed1;}
      if (tempBlue1 < Blue11 && (tempBlue1 + speed1) > Blue11) {tempBlue1 = Blue11;}

      if (tempBlue1 > Blue11 && (tempBlue1 - speed1) > Blue11) {tempBlue1 -= speed1;}
      if (tempBlue1 > Blue11 && (tempBlue1 - speed1) < Blue11) {tempBlue1 = Blue11;}
    }
    else if ((tempRed1 == Red11 || tempGreen1 == Green11 || tempBlue1 == Blue11) && tempcolor1 == 0 && tempcolor1 < colorN1){
      tempcolor1++;
    }
    else if ((tempRed1 != Red12 || tempGreen1 != Green12 || tempBlue1 != Blue12) && tempcolor1 == 1){
      if (tempRed1 < Red12 && (tempRed1 + speed1) < Red12) {tempRed1 += speed1;}
      if (tempRed1 < Red12 && (tempRed1 + speed1) > Red12) {tempRed1 = Red12;}

      if (tempRed1 > Red12 && (tempRed1 - speed1) > Red12) {tempRed1 -= speed1;}
      if (tempRed1 > Red12 && (tempRed1 - speed1) < Red12) {tempRed1 = Red12;}


      if (tempGreen1 < Green12 && (tempGreen1 + speed1) < Green12) {tempGreen1 += speed1;}
      if (tempGreen1 < Green12 && (tempGreen1 + speed1) > Green12) {tempGreen1 = Green12;}

      if (tempGreen1 > Green12 && (tempGreen1 - speed1) > Green12) {tempGreen1 -= speed1;}
      if (tempGreen1 > Green12 && (tempGreen1 - speed1) < Green12) {tempGreen1 = Green12;}


      if (tempBlue1 < Blue12 && (tempBlue1 + speed1) < Blue12) {tempBlue1 += speed1;}
      if (tempBlue1 < Blue12 && (tempBlue1 + speed1) > Blue12) {tempBlue1 = Blue12;}

      if (tempBlue1 > Blue12 && (tempBlue1 - speed1) > Blue12) {tempBlue1 -= speed1;}
      if (tempBlue1 > Blue12 && (tempBlue1 - speed1) < Blue12) {tempBlue1 = Blue12;}
    }
    else if ((tempRed1 == Red12 || tempGreen1 == Green12 || tempBlue1 == Blue12) && tempcolor1 == 1 && tempcolor1 < colorN1){
      tempcolor1++;
    }
    else if ((tempRed1 != Red13 || tempGreen1 != Green13 || tempBlue1 != Blue13) && tempcolor1 == 2){
      if (tempRed1 < Red13 && (tempRed1 + speed1) < Red13) {tempRed1 += speed1;}
      if (tempRed1 < Red13 && (tempRed1 + speed1) > Red13) {tempRed1 = Red13;}

      if (tempRed1 > Red13 && (tempRed1 - speed1) > Red13) {tempRed1 -= speed1;}
      if (tempRed1 > Red13 && (tempRed1 - speed1) < Red13) {tempRed1 = Red13;}


      if (tempGreen1 < Green13 && (tempGreen1 + speed1) < Green13) {tempGreen1 += speed1;}
      if (tempGreen1 < Green13 && (tempGreen1 + speed1) > Green13) {tempGreen1 = Green13;}

      if (tempGreen1 > Green13 && (tempGreen1 - speed1) > Green13) {tempGreen1 -= speed1;}
      if (tempGreen1 > Green13 && (tempGreen1 - speed1) < Green13) {tempGreen1 = Green13;}


      if (tempBlue1 < Blue13 && (tempBlue1 + speed1) < Blue13) {tempBlue1 += speed1;}
      if (tempBlue1 < Blue13 && (tempBlue1 + speed1) > Blue13) {tempBlue1 = Blue13;}

      if (tempBlue1 > Blue13 && (tempBlue1 - speed1) > Blue13) {tempBlue1 -= speed1;}
      if (tempBlue1 > Blue13 && (tempBlue1 - speed1) < Blue13) {tempBlue1 = Blue13;}
    }
    else if ((tempRed1 == Red13 || tempGreen1 == Green13 || tempBlue1 == Blue13) && tempcolor1 == 2 && tempcolor1 < colorN1){
      tempcolor1++;
    }
    else if ((tempRed1 != Red14 || tempGreen1 != Green14 || tempBlue1 != Blue14) && tempcolor1 == 3){
      if (tempRed1 < Red14 && (tempRed1 + speed1) < Red14) {tempRed1 += speed1;}
      if (tempRed1 < Red14 && (tempRed1 + speed1) > Red14) {tempRed1 = Red14;}

      if (tempRed1 > Red14 && (tempRed1 - speed1) > Red14) {tempRed1 -= speed1;}
      if (tempRed1 > Red14 && (tempRed1 - speed1) < Red14) {tempRed1 = Red14;}


      if (tempGreen1 < Green14 && (tempGreen1 + speed1) < Green14) {tempGreen1 += speed1;}
      if (tempGreen1 < Green14 && (tempGreen1 + speed1) > Green14) {tempGreen1 = Green14;}

      if (tempGreen1 > Green14 && (tempGreen1 - speed1) > Green14) {tempGreen1 -= speed1;}
      if (tempGreen1 > Green14 && (tempGreen1 - speed1) < Green14) {tempGreen1 = Green14;}


      if (tempBlue1 < Blue14 && (tempBlue1 + speed1) < Blue14) {tempBlue1 += speed1;}
      if (tempBlue1 < Blue14 && (tempBlue1 + speed1) > Blue14) {tempBlue1 = Blue14;}

      if (tempBlue1 > Blue14 && (tempBlue1 - speed1) > Blue14) {tempBlue1 -= speed1;}
      if (tempBlue1 > Blue14 && (tempBlue1 - speed1) < Blue14) {tempBlue1 = Blue14;}
    }
    else if ((tempRed1 == Red14 || tempGreen1 == Green14 || tempBlue1 == Blue14) && tempcolor1 == 3 && tempcolor1 < colorN1){
      tempcolor1++;
    }
//ROOM 2 COLOR SHIFT CODE
    if ((tempRed2 != Red21 || tempGreen2 != Green21 || tempBlue2 != Blue21) && tempcolor2 == 0){
      if (tempRed2 < Red21 && (tempRed2 + speed2) < Red21) {tempRed2 += speed2;}
      if (tempRed2 < Red21 && (tempRed2 + speed2) > Red21) {tempRed2 = Red21;}

      if (tempRed2 > Red21 && (tempRed2 - speed2) > Red21) {tempRed2 -= speed2;}
      if (tempRed2 > Red21 && (tempRed2 - speed2) < Red21) {tempRed2 = Red21;}


      if (tempGreen2 < Green21 && (tempGreen2 + speed2) < Green21) {tempGreen2 += speed2;}
      if (tempGreen2 < Green21 && (tempGreen2 + speed2) > Green21) {tempGreen2 = Green21;}

      if (tempGreen2 > Green21 && (tempGreen2 - speed2) > Green21) {tempGreen2 -= speed2;}
      if (tempGreen2 > Green21 && (tempGreen2 - speed2) < Green21) {tempGreen2 = Green21;}


      if (tempBlue2 < Blue21 && (tempBlue2 + speed2) < Blue21) {tempBlue2 += speed2;}
      if (tempBlue2 < Blue21 && (tempBlue2 + speed2) > Blue21) {tempBlue2 = Blue21;}

      if (tempBlue2 > Blue21 && (tempBlue2 - speed2) > Blue21) {tempBlue2 -= speed2;}
      if (tempBlue2 > Blue21 && (tempBlue2 - speed2) < Blue21) {tempBlue2 = Blue21;}
    }
    else if ((tempRed2 == Red21 || tempGreen2 == Green21 || tempBlue2 == Blue21) && tempcolor2 == 0 && tempcolor2 < colorN2){
      tempcolor2++;
    }
    else if ((tempRed2 != Red22 || tempGreen2 != Green22 || tempBlue2 != Blue22) && tempcolor2 == 1){
      if (tempRed2 < Red22 && (tempRed2 + speed2) < Red22) {tempRed2 += speed2;}
      if (tempRed2 < Red22 && (tempRed2 + speed2) > Red22) {tempRed2 = Red22;}

      if (tempRed2 > Red22 && (tempRed2 - speed2) > Red22) {tempRed2 -= speed2;}
      if (tempRed2 > Red22 && (tempRed2 - speed2) < Red22) {tempRed2 = Red22;}


      if (tempGreen2 < Green22 && (tempGreen2 + speed2) < Green22) {tempGreen2 += speed2;}
      if (tempGreen2 < Green22 && (tempGreen2 + speed2) > Green22) {tempGreen2 = Green22;}

      if (tempGreen2 > Green22 && (tempGreen2 - speed2) > Green22) {tempGreen2 -= speed2;}
      if (tempGreen2 > Green22 && (tempGreen2 - speed2) < Green22) {tempGreen2 = Green22;}


      if (tempBlue2 < Blue22 && (tempBlue2 + speed2) < Blue22) {tempBlue2 += speed2;}
      if (tempBlue2 < Blue22 && (tempBlue2 + speed2) > Blue22) {tempBlue2 = Blue22;}

      if (tempBlue2 > Blue22 && (tempBlue2 - speed2) > Blue22) {tempBlue2 -= speed2;}
      if (tempBlue2 > Blue22 && (tempBlue2 - speed2) < Blue22) {tempBlue2 = Blue22;}
    }
    else if ((tempRed2 == Red22 || tempGreen2 == Green22 || tempBlue2 == Blue22) && tempcolor2 == 1 && tempcolor2 < colorN2){
      tempcolor2++;
    }
    else if ((tempRed2 != Red23 || tempGreen2 != Green23 || tempBlue2 != Blue23) && tempcolor2 == 2){
      if (tempRed2 < Red23 && (tempRed2 + speed2) < Red23) {tempRed2 += speed2;}
      if (tempRed2 < Red23 && (tempRed2 + speed2) > Red23) {tempRed2 = Red23;}

      if (tempRed2 > Red23 && (tempRed2 - speed2) > Red23) {tempRed2 -= speed2;}
      if (tempRed2 > Red23 && (tempRed2 - speed2) < Red23) {tempRed2 = Red23;}


      if (tempGreen2 < Green23 && (tempGreen2 + speed2) < Green23) {tempGreen2 += speed2;}
      if (tempGreen2 < Green23 && (tempGreen2 + speed2) > Green23) {tempGreen2 = Green23;}

      if (tempGreen2 > Green23 && (tempGreen2 - speed2) > Green23) {tempGreen2 -= speed2;}
      if (tempGreen2 > Green23 && (tempGreen2 - speed2) < Green23) {tempGreen2 = Green23;}


      if (tempBlue2 < Blue23 && (tempBlue2 + speed2) < Blue23) {tempBlue2 += speed2;}
      if (tempBlue2 < Blue23 && (tempBlue2 + speed2) > Blue23) {tempBlue2 = Blue23;}

      if (tempBlue2 > Blue23 && (tempBlue2 - speed2) > Blue23) {tempBlue2 -= speed2;}
      if (tempBlue2 > Blue23 && (tempBlue2 - speed2) < Blue23) {tempBlue2 = Blue23;}
    }
    else if ((tempRed2 == Red23 || tempGreen2 == Green23 || tempBlue2 == Blue23) && tempcolor2 == 2 && tempcolor2 < colorN2){
      tempcolor2++;
    }
    else if ((tempRed2 != Red24 || tempGreen2 != Green24 || tempBlue2 != Blue24) && tempcolor2 == 3){
      if (tempRed2 < Red24 && (tempRed2 + speed2) < Red24) {tempRed2 += speed2;}
      if (tempRed2 < Red24 && (tempRed2 + speed2) > Red24) {tempRed2 = Red24;}

      if (tempRed2 > Red24 && (tempRed2 - speed2) > Red24) {tempRed2 -= speed2;}
      if (tempRed2 > Red24 && (tempRed2 - speed2) < Red24) {tempRed2 = Red24;}


      if (tempGreen2 < Green24 && (tempGreen2 + speed2) < Green24) {tempGreen2 += speed2;}
      if (tempGreen2 < Green24 && (tempGreen2 + speed2) > Green24) {tempGreen2 = Green24;}

      if (tempGreen2 > Green24 && (tempGreen2 - speed2) > Green24) {tempGreen2 -= speed2;}
      if (tempGreen2 > Green24 && (tempGreen2 - speed2) < Green24) {tempGreen2 = Green24;}


      if (tempBlue2 < Blue24 && (tempBlue2 + speed2) < Blue24) {tempBlue2 += speed2;}
      if (tempBlue2 < Blue24 && (tempBlue2 + speed2) > Blue24) {tempBlue2 = Blue24;}

      if (tempBlue2 > Blue24 && (tempBlue2 - speed2) > Blue24) {tempBlue2 -= speed2;}
      if (tempBlue2 > Blue24 && (tempBlue2 - speed2) < Blue24) {tempBlue2 = Blue24;}
    }
    else if ((tempRed2 == Red24 || tempGreen2 == Green24 || tempBlue2 == Blue24) && tempcolor2 == 3 && tempcolor2 < colorN2){
      tempcolor2++;
    }

    //Serial.print("Current color number is:");
    //Serial.println(tempcolor1);
    if(tempcolor1 >= colorN1){
      tempcolor1 = 0;
    }

    if(tempcolor2 >= colorN2){
      tempcolor2 = 0;
    }
    if (switch1){
      ledcWrite(0, tempRed1); // set the brightness of the LED
      ledcWrite(1, tempGreen1);
      ledcWrite(2, tempBlue1);
    }
    else{
      ledcWrite(0, 0); // set the brightness of the LED
      ledcWrite(1, 0);
      ledcWrite(2, 0);
    }
    if (switch2){
    ledcWrite(3, tempRed2); // set the brightness of the LED
    ledcWrite(4, tempGreen2);
    ledcWrite(5, tempBlue2);
    }
    else{
      ledcWrite(3, 0); // set the brightness of the LED
      ledcWrite(4, 0);
      ledcWrite(5, 0);
    }
    }

  /*Serial.print("0: ");
  Serial.println(sonicMaster.getDist_cm(0)); //(R1 outside)

  Serial.print("3: ");
  Serial.println(sonicMaster.getDist_cm(3)); //(R1 inside)*/

  Serial.print("1: ");
  Serial.println(sonicMaster.getDist_cm(1)); //(R2 outside)
  Serial.print("2: ");
  Serial.println(sonicMaster.getDist_cm(2)); //(R2 inside)

  int light1 = analogRead(photoPin1);
  int light2 = analogRead(photoPin2);


//ROOM 1 AUTO-MODE CODE
if (sonicMaster.getDist_cm(0) < 80 && sonicMaster.getDist_cm(3) > 50 && insider1 == 0 && outsider1 == 0){
    outsider1 = 1;
    Serial.println("OUTSIDE Door ROOM 1");
  }
  if (sonicMaster.getDist_cm(3) < 50 && outsider1 == 1){
    Serial.println("Someone Came IN ROOM 1");
    Serial.println("");
    R1counter++;
    outsider1 = 0;
    delay(2000);
  }

  if (sonicMaster.getDist_cm(3) < 50 && sonicMaster.getDist_cm(0) > 80 && insider1 == 0 && outsider1 == 0 && R1counter > 0){
    insider1 = 1;
    Serial.println("INSIDE Door ROOM 1");
  }
  if (sonicMaster.getDist_cm(0) < 80 && insider1 == 1){
    Serial.println("Someone Came OUT ROOM 1");
    Serial.println("");
    R1counter--;
    insider1 = 0;
    delay(2000);
  }

    //Serial.print("Light sensor 1: ");
    //Serial.println(light1);
    if ((light1 < 10) && (autmode) && (R1counter > 0)) {
      //Serial.println("Room 1 is dark and occupied, turning on lights...");
      //ROOM 1 COLOR SHIFT CODE
      autoswitch1 = true;
if ((tempRed1 != Red11 || tempGreen1 != Green11 || tempBlue1 != Blue11) && tempcolor1 == 0){
      if (tempRed1 < Red11 && (tempRed1 + speed1) < Red11) {tempRed1 += speed1;}
      if (tempRed1 < Red11 && (tempRed1 + speed1) > Red11) {tempRed1 = Red11;}

      if (tempRed1 > Red11 && (tempRed1 - speed1) > Red11) {tempRed1 -= speed1;}
      if (tempRed1 > Red11 && (tempRed1 - speed1) < Red11) {tempRed1 = Red11;}


      if (tempGreen1 < Green11 && (tempGreen1 + speed1) < Green11) {tempGreen1 += speed1;}
      if (tempGreen1 < Green11 && (tempGreen1 + speed1) > Green11) {tempGreen1 = Green11;}

      if (tempGreen1 > Green11 && (tempGreen1 - speed1) > Green11) {tempGreen1 -= speed1;}
      if (tempGreen1 > Green11 && (tempGreen1 - speed1) < Green11) {tempGreen1 = Green11;}


      if (tempBlue1 < Blue11 && (tempBlue1 + speed1) < Blue11) {tempBlue1 += speed1;}
      if (tempBlue1 < Blue11 && (tempBlue1 + speed1) > Blue11) {tempBlue1 = Blue11;}

      if (tempBlue1 > Blue11 && (tempBlue1 - speed1) > Blue11) {tempBlue1 -= speed1;}
      if (tempBlue1 > Blue11 && (tempBlue1 - speed1) < Blue11) {tempBlue1 = Blue11;}
    }
    else if ((tempRed1 == Red11 || tempGreen1 == Green11 || tempBlue1 == Blue11) && tempcolor1 == 0 && tempcolor1 < colorN1){
      tempcolor1++;
    }
    else if ((tempRed1 != Red12 || tempGreen1 != Green12 || tempBlue1 != Blue12) && tempcolor1 == 1){
      if (tempRed1 < Red12 && (tempRed1 + speed1) < Red12) {tempRed1 += speed1;}
      if (tempRed1 < Red12 && (tempRed1 + speed1) > Red12) {tempRed1 = Red12;}

      if (tempRed1 > Red12 && (tempRed1 - speed1) > Red12) {tempRed1 -= speed1;}
      if (tempRed1 > Red12 && (tempRed1 - speed1) < Red12) {tempRed1 = Red12;}


      if (tempGreen1 < Green12 && (tempGreen1 + speed1) < Green12) {tempGreen1 += speed1;}
      if (tempGreen1 < Green12 && (tempGreen1 + speed1) > Green12) {tempGreen1 = Green12;}

      if (tempGreen1 > Green12 && (tempGreen1 - speed1) > Green12) {tempGreen1 -= speed1;}
      if (tempGreen1 > Green12 && (tempGreen1 - speed1) < Green12) {tempGreen1 = Green12;}


      if (tempBlue1 < Blue12 && (tempBlue1 + speed1) < Blue12) {tempBlue1 += speed1;}
      if (tempBlue1 < Blue12 && (tempBlue1 + speed1) > Blue12) {tempBlue1 = Blue12;}

      if (tempBlue1 > Blue12 && (tempBlue1 - speed1) > Blue12) {tempBlue1 -= speed1;}
      if (tempBlue1 > Blue12 && (tempBlue1 - speed1) < Blue12) {tempBlue1 = Blue12;}
    }
    else if ((tempRed1 == Red12 || tempGreen1 == Green12 || tempBlue1 == Blue12) && tempcolor1 == 1 && tempcolor1 < colorN1){
      tempcolor1++;
    }
    else if ((tempRed1 != Red13 || tempGreen1 != Green13 || tempBlue1 != Blue13) && tempcolor1 == 2){
      if (tempRed1 < Red13 && (tempRed1 + speed1) < Red13) {tempRed1 += speed1;}
      if (tempRed1 < Red13 && (tempRed1 + speed1) > Red13) {tempRed1 = Red13;}

      if (tempRed1 > Red13 && (tempRed1 - speed1) > Red13) {tempRed1 -= speed1;}
      if (tempRed1 > Red13 && (tempRed1 - speed1) < Red13) {tempRed1 = Red13;}


      if (tempGreen1 < Green13 && (tempGreen1 + speed1) < Green13) {tempGreen1 += speed1;}
      if (tempGreen1 < Green13 && (tempGreen1 + speed1) > Green13) {tempGreen1 = Green13;}

      if (tempGreen1 > Green13 && (tempGreen1 - speed1) > Green13) {tempGreen1 -= speed1;}
      if (tempGreen1 > Green13 && (tempGreen1 - speed1) < Green13) {tempGreen1 = Green13;}


      if (tempBlue1 < Blue13 && (tempBlue1 + speed1) < Blue13) {tempBlue1 += speed1;}
      if (tempBlue1 < Blue13 && (tempBlue1 + speed1) > Blue13) {tempBlue1 = Blue13;}

      if (tempBlue1 > Blue13 && (tempBlue1 - speed1) > Blue13) {tempBlue1 -= speed1;}
      if (tempBlue1 > Blue13 && (tempBlue1 - speed1) < Blue13) {tempBlue1 = Blue13;}
    }
    else if ((tempRed1 == Red13 || tempGreen1 == Green13 || tempBlue1 == Blue13) && tempcolor1 == 2 && tempcolor1 < colorN1){
      tempcolor1++;
    }
    else if ((tempRed1 != Red14 || tempGreen1 != Green14 || tempBlue1 != Blue14) && tempcolor1 == 3){
      if (tempRed1 < Red14 && (tempRed1 + speed1) < Red14) {tempRed1 += speed1;}
      if (tempRed1 < Red14 && (tempRed1 + speed1) > Red14) {tempRed1 = Red14;}

      if (tempRed1 > Red14 && (tempRed1 - speed1) > Red14) {tempRed1 -= speed1;}
      if (tempRed1 > Red14 && (tempRed1 - speed1) < Red14) {tempRed1 = Red14;}


      if (tempGreen1 < Green14 && (tempGreen1 + speed1) < Green14) {tempGreen1 += speed1;}
      if (tempGreen1 < Green14 && (tempGreen1 + speed1) > Green14) {tempGreen1 = Green14;}

      if (tempGreen1 > Green14 && (tempGreen1 - speed1) > Green14) {tempGreen1 -= speed1;}
      if (tempGreen1 > Green14 && (tempGreen1 - speed1) < Green14) {tempGreen1 = Green14;}


      if (tempBlue1 < Blue14 && (tempBlue1 + speed1) < Blue14) {tempBlue1 += speed1;}
      if (tempBlue1 < Blue14 && (tempBlue1 + speed1) > Blue14) {tempBlue1 = Blue14;}

      if (tempBlue1 > Blue14 && (tempBlue1 - speed1) > Blue14) {tempBlue1 -= speed1;}
      if (tempBlue1 > Blue14 && (tempBlue1 - speed1) < Blue14) {tempBlue1 = Blue14;}
    }
    else if ((tempRed1 == Red14 || tempGreen1 == Green14 || tempBlue1 == Blue14) && tempcolor1 == 3 && tempcolor1 < colorN1){
      tempcolor1++;
    }    

    if(tempcolor1 >= colorN1){
      tempcolor1 = 0;
    }

    ledcWrite(0, tempRed1); // set the brightness of the LED
    ledcWrite(1, tempGreen1);
    ledcWrite(2, tempBlue1);
  
    } 
    if (autmode && (R1counter == 0 || light1 > 10) && autoswitch1) {
      //Serial.println("Room 1 is okay or empty, turning off lights..."); 
      ledcWrite(0, 0); // set the brightness of the LED
      ledcWrite(1, 0);
      ledcWrite(2, 0);  
      autoswitch1 = false;   
    }
  //Serial.print("ROOM 1 has "); 
  //Serial.println(R1counter);  


  //ROOM 2 AUTO-MODE CODE
  if (sonicMaster.getDist_cm(1) < 60 && sonicMaster.getDist_cm(2) > 60 && insider2 == 0 && outsider2 == 0){
    outsider2 = 1;
    Serial.println("OUTSIDE Door ROOM 2");
  }
  if (sonicMaster.getDist_cm(2) < 60 && outsider2 == 1){
    Serial.println("Someone Came IN ROOM 2");
    Serial.println("");
    outsider2 = 0;
    R2counter++;
    delay(2000);
  }

  if (sonicMaster.getDist_cm(2) < 60 && sonicMaster.getDist_cm(1) > 60 && insider2 == 0 && outsider2 == 0 && R2counter > 0){
    insider2 = 1;
    Serial.println("INSIDE Door ROOM 2");
  }
  if (sonicMaster.getDist_cm(1) < 60 && insider2 == 1){
    Serial.println("Someone Came OUT ROOM 2");
    Serial.println("");
    insider2 = 0;
    R2counter--;
    delay(2000);
  }
  
  //Serial.print("Light sensor 2: ");
  //Serial.println(light2);
    if ((light2 < 10) && (autmode) && (R2counter > 0)) {
      //Serial.println("Room 2 is dark and occupied, turning on lights...");
      autoswitch2 = true;
      //ROOM 2 COLOR SHIFT CODE
    if ((tempRed2 != Red21 || tempGreen2 != Green21 || tempBlue2 != Blue21) && tempcolor2 == 0){
      if (tempRed2 < Red21 && (tempRed2 + speed2) < Red21) {tempRed2 += speed2;}
      if (tempRed2 < Red21 && (tempRed2 + speed2) > Red21) {tempRed2 = Red21;}

      if (tempRed2 > Red21 && (tempRed2 - speed2) > Red21) {tempRed2 -= speed2;}
      if (tempRed2 > Red21 && (tempRed2 - speed2) < Red21) {tempRed2 = Red21;}


      if (tempGreen2 < Green21 && (tempGreen2 + speed2) < Green21) {tempGreen2 += speed2;}
      if (tempGreen2 < Green21 && (tempGreen2 + speed2) > Green21) {tempGreen2 = Green21;}

      if (tempGreen2 > Green21 && (tempGreen2 - speed2) > Green21) {tempGreen2 -= speed2;}
      if (tempGreen2 > Green21 && (tempGreen2 - speed2) < Green21) {tempGreen2 = Green21;}


      if (tempBlue2 < Blue21 && (tempBlue2 + speed2) < Blue21) {tempBlue2 += speed2;}
      if (tempBlue2 < Blue21 && (tempBlue2 + speed2) > Blue21) {tempBlue2 = Blue21;}

      if (tempBlue2 > Blue21 && (tempBlue2 - speed2) > Blue21) {tempBlue2 -= speed2;}
      if (tempBlue2 > Blue21 && (tempBlue2 - speed2) < Blue21) {tempBlue2 = Blue21;}
    }
    else if ((tempRed2 == Red21 || tempGreen2 == Green21 || tempBlue2 == Blue21) && tempcolor2 == 0 && tempcolor2 < colorN2){
      tempcolor2++;
    }
    else if ((tempRed2 != Red22 || tempGreen2 != Green22 || tempBlue2 != Blue22) && tempcolor2 == 1){
      if (tempRed2 < Red22 && (tempRed2 + speed2) < Red22) {tempRed2 += speed2;}
      if (tempRed2 < Red22 && (tempRed2 + speed2) > Red22) {tempRed2 = Red22;}

      if (tempRed2 > Red22 && (tempRed2 - speed2) > Red22) {tempRed2 -= speed2;}
      if (tempRed2 > Red22 && (tempRed2 - speed2) < Red22) {tempRed2 = Red22;}


      if (tempGreen2 < Green22 && (tempGreen2 + speed2) < Green22) {tempGreen2 += speed2;}
      if (tempGreen2 < Green22 && (tempGreen2 + speed2) > Green22) {tempGreen2 = Green22;}

      if (tempGreen2 > Green22 && (tempGreen2 - speed2) > Green22) {tempGreen2 -= speed2;}
      if (tempGreen2 > Green22 && (tempGreen2 - speed2) < Green22) {tempGreen2 = Green22;}


      if (tempBlue2 < Blue22 && (tempBlue2 + speed2) < Blue22) {tempBlue2 += speed2;}
      if (tempBlue2 < Blue22 && (tempBlue2 + speed2) > Blue22) {tempBlue2 = Blue22;}

      if (tempBlue2 > Blue22 && (tempBlue2 - speed2) > Blue22) {tempBlue2 -= speed2;}
      if (tempBlue2 > Blue22 && (tempBlue2 - speed2) < Blue22) {tempBlue2 = Blue22;}
    }
    else if ((tempRed2 == Red22 || tempGreen2 == Green22 || tempBlue2 == Blue22) && tempcolor2 == 1 && tempcolor2 < colorN2){
      tempcolor2++;
    }
    else if ((tempRed2 != Red23 || tempGreen2 != Green23 || tempBlue2 != Blue23) && tempcolor2 == 2){
      if (tempRed2 < Red23 && (tempRed2 + speed2) < Red23) {tempRed2 += speed2;}
      if (tempRed2 < Red23 && (tempRed2 + speed2) > Red23) {tempRed2 = Red23;}

      if (tempRed2 > Red23 && (tempRed2 - speed2) > Red23) {tempRed2 -= speed2;}
      if (tempRed2 > Red23 && (tempRed2 - speed2) < Red23) {tempRed2 = Red23;}


      if (tempGreen2 < Green23 && (tempGreen2 + speed2) < Green23) {tempGreen2 += speed2;}
      if (tempGreen2 < Green23 && (tempGreen2 + speed2) > Green23) {tempGreen2 = Green23;}

      if (tempGreen2 > Green23 && (tempGreen2 - speed2) > Green23) {tempGreen2 -= speed2;}
      if (tempGreen2 > Green23 && (tempGreen2 - speed2) < Green23) {tempGreen2 = Green23;}


      if (tempBlue2 < Blue23 && (tempBlue2 + speed2) < Blue23) {tempBlue2 += speed2;}
      if (tempBlue2 < Blue23 && (tempBlue2 + speed2) > Blue23) {tempBlue2 = Blue23;}

      if (tempBlue2 > Blue23 && (tempBlue2 - speed2) > Blue23) {tempBlue2 -= speed2;}
      if (tempBlue2 > Blue23 && (tempBlue2 - speed2) < Blue23) {tempBlue2 = Blue23;}
    }
    else if ((tempRed2 == Red23 || tempGreen2 == Green23 || tempBlue2 == Blue23) && tempcolor2 == 2 && tempcolor2 < colorN2){
      tempcolor2++;
    }
    else if ((tempRed2 != Red24 || tempGreen2 != Green24 || tempBlue2 != Blue24) && tempcolor2 == 3){
      if (tempRed2 < Red24 && (tempRed2 + speed2) < Red24) {tempRed2 += speed2;}
      if (tempRed2 < Red24 && (tempRed2 + speed2) > Red24) {tempRed2 = Red24;}

      if (tempRed2 > Red24 && (tempRed2 - speed2) > Red24) {tempRed2 -= speed2;}
      if (tempRed2 > Red24 && (tempRed2 - speed2) < Red24) {tempRed2 = Red24;}


      if (tempGreen2 < Green24 && (tempGreen2 + speed2) < Green24) {tempGreen2 += speed2;}
      if (tempGreen2 < Green24 && (tempGreen2 + speed2) > Green24) {tempGreen2 = Green24;}

      if (tempGreen2 > Green24 && (tempGreen2 - speed2) > Green24) {tempGreen2 -= speed2;}
      if (tempGreen2 > Green24 && (tempGreen2 - speed2) < Green24) {tempGreen2 = Green24;}


      if (tempBlue2 < Blue24 && (tempBlue2 + speed2) < Blue24) {tempBlue2 += speed2;}
      if (tempBlue2 < Blue24 && (tempBlue2 + speed2) > Blue24) {tempBlue2 = Blue24;}

      if (tempBlue2 > Blue24 && (tempBlue2 - speed2) > Blue24) {tempBlue2 -= speed2;}
      if (tempBlue2 > Blue24 && (tempBlue2 - speed2) < Blue24) {tempBlue2 = Blue24;}
    }
    else if ((tempRed2 == Red24 || tempGreen2 == Green24 || tempBlue2 == Blue24) && tempcolor2 == 3 && tempcolor2 < colorN2){
      tempcolor2++;
    }

    if(tempcolor2 >= colorN2){
      tempcolor2 = 0;
    }

    ledcWrite(3, tempRed2); // set the brightness of the LED
    ledcWrite(4, tempGreen2);
    ledcWrite(5, tempBlue2);    
  
    } 
    if (autmode && (R2counter == 0 || light2 > 10) && autoswitch2) {
      //Serial.println("Room 1 is okay or empty, turning off lights..."); 
      ledcWrite(3, 0); // set the brightness of the LED
      ledcWrite(4, 0);
      ledcWrite(5, 0);     
      autoswitch2 = false;
    }
  //Serial.print("ROOM 2 has "); 
  //Serial.println(R2counter);
  
  delay(200);

}

