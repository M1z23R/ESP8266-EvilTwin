# ESP8266-EvilTwin

Welcome to the project.
This project is inspired by https://github.com/vk496/linset and https://github.com/SpacehuhnTech/esp8266_deauther

Use this tool only on networks that you have permission for.

I would like to point out that the project is in a very early stage and that compiling isn't possible at the moment due to the fact that I've modified https://github.com/me-no-dev/ESPAsyncWebServer a bit and even SDK.  
I'll write a full tutorial on how to set everything up once I write everything down.

Note: I don't know anything about licenes, copyrights, etc.  

Here is a list of libraries I've used:  

Deauthing: https://github.com/SpacehuhnTech/esp8266_deauther        // Learned from  
ESPAsyncWebServer: https://github.com/me-no-dev/ESPAsyncWebServer   // Edited and used  
ESP8266 Core: https://github.com/espressif/arduino-esp32            // Edited and used  




# How to use:
Flash the realsed .bin file onto your ESP8266 board  
Open Arduino IDE  
Create a new project  
Create a folder named 'data' inside it  
Create an empty logs.txt file inside of the data folder  
Upload sketch data using the 'ESP8266 Sketch Data Uploader' - https://github.com/esp8266/arduino-esp8266fs-plugin  
Reset your board  
Find an AP named "M1z23R" - password is: 12341234  
Scan your targets.  
Select your target (Please be kind and use your own network).  
Press the 'Start deauth' button or don't (it's your choice if you want to be aggressive or not).  
Press the 'Start EvilTwin' - existing AP will be replaced with a fake one (evil twin) named same as your target without a password.  
Connect to it.  
Enter a password and click confirm. (if AP is on long range it might not accept the correct password).  
And that's it, your 'admin' AP will be back, deauthing will stop on correct password if it was initially started.  
