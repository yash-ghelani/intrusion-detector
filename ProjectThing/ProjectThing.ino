#include "esp_camera.h"
#include "sketch.h"
#include <Arduino.h>
#include <Wire.h>
#include <esp_log.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include "ESP32_MailClient.h"
#include <FS.h>
#include <SPIFFS.h>

// Photo File Name to save in SPIFFS
#define FILE_PHOTO "/photo.jpg"

// Gmail SMTP server settings
#define emailSenderAccount    "newtempmail08@gmail.com"
#define emailSenderPassword   "Temporary08!"
#define smtpServer            "smtp.gmail.com"
#define smtpServerPort        465
#define emailSubject          "Intruder Detected"
#define emailRecipient        "yashghelani08@gmail.com"

// Camera
#define CAMERA_MODEL_AI_THINKER // Has PSRAM

#include "camera_pins.h"

// SMTP object
SMTPData smtpData;

// Webserver object
WebServer webServer(80);

// Iterations counter
int iterations = 0;

// LEDs
const int LED_R = 15;
const int LED_Y = 14;
const int LED_G = 2;
bool go = true;

// PIR sensor
const int PIR = 13;

// Flash
const int FLASH = 4;

// Provisioning
int nearbySSIDs = 0;
bool wifi = true;

// IFTTT
const char* request = "https://maker.ifttt.com/trigger/INTRUDER/with/key/uVFu5_EVZh-ldMw9irxrV";
const char* request2 = "https://maker.ifttt.com/trigger/HOMEALONE/with/key/uVFu5_EVZh-ldMw9irxrV";
const char* request3 = "https://maker.ifttt.com/trigger/RESET/with/key/uVFu5_EVZh-ldMw9irxrV";

// CSS
const char *styleArr[] = { // boilerplate: constants & pattern parts of template         // 8
  "html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n",
  "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 30px;}\n",
  ".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n",
  ".button-submit {display: block;background-color: #34495e;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n",
  ".button-on {background-color: #f44336;}\n",
  ".button-on:active {background-color: #2980b9;}\n",
  ".button-off {background-color: #34495e;}\n",
  ".button-off:active {background-color: #2c3e50;}\n",
  "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n",
  
};

// Body HTML content
const char *bodyArr[] = { 
  
  "<h1>ESP32 Web Server</h1>\n",
  "<h3>Using Access Point(AP) Mode</h3>\n",

};

// Body HTML content - System Control
const char *bodyArr2[] = { 
  
  "<h1>ESP32 Web Server</h1>\n",
  "<h3>Using Access Point(AP) Mode</h3>\n",
  "<p>System Control: </p>\n",
  "<a class=\"button button-off\" href=\"/activate\">Activate System</a>\n",
  "<a class=\"button button-off\" href=\"/photo\">View Room</a>\n",
  "<a class=\"button button-on\" href=\"/homealone\">HOME ALONE</a>\n",

};

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

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
  
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

  // ******************* new code ********************************]

  //set the LED as the output 
  pinMode(LED_R, OUTPUT);
  pinMode(LED_Y, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(FLASH, INPUT);
  
  //System starts deactivated
  digitalWrite(LED_G, HIGH);
  digitalWrite(4, LOW);
  
  nearbySSIDs = WiFi.scanNetworks(); //get number of nearby ssids
  
  dp("Starting webserver");
  
  if(! WiFi.mode(WIFI_AP_STA)){
    dp("failed to set Wifi mode");
  }
  if(! WiFi.softAP("ssid", "password")){
    dp("failed to start soft AP");
  }
  
  startWebServer();
  dp("System ready - connect to access point 'ssid'\n");

  // Initialize SPIFFS (SPI Flash File System) to save the last photo taken with the ESP32-CAM
  dp("Mounting SPIFFS");

  if (!SPIFFS.begin(true)) {
    dp("An Error has occurred while mounting SPIFFS");
    ESP.restart();
  }
  else {
    delay(500);
    dp("SPIFFS mounted successfully\n");
  }
 
}

void startWebServer(){
  webServer.on("/", handleRoot);
  webServer.on("/control", handleControl);
  webServer.on("/activate", handleActivate);
  webServer.on("/photo", handlePhoto);
  webServer.on("/homealone", handleAlone);
  webServer.on("/connect", handleConnect);
  webServer.begin();
  dp("Web server started\n");

}

// Building HTML for root page

void handleRoot(){
  dp("\nAccessing root page")
  
  String toSend = getPageTop("COM3505 - WiFi Login");
  toSend += getPageStyle();
  toSend += getPageBody();
  toSend += getPageForm();
  toSend += getPageFooter();

  webServer.send(200, "text/html", toSend);
}

// Building HTML for System control page

void handleControl(){

  dp("\nAccessing control page");
  
  String toSend = getPageTop("COM3505 - System Control");
  toSend += getPageStyle();
  toSend += getControlBody();
  toSend += getPageFooter();

  webServer.send(200, "text/html", toSend);
}

void handleActivate(){
  iterations = 0;
  if (digitalRead(LED_G) == HIGH){
    dp("\nActivating System\n");
    lightChange();
  } else {
    dp("\nDe-activating System\n");
    lightChange();
    doGET(request3);
  }
  
  
  String toSend = getPageTop("COM3505 - System Control");
  toSend += getPageStyle();
  toSend += getControlBody();
  toSend += getPageFooter();

  webServer.send(200, "text/html", toSend);
}

void handlePhoto(){
  takePhoto();
  sendPhoto();
  
  String toSend = getPageTop("COM3505 - System Control");
  toSend += getPageStyle();
  toSend += getControlBody();
  toSend += getPageFooter();

  webServer.send(200, "text/html", toSend);
}

void handleAlone(){
  dp("\nInitiating HOME ALONE PROTOCOL\n")
  iterations = 0;
  lightChange();
  doGET(request2);
  
  String toSend = getPageTop("COM3505 - System Control");
  toSend += getPageStyle();
  toSend += getControlBody();
  toSend += getPageFooter();

  webServer.send(200, "text/html", toSend);
}

// Building HTML for Wifi Status page

void handleConnect() {

  // Strings to store agrs from provisioning form
  String ssid = "";
  String password = "";
  
  for(uint8_t i = 0; i < webServer.args(); i++ ) {
    if(webServer.argName(i) == "ssid")
      ssid = webServer.arg(i);
    else if(webServer.argName(i) == "password")
      password = webServer.arg(i);
  }

  // Getting credentials
  char ssidchars[ssid.length()+1];
  char passchars[password.length()+1];
  ssid.toCharArray(ssidchars, ssid.length()+1);
  password.toCharArray(passchars, password.length()+1);

  // Entering credentials
  WiFi.begin(ssidchars, passchars);    
  dp("Connecting");

  // Test status 5 times
  int tryCount = 0;  
  while(WiFi.status() != WL_CONNECTED && tryCount < 5) {
    delay(500);
    dp("Not connected - trying again");
    tryCount++;
  }

  if (WiFi.status() != WL_CONNECTED){
    wifi = false;
    dp("Connection failed - re-enter password");
  } else {
    wifi = true;
    dps("Connected to WiFi network with IP Address: ", WiFi.localIP());
  }
  
  // Loading the status page
  String toSend = getPageTop("COM3505 - Connect");
  toSend += getPageStyle();
  toSend += getConnectBody(wifi);
  toSend += getPageFooter();

  webServer.send(200, "text/html", toSend);
}

// Boilerplate template for all pages

String getPageTop(String title) {
  return
    "<html><head><title>"+title+" </title>\n"
    "<meta charset=\"utf-8\">"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
    "<style>\n"
  ;
};

// Stylesheets for all pages

String getPageStyle(){
  String style = "";
  int styleArrSize = sizeof(styleArr)/sizeof(styleArr[0]); //8
  for(int i = 0; i < styleArrSize; i++) {
      style.concat(styleArr[i]);
  }
  style.concat("</style></head><body>\n");
  return style;
}

// Body content for root page

String getPageBody() {

  String body = "";
  int bodyArrSize = sizeof(bodyArr)/sizeof(bodyArr[0]); //4
  for(int i = 0; i < bodyArrSize; i++) {
      body.concat(bodyArr[i]);
  }
  return body;
}

// Body content for change page

String getControlBody() {

  String body = "";
  int bodyArrSize = sizeof(bodyArr2)/sizeof(bodyArr2[0]); //4
  for(int i = 0; i < bodyArrSize; i++) {
      body.concat(bodyArr2[i]);
  }
  return body;
}

// Body content for status page

String getConnectBody(bool wifi) {

  String body = "";
  int bodyArrSize = sizeof(bodyArr)/sizeof(bodyArr[0]);
  for(int i = 0; i < bodyArrSize; i++) {
      body.concat(bodyArr[i]);
  }

  body.concat("<h3>WiFi status: </h3>\n");

  // Getting connection status  
  String state = "";
  switch(WiFi.status()) {
    case WL_IDLE_STATUS:
      state = "WL_IDLE_STATUS - Connecting... "; break;
    case WL_CONNECTED:
      state = "WL_CONNECTED - Successfully connected! </li>"; break;
    case WL_CONNECT_FAILED:
      state = "WL_CONNECT_FAILED - Connection failed </li>"; break;
    case WL_CONNECTION_LOST:
      state = "WL_CONNECTION_LOST - Connection Lost</li>"; break;
    case WL_DISCONNECTED:
      state = "WL_DISCONNECTED - Disconnected</li>"; break;
    default:
      state = "unknown</li>";
  }

  
  body.concat("<p>" + state + "</p>\n");
  
  return body;
}

// HTML for provisioning form

