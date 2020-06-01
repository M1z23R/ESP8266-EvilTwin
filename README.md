# ESP8266-EvilTwin

Welcome to the project.
This project is inspired by https://github.com/vk496/linset and https://github.com/SpacehuhnTech/esp8266_deauther

Use this tool only on networks that you have permission for.

I would like to point out that the project is in a very early stage and that compiling isn't possible at the moment due to the fact that I've modified https://github.com/me-no-dev/ESPAsyncWebServer a bit and even.  
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


# How to compile (WINDOWS 10 64bit)
This is step by step what errors I got and what steps I did to make async libs work with 2.0.0 sdk.

```
Install both sdk 2.0.0 and sdk 2.7.1

Set SDK to 2.0.0

Extract
%UserProfile%\Documents\ArduinoData\staging\packages\esp8266-2.7.1.zip



ERROR 1
libb64/cencode.h: No such file or directory
FIX 1
Copy folder libb64 from extracted (newer sdk) to 
%UserProfile%\Documents\ArduinoData\packages\esp8266\hardware\esp8266\2.0.0\cores\esp8266

ERROR 2
class cbuf
FIX 2
Rename cbuf.cpp and cbuf.h to cbuf_alt.cpp and cbuf_alt.h from extracted (newer sdk) then copy to
%UserProfile%\Documents\ArduinoData\packages\esp8266\hardware\esp8266\2.0.0\cores\esp8266
Edit cbuf_alt.cpp to include cbuf_alt.h instead of cbuf.h
Replace includes for cbuf with cbuf_alt in AsyncTCP lib:
SyncClient.cpp line 24
ESPAsyncTCPbuffer.h line 35

Edit WebAuthentication.cpp from AsyncWebServer lib line 91
  uint32_t r = rand(); //RANDOM_REG32;


Replace includes for cbuf with cbuf_alt in AsyncWebServer lib:
WebResponses.cpp line 23

ERROR 3
undefined reference to '__cxa_guard_acquire'
FIX 3

Edit the following file:
%UserProfile%\Documents\ArduinoData\packages\esp8266\hardware\esp8266\2.0.0\cores\esp8266\platform.txt
Line 30
compiler.cpp.flags=-c -Os -g -mlongcalls -mtext-section-literals -fno-exceptions -fno-rtti -falign-functions=4 -std=c++11 -MMD -ffunction-sections -fdata-sections
compiler.cpp.flags=-c -Os -g -mlongcalls -mtext-section-literals -fno-exceptions -fno-rtti -falign-functions=4 -std=c++11 -MMD -ffunction-sections -fdata-sections -fno-threadsafe-statics

In case line number isn't the same fine the compiler.cpp.flags and add " -fno-threadsafe-statics"

ERROR 4
std::nothrow
FIX 4
%UserProfile%\Documents\Arduino\libraries\ESPAsyncTCP-master\src\ESPAsyncTCP.cpp
Lines: 1251, 1267, 1269
Remove (std::nothrow)
		1251	AsyncClient *c = new AsyncClient(pcb, _ssl_ctx);
		1267	AsyncClient *c = new AsyncClient(pcb, NULL);
		1269	AsyncClient *c = new AsyncClient(pcb);

```
