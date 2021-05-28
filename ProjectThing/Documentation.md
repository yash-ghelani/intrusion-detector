# COM3505 - Assignment 2

### The 'Home Alone' Intrusion Detection System!


The idea for this project was based around this scene from one of my favourite movies - Home Alone. 

[![Home Alone House Party](https://img.youtube.com/vi/_NrfND5KGtk/0.jpg)](https://www.youtube.com/watch?v=_NrfND5KGtk "Home Alone House Party")

There are three main features:
- A standard intrusion detection mode: when activated, the alarm is sounded
- Live photo capture and storage: the chip camera takes a photo of the room and stores to an SD card
- House Party mode: Lights are turned on, music is playing, the party is started! (with the intrusion detection running in the background)

All of which can be controlled from a webserver!

### Using the Device:

1. Connect to the ESP32's access point
2. Provide network details to connect the device to the internet
3. Access the control page to:
    - Activate/Deactivate the intrusion system
    - Take and save a photo with the chip camera
    - Activate the 'Home Alone Protocol'

A green LED indicates the system is not active - being in the room will not trigger the alarm. A red LED indiates the system is active - there is a 10 second window from when the system is activated to give the user time to leave the room before the alarm goes off. The system can also be deactivated from the webserver so that the lights return to normal and movement will no longer trigger the alarm.

Photos taken on the ESP32-CAM board are sent as email attachments to the user's email address using the ESP32 Mail Client library and an SMTP server. This action can be triggered either manually via the controls on the webserver, or automatically when an intruder is detected.

The 'Home Alone Protocol', as shown in the demo, triggers an IFTTT webhook which runs a routine involving changing the light colours and playing music - just like a party. However, the system is also live, meaning movements after the initial buffer period will trigger the alarm routine.


### Demo

System demo:



Provisioning demo:

https://drive.google.com/file/d/1oypjRXK1XpKEpK6XduvklJfTvHOe3iBW/view?usp=sharing

### Device & Site breakdown

###### Design

The circuit design of the device:
- ESP32-CAM board
- FTDI adapter
- PIR sensor
- 3 LED's with resistors

![](https://i.imgur.com/9zX1iqD.jpg)

> My IoT Device.

###### Website design & construction

When flashed, the device will start up a webserver on port 80 with the SSID: 'ssid' and Password: 'password' (kept it simple). Once connected, a webpage is loaded. The HTML for the webpages is constructed in pieces:
- The header - same for all pages except the tite tag contents (which gets passed as an input)
- The CSS (style tag) - same for all pages
- The body - there are different functions with different content for each of the different pages
    - The provisioning form
    - The LED control buttons
    - The device network status
- The footer - The same for all pages, contains links to the LED page and Form page

The different sections are stored in arrays - functions within the code use these arrays to build a string which then contains the full markup for that page. Extra markup is concatenated to the strings in some of the functions, depending on the state of the device.

The array  | The function | The page
------------- | ------------- | ------------
![](https://i.imgur.com/if3jPMo.png) | ![](https://i.imgur.com/Tp3Bt7S.png) | ![](https://i.imgur.com/lnXk7jn.png)

### Testing

###### Testing PIR sensor readings

The PIR sensor operates on GPIO pin 13, setting the pin to `HIGH` when motion is detected and `LOW` otherwise. I tested the sensor by printing the readings every half a second to the Serial output using `Serial.println(digitalRead(PIR));`

![](https://i.imgur.com/HJM1TZa.jpg)


###### Testing Provisioning

A user is able to provide internet access to a device by displaying a form for the user to choose a network and enter a password - the detected SSID's are displayed in a dropdown list, with the network witht he strongest connectivity being displayed at the top. Once a network is chosen, the password is entered and the user can submit the form. The SSID and the password are then entereed when trying to cennect and if the password is correct, the page will display the status page with the WiFi showing up as connected.

- Scanning nearby networks

![](https://i.imgur.com/HJM1TZa.jpg)

The device is clearly able to detect and display the SSID's and connection strengths of the nearby networks

- Signing into a network

The status of the connection is checked 5 times before requesting that the user reenters the password. This is because when the credentials are entered, the WiFi status can register as 'WL_IDLE_STATUS' (basically neither connected or disconnected). It usually only takes 1-2 checks of the status before the ESP connects to the network (unless the password is wrong, in which case it loops 5 times).

Correct password  | Incorrect password
------------- | -------------
![](https://i.imgur.com/jGh6yPW.png) | ![](https://i.imgur.com/lX4a29X.png)
![](https://i.imgur.com/hZ59kk7.jpg) | ![](https://i.imgur.com/qWMoZEl.jpg)


###### Testing IFTTT events

The IFTTT event I set up uses the 'Webhooks' service which receives a web request, then triggers a Spotify playback event. I first ensure that the ESP32 is connected to the internet by checking the status of the WiFi, then send a GET request with the custom URL. I then verify the success of the request by printing the HTTP code - a returned value of 200 demonstrates that the IFTTT server received the request and triggered the action. I also print out the payload which is a stock message sent by the IFTTT server, confirming the action.

*Serial output for IFTTT event triggering*  | IFTTT confirmation
----------------------------------- | ----------------------------------
![](https://i.imgur.com/uvJjbIl.png) | ![](https://i.imgur.com/Bk7UDib.png)


There is a limitation to this - If the spotify app on my device (phone or laptop) is not running/idle, the triggered action will fail due to a 400 error as show below

![](https://i.imgur.com/sdFJrXb.png)
*Spotify 400 error*


###### Testing photo capture and sending

I have used and adapted files from the ESP32 CameraWebServer example sketch for this project. The example code provided all of the complex camera pin definitions, as well as the camera configuration and intialisation. The inbuilt method `fb = esp_camera_fb_get();` allows me to capture a photo and store it in the frame buffer. Then, using the SPIFFS library, I create a new image file in memory and write out the contents of the frame buffer.

To send the image file over email, an SMTP server is used. I create an smtpData object, to which I set a SMTP Server host, SMTP port, account email address and password. I then construct the email, and use the ESP32 MailClient library to send the email with the attached image from the board memory.

*Serial output for Image Saving*  | Email confirmation
----------------------------------- | ----------------------------------
![](https://i.imgur.com/uvJjbIl.png) | ![](https://i.imgur.com/Bk7UDib.png)



### References

I have used and adapted files from the ESP32 CameraWebServer example sketch.

SPIFFS (Serial Peripheral Interface Flash File System):
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/spiffs.html

ESP32 Mail Client Library:
https://github.com/mobizt/ESP-Mail-Client

Sending email with attachments:
https://github.com/mobizt/ESP-Mail-Client/blob/master/examples/Send_Attachment_File/Send_Attachment_File.ino

WiFi library:
https://www.arduino.cc/en/Reference/WiFiStatus

HTTPClient:
https://github.com/amcewen/HttpClient

GET requests:
https://techtutorialsx.com/2017/05/19/esp32-http-get-requests/