#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <ESPmDNS.h>
#else
#error "Board not found"
#endif

#include <WebSocketsServer.h>



#define In1 D3
#define In2 D4
#define In3 D5
#define In4 D6
#define En1 D7
#define En2 D8

char webpage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <style>
        h1 {
            font-size: 4rem;
            color: #e6c11d;
            font-family: "Arvo", cursive;
            text-shadow: 3px 0 #DA0463;
        }
        .drum {
            outline: none;
            border: 10px solid #404B69;
            font-size: 5rem;
            font-family: 'Arvo';
            line-height: 2;
            font-weight: 900;
            color: #7207fd;
            text-shadow: 3px 0 #DBEDF3;
            border-radius: 15px;
            display: inline-block;
            width: 150px;
            height: 150px;
            text-align: center;
            margin: 5px;
            background-color: white;
        }
        .a {
            margin-right: 8rem;
        }
        .pressed {
            box-shadow: 0 3px 4px 0 #9900ff;
            opacity: 0.8w;
        }
    </style>
</head>
        
<script>
var connection = new WebSocket('ws://'+location.hostname+':81/');
function downPress(downKey){
    livePress(downKey);
    switch (downKey) {
        case "w":
            console.log("forward");
            connection.send("forward");
            break;
        case "a":
            console.log("right");
            connection.send("right");
            break;
        case "d":
            console.log("left");
            connection.send("left");
            break;
        case "s":
            console.log("backward");
            connection.send("backward");
            break;
        default:
            console.log("nope");
    }
}
function up(upKey) {
    removelivePress(upKey);
    switch (upKey) {
        case "w":
            console.log("stop");
            connection.send("stop");
            break;
        case "a":
            console.log("stop");
            connection.send("stop");
            break;
        case "d":
            console.log("stop");
            connection.send("stop");
            break;
        case "s":
            console.log("stop");
            connection.send("stop");
            break;
        default:
            console.log("nope");
    }
}
function livePress(keyPressed) {
    var activeButton = document.querySelector("." + keyPressed);
    activeButton.classList.add("pressed");
}
function removelivePress(keyPressed) {
    var activeButton = document.querySelector("." + keyPressed);
    activeButton.classList.remove("pressed");
}
document.addEventListener("keydown", function(event){
    downPress(event.key);
});
document.addEventListener("keyup", function(event){
    up(event.key);
});

</script>
<body>
<center>
<h1>Car Panel</h1>

<div class="first">
    <button class="w drum">w</button>
</div>
<div class="second">
    <button class="a drum">a</button>
    <button class="d drum">d</button>
</div>
<div class="third">
    <button class="s drum">s</button>
</div>
  
</center>
</body>
</html>
)=====";

// ipaddress/led1/on
//ipaddress/led1/off

// ipaddress/led2/on
//ipaddress/led2/off
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80); // server port 80
WebSocketsServer websockets(81);

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Page Not found");
}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch (type) 
  {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
        IPAddress ip = websockets.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        websockets.sendTXT(num, "Connected from server");
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\n", num, payload);
      String message = String((char*)( payload));
      Serial.println(message);

      if(message == "forward"){
        digitalWrite(In1,LOW);
        digitalWrite(In2,HIGH);
        analogWrite(En1,100);
        digitalWrite(In3,HIGH);
        digitalWrite(In4,LOW);
        analogWrite(En2,100);
      }

      if(message == "left"){
        digitalWrite(In1,HIGH);
        digitalWrite(In2,LOW);
        analogWrite(En1,200);
        digitalWrite(In3,HIGH);
        digitalWrite(In4,LOW);
        analogWrite(En2,200);
      }

      if(message == "right"){
        digitalWrite(In1,LOW);
        digitalWrite(In2,HIGH);
        analogWrite(En1,200);
        digitalWrite(In3,LOW);
        digitalWrite(In4,HIGH);
        analogWrite(En2,200);
      }

      if(message == "backward"){
        digitalWrite(In1,HIGH);
        digitalWrite(In2,LOW);
        analogWrite(En1,100);
        digitalWrite(In3,LOW);
        digitalWrite(In4,HIGH);
        analogWrite(En2,100);
      }
      
      if(message == "stop") {
        digitalWrite(In1,LOW);
        digitalWrite(In2,LOW);
        analogWrite(En1,0);
        digitalWrite(In3,LOW);
        digitalWrite(In4,LOW);
        analogWrite(En2,0);
      }

  }
}

void setup(void)
{
  
  Serial.begin(115200);
  pinMode(In1,OUTPUT);
  pinMode(In2,OUTPUT);
  pinMode(En1, OUTPUT);
  pinMode(In3,OUTPUT);
  pinMode(In4,OUTPUT);
  pinMode(En2, OUTPUT);
  
  WiFi.softAP("MCU", "12345678");
  Serial.println("softap");
  Serial.println("");
  Serial.println(WiFi.softAPIP());


  if (MDNS.begin("ESP")) { //esp.local/
    Serial.println("MDNS responder started");
  }



  server.on("/", [](AsyncWebServerRequest * request)
  { 
   
  request->send_P(200, "text/html", webpage);
  });

  server.onNotFound(notFound);

  server.begin();  // it will start webserver
  websockets.begin();
  websockets.onEvent(webSocketEvent);

}


void loop(void)
{
 websockets.loop();
}
