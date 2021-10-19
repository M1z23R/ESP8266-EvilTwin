# ESP8266-EvilTwin v2

Welcome to the project.
This project is inspired by https://github.com/vk496/linset and https://github.com/SpacehuhnTech/esp8266_deauther

Use this tool only on networks that you have permission for.

Note: Removed AsyncWebServer due to performance and stability
Note: Reworked deauthing - now it changes channel so deauthing can work properly


Note: I don't know anything about licenes, copyrights, etc.  

Credits to:
	Deauthing: https://github.com/SpacehuhnTech/esp8266_deauther        // Learned from  
	ESP8266 Core: https://github.com/espressif/arduino-esp32            // Edited and used  




# How to compile:
Check first: https://github.com/SpacehuhnTech/esp8266_deauther/tree/v1
It has steps on how to enable wifi_send_pkt_freedom (crucial for deauthing to work)

After that, simply open in Arduino IDE and compile/upload
Feel free to change html pages (I am still using strings, version 3 might be even more stable with the use of char arrays)
Also, feel free to add suggestions, I am sorry about not being active for a long time, and for unanswered issues but I hope this basic version works for everyone.

Note: Deauthing might not work on some phones due to the fact that I am using broadcast as station target (attacks everyone on network and not specific client).
