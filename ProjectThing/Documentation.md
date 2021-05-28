# COM3505 - Assignment 2

### The 'Home Alone' Intrusion Detection System!


The idea for this project was based around this scene from one of my favourite movies - Home Alone. 

[![Home Alone](https://res.cloudinary.com/marcomontalbano/image/upload/v1622180650/video_to_markdown/images/youtube--_NrfND5KGtk-c05b58ac6eb4c4700831b2b3070cd403.jpg)](https://www.youtube.com/watch?v=_NrfND5KGtk "Home Alone")


There are three main features:
- A standard intrusion detection mode: when activated, the alarm is sounded
- Live photo capture: the chip camera takes a photo of the room and sends it to the user via email
- House Party mode: Lights are turned on, music is playing, the party is started! (with the intrusion detection running in the background)

All of which can be controlled from a webserver!

### Using the Device:

1. Connect to the ESP32's access point
2. Provide network details to connect the device to the internet
3. Access the control page to:
    - Activate/Deactivate the intrusion system
    - Take and save a photo with the chip camera
    - Activate the 'Home Alone Protocol'

A green LED indicates the system is not active - being in the room will not trigger the alarm. A red LED indiates the system is active - there is a 10 second window from when the system is activated to give the user time to leave the room before the alarm goes off. The system can also be deactivated from the webserver so that the lights return to normal and movement no longer triggers the alarm.

Photos taken on the ESP32-CAM board are sent as email attachments to the user's email address using the ESP32 Mail Client library and an SMTP server. This action can be triggered either manually via the controls on the webserver, or automatically when an intruder is detected.

The 'Home Alone Protocol', as shown in the demo, triggers an IFTTT webhook which runs a routine involving changing the light colours and playing music - just like a party. However, the system is also live, meaning movements after the initial buffer period will trigger the alarm routine.


### Demo

System demo:

[![System Demo](https://res.cloudinary.com/marcomontalbano/image/upload/v1622172209/video_to_markdown/images/google-drive--105kWHTH2NAy-C77CWtCL1wmI0W2H55-u-c05b58ac6eb4c4700831b2b3070cd403.jpg)](https://drive.google.com/file/d/105kWHTH2NAy-C77CWtCL1wmI0W2H55-u/view?usp=sharing "System Demo")

https://drive.google.com/file/d/105kWHTH2NAy-C77CWtCL1wmI0W2H55-u/view?usp=sharing

Provisioning demo:

https://drive.google.com/file/d/1oypjRXK1XpKEpK6XduvklJfTvHOe3iBW/view?usp=sharing

### Device & Site breakdown

###### Design

The circuit design of the device:
- ESP32-CAM board
- FTDI adapter
- PIR sensor
- 3 LED's with resistors

Device  | Circuit diagram
------------- | -------------
![](https://i.imgur.com/VWXf30f.png) | ![](https://i.imgur.com/oo7ZAX3.jpg)

> My IoT Device.

The site pages:

WiFi Form  | System Control Page | Device Status Page
------------- | ------------- | ------------
![](https://i.imgur.com/lNzoy3b.png) | ![](https://i.imgur.com/ZyYbycp.png) | ![](https://i.imgur.com/bHPYVxt.png)


###### Website design & construction

When flashed, the device will start up a webserver on port 80 with the SSID: 'ssid' and Password: 'password' (kept it simple). Once connected, a webpage is loaded. The HTML for the webpages is constructed in pieces:
- The header - same for all pages except the tite tag contents (which gets passed as an input)
- The CSS (style tag) - same for all pages
- The body - there are different functions with different content for each of the different pages
    - The WiFi provisioning form
    - The System control buttons
    - The device network status
- The footer - The same for all pages, contains links to the System Control page and WiFi Form page

The different sections are stored in arrays - functions within the code use these arrays to build a string which then contains the full markup for that page. Extra markup is concatenated to the strings in some of the functions, depending on the state of the device.

The array  | The function | The page
------------- | ------------- | ------------
![](https://i.imgur.com/if3jPMo.png) | ![](https://i.imgur.com/Tp3Bt7S.png) | ![](https://i.imgur.com/lnXk7jn.png)

### Testing

###### Testing PIR sensor readings

The PIR sensor operates on GPIO pin 13, setting the pin to `HIGH` when motion is detected and `LOW` otherwise. I tested the sensor by printing the readings every half a second to the Serial output using `Serial.println(digitalRead(PIR));`

![](https://i.imgur.com/lRODWCB.jpg)

1. Section 1 of the readings is me opening the door and leaving the room. (The small peak is me closing the door shut)
2. Section 2 represents the readings in an empty room - no movement, no peaks. (ensures no random noisy readings)
3. Section 3 I enter the room again and move around, getting closer to the sensor every second. Clear indication of movement.


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

For this project, I have set up 3 IFTTT events which use the 'Webhooks' service to trigger different light and sound routines. The 3 events are:
- An alarm event - triggered when an intruder is detected, lights turn red and alarms played through speakers
- A House Party event - sets lights to random colours and plays music (inspired by the home alone scene)
- A reset evet - triggered when deactivating the system, sets the lights back to normal and cuts the music

To test the IFTTT events, I first ensure that the ESP32 is connected to the internet by checking the status of the WiFi, then send a GET request with the custom URL. I then verify the success of the request by printing the HTTP code - a returned value of 200 demonstrates that the IFTTT server received the request and triggered the action. I also print out the payload which is a stock message sent by the IFTTT server, confirming the action.

Event | Serial output for IFTTT event triggering  | IFTTT confirmation
------|----------------------------- | ----------------------------------
1 | ![](https://i.imgur.com/6uf9jrX.png) | ![](https://i.imgur.com/bUi9IDQ.png) 
2 | ![](https://i.imgur.com/lfwr4c4.png) | ![](https://i.imgur.com/XccfvI8.png) 
3 | ![](https://i.imgur.com/jYJfPy1.png) | ![](https://i.imgur.com/rhsrnY1.png) 


###### Testing photo capture and sending

I have used and adapted files from the ESP32 CameraWebServer example sketch for this project. The example code provided all of the complex camera pin definitions, as well as the camera configuration and intialisation. The inbuilt method `fb = esp_camera_fb_get();` allows me to capture a photo and store it in the frame buffer. Then, using the SPIFFS library, I create a new image file in memory and write out the contents of the frame buffer.

To send the image file over email, an SMTP server is used. I create an smtpData object, to which I set a SMTP Server host, SMTP port, account email address and password. I then construct the email, and use the ESP32 MailClient library to send the email with the attached image from the board memory.

Serial output for Image Saving  | Email confirmation
----------------------------------- | ----------------------------------
![](https://i.imgur.com/qqGLzDE.png) | ![](https://i.imgur.com/Vjejqap.png) 


### Self Assessment

Buy and large, the system works well - the PIR sensor is capable of capturing movement within a room, the IFTTT events are triggered reliably and the photo capture and sending works as intended.

However, as with any project, there is room for improvement. Although the PIR sensor works well enough to detect most movement, through testing I have found that it is possible to be within the same room and move slow enough as to not set off the sensor. This is because the PIR sensor used in this project is fairly basic, and measuring subtle heat changes from minor movements is difficult. This issue can be overcome by using a more sensitive movement sensor, which makes the entire system more reliable.

Another issue is the quality of the photos captured by the ESP32-CAM board. The average size of the photos captured and sent over email is 5KB, therefore, the images are of very low quality. The red lighting helps illuminate objects in the room however it is still fairly pixelated and difficult to make out faces. A solution to this would be to improve the lighting and make use of the inbuilt flash on the board - I attempted to get this working, however, decided to focus my efforts on the core parts of the system functionality due to time constraints.

The device is still currently a prototype, assembled on a breadboard - an obvious improvement would be to move the components over to a circuit board, making the device a bit more physically robust and permanent.

To conclude, there are definitely improvements that can be made which will increase the usefulness and performance of the device, nonetheless, I believe that all of the features implemented work reliably and the device serves as an effective prototype. The site controls, although fairly rudimentry, are intuitive, easy to use and aesthetically minimal - the site structure allows more features to be added to the control page, with options for new pages as well.


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

Alexa Routines from Webhooks:
https://mkzense.com/