String getPageForm(){
  String form = "";

  if(nearbySSIDs == 0) {
    
    dp("No nearby networks found");
    form.concat("<h3>No nearby networks found </h3>\n");
    form.concat("<p><a href='/'>Scan Again</a></p>\n");
    
  } else {

    form.concat("<h3>Nearby networks found: </h3>\n");
    form.concat("<p><form method='POST' action='connect'\n> ");
    form.concat("<label for='ssid'>Select a network: </label>\n");
    form.concat("<select name='ssid' required>\n");
         
    for(int i = 0; i < nearbySSIDs; ++i) {
      form.concat("<option value='");
      form.concat(WiFi.SSID(i));
      form.concat("'>");
      form.concat(WiFi.SSID(i));
      form.concat(" - Strength: ");
      form.concat(WiFi.RSSI(i));
      form.concat("</option><br><br>\n");
    }
    
    form.concat("</select><br><br>\n");
    form.concat("<label for='password'>Password: </label>\n");
    form.concat("<input type='textarea' name='password'><br/><br/> ");
    form.concat("<input type='submit' value='Submit' class='button-submit'></form></p>");
  }

  return form;
}

// Boilerplate template footer for all pages

String getPageFooter() {
  return "\n<p><a href='/'>Home</a>   <a href='/control'>Control</a></p></body></html>\n";
}

// Function to operate LED's like traffic lights

void lightChange(){
  
  if (digitalRead(LED_G) == HIGH){ // if system is inactive
    delay(400);
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_Y, HIGH);
    delay(400);
    digitalWrite(LED_Y, LOW);
    digitalWrite(LED_R, HIGH);
  
  } else { // if system is active
    delay(400);
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_Y, HIGH);
    delay(400);
    digitalWrite(LED_Y, LOW);
    digitalWrite(LED_G, HIGH);
  }
}

// GET request for IFTTT applet

void doGET(const char* request){
  //Check WiFi connection status
  if(WiFi.status()== WL_CONNECTED){
    
    HTTPClient http;
    dp("Sending request");
    http.begin(request);
    int httpCode = http.GET();
    dps("HTTP Code: ", httpCode);
    
    if (httpCode > 0) { //Check for the returning code
      
      String payload = http.getString();
      dps("Output: ", payload);
      
    } else {
      dp("Error on HTTP request");
    }
      
    // Free resources
    http.end();
    dp("HTTP client session closed\n");
    
  } else {
    dp("WiFi Disconnected");
  }
  
}

// Check if photo capture was successful
bool checkPhoto( fs::FS &fs ) {
  File f_pic = fs.open( FILE_PHOTO );
  unsigned int pic_sz = f_pic.size();
  return ( pic_sz > 100 );
}

// Capture Photo and Save it to SPIFFS
void takePhoto() {

  digitalWrite(FLASH, HIGH);
  
  camera_fb_t * fb = NULL; // pointer
  bool ok = 0; // Boolean indicating if the picture has been taken correctly

  do {
    // Take a photo with the camera
    dp("\nTaking a photo...");

    fb = esp_camera_fb_get();
    if (!fb) {
      dp("Camera capture failed");
      return;
    }

    // Photo file name
    dps("Picture file name: ", FILE_PHOTO);
    File file = SPIFFS.open(FILE_PHOTO, FILE_WRITE);

    // Insert the data in the photo file
    if (!file) {
      dp("Failed to open file in writing mode");
    } else {
      file.write(fb->buf, fb->len); // payload (image), payload length
      dp("The picture has been saved in ");
      dp(FILE_PHOTO);
    }
    // Close the file
    file.close();
    esp_camera_fb_return(fb);

    // check if file has been correctly saved in SPIFFS
    ok = checkPhoto(SPIFFS);
  } while ( !ok );

  digitalWrite(FLASH, LOW);
}

void sendPhoto( void ) {
  dp("\nSending email...");
  
  // Set the SMTP Server Email host, port, account and password
  smtpData.setLogin(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword);
  smtpData.setSender("Intrusion Detection System", emailSenderAccount); // Set the sender name and Email
  smtpData.setPriority("High"); // email priority
  smtpData.setSubject(emailSubject); // Set the subject
  smtpData.setMessage("<h2>Photo captured with ESP32-CAM and attached in this email.</h2>", true);
  smtpData.addRecipient(emailRecipient); 
  smtpData.addAttachFile(FILE_PHOTO, "image/jpg"); // Add attach files from SPIFFS
  smtpData.setFileStorageType(MailClientStorageType::SPIFFS); // Set the storage type to attach files in your email (SPIFFS)
  smtpData.setSendCallback(sendCallback); // email status
  
  // Start sending Email, can be set callback function to track the status
  if (!MailClient.sendMail(smtpData)){
    dps("Error sending Email: ", MailClient.smtpErrorReason());
  } else {
    dp("Email successful");
  }
  smtpData.empty();
}

// Callback function to get the Email sending status
void sendCallback(SendStatus msg) {
  //Print the current status
  Serial.println(msg.info());
}


void loop() {
 
  if(digitalRead(LED_R) == HIGH){ // if system is active
    
    if (iterations > 10000 && digitalRead(PIR) == 1){ // time between trigger ~10 seconds
      iterations = 0;
      dps("INTRUDER DETECTED");
//      doGET(request);
//      takePhoto();
//      sendPhoto();
    }
  }
    
  // iterations counter - regulated by 1 ms delay
  iterations ++;
  delay(3000);
  Serial.println(digitalRead(PIR));
  
  // deal with any pending web requests
  webServer.handleClient();
}
