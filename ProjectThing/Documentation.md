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

A green LED indicates the system is not active - being in the room will not trigger the alarm. A red LED indiates the system is active - there is a 10 second window from when the system is activated to give the user time to leave the room before the alarm goes off. 


### Demo

<figure class="video_container">
  <iframe src="https://drive.google.com/file/d/1oypjRXK1XpKEpK6XduvklJfTvHOe3iBW/preview" frameborder="0" allowfullscreen="true"> </iframe>
</figure>

https://drive.google.com/file/d/1ozbMkNnQQQ6YjLn9UOkw2xQf9SBb08y-/view?usp=sharing

<figure class="video_container">
  <iframe src="https://drive.google.com/file/d/1ozbMkNnQQQ6YjLn9UOkw2xQf9SBb08y-/preview" frameborder="0" allowfullscreen="true"> </iframe>
</figure>

https://drive.google.com/file/d/1oypjRXK1XpKEpK6XduvklJfTvHOe3iBW/view?usp=sharing

### Device & Site breakdown

###### Design

The circuit design of the device:
- ESP32-CAM board and FTDI adapter
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
For the touch control I utilised the T6 pin on the ESP32 
- touching the bare wire gave me readings within the range 10-15
- touching the plastic gave me readings within the range 67-69
- leaving the wire untouched gave me readings of 76-78

Occasionally, as seen in the serial plot, the ESP would pick up anomalous, noisy readings from the wire - to correct this, I decided to take 2 readings, 200ms apart. I compared the two values and if they within +- 3 from one another, the reading was valid. If not, one of the readings was anomalous and another set of readings would be taken. The method is simple but effective, as below I have plotted 2 graphs of readings; with noise (noisy values set at 100 for clarity) and without noise.

Noisy readings  | No noisy readings
------------- | -------------
![](https://i.imgur.com/R3lULKQ.png) | ![](https://i.imgur.com/1bIYSEP.png)

*Serial output for different controls*

>01:45:54.840 -> Light change
>01:45:55.823 -> Light change
>01:45:58.032 -> Light change
>01:45:59.423 -> Light flash
>01:45:59.831 -> WiFi Disconnected
>01:46:01.219 -> Light flash
>01:46:01.628 -> WiFi Disconnected
>01:46:02.823 -> Light flash
>01:46:03.228 -> WiFi Disconnected

The output shows 'WiFi Disconnected' because the action of touching the plastic part of the cable is meant to trigger an IFTTT event if the device is connected to the internet - when it isnt connected, the loop carries on as normal and the lights continue to flash whilst the wire is held.


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


###### Testing photo capture and saving




### References

WiFi library:
https://www.arduino.cc/en/Reference/WiFiStatus

HTTPClient:
https://github.com/amcewen/HttpClient

GET requests:
https://techtutorialsx.com/2017/05/19/esp32-http-get-requests/