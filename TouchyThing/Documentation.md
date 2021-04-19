# COM3505 - Assignment 1
========================

### The IoT device I have created implements:

- Touch-based LED control
- Web-based LED control
- ESP32 provisioning
- Touch-based IFTTT Spotify control

### Using the Device:

##### Offline control
Before providing your network details to the ESP32 there are 2 ways to control the LED's on the board.

- Touching the wire will light up the LED's in a traffic light fashion (R -> Y -> G-> Y -> R...)
- Touching the plastic will flash the LED's simultaneously in a heartbeat fashion

###### Online control
After connecting to the ESP32's access point, the user can:
- Provide network details to connect the device to the internet
    - Touch the plastic part of the wire to trigger the IFTTT Spotify event
- Control the traffic light LED's via the 'Change' page buttons (If the lights are green, you can change it to red and vice versa)

### Demo

<figure class="video_container">
  <iframe src="https://drive.google.com/file/d/0B6m34D8cFdpMZndKTlBRU0tmczg/preview" frameborder="0" allowfullscreen="true"> </iframe>
</figure>

<figure class="video_container">
  <iframe src="https://drive.google.com/file/d/0B6m34D8cFdpMZndKTlBRU0tmczg/preview" frameborder="0" allowfullscreen="true"> </iframe>
</figure>

### Device & Site breakdown

###### Design

The circuit design of the device is simple; it consists of:
- The ESP32
- 3 LED's with resistors
- A touch control wire

![](https://pandao.github.io/editor.md/examples/images/4.jpg)

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

### Testing

###### Testing Touch Control
For the touch control I utilised the T6 pin on the ESP32 
- touching the bare wire gave me readings within the range 10-12
- touching the plastic gave me readings within the range 69-73
- leaving the wire untouched gave me readings of 78-81

Occasionally, as seen in the serial plot, the ESP would pick up anomalous, noisy readings from the wire - to correct this, I decided to take 2 readings, 200ms apart. I compared the two values and if they within +- 3 from one another, the reading was valid. If not, one of the readings was anomalous and another set of readings would be taken. The method is simple but effective, as below I have plotted another graph of readings except this time i have given noisy readings a value of 100 to make it clear.

Noisy readings  | Controlled noisy readings
------------- | -------------
![](https://pandao.github.io/editor.md/examples/images/4.jpg) | ![](https://pandao.github.io/editor.md/examples/images/4.jpg)

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

###### Testing Provisioning

- Scanning nearby networks

    ![](https://pandao.github.io/editor.md/examples/images/4.jpg)

    The device is clearly able to detect and display the SSID's and connection strengths of the nearby networks

- Signing into a network

    Correct password  | Incorrect password
    ------------- | -------------
    ![](https://pandao.github.io/editor.md/examples/images/4.jpg) | ![](https://pandao.github.io/editor.md/examples/images/4.jpg)


###### Testing IFTTT event

The IFTTT event I set up uses the 'Webhooks' service which receives a web request, then triggers a Spotify playback event. I first ensure that the ESP32 is connected to the internet by checking the status of the WiFi, then send a GET request with the custom URL. I then verify the success of the request by printing the HTTP code - a returned value of 200 demonstrates that the IFTTT server received the request and triggered the action. I also print out the payload which is a stock message sent by the IFTTT server, confirming the action.

*Serial output for IFTTT event triggering*
>

There is a limitation to this - If the spotify app on my device (phone or laptop) is not running/idle, the triggered action will fail due to a 400 error as show below

![Spotify 400 error](https://pandao.github.io/editor.md/examples/images/4.jpg)