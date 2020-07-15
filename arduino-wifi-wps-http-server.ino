#include "WiFi.h"
#include "esp_wps.h"

String header;
WiFiServer server(89);
/*
Change the definition of the WPS mode
from WPS_TYPE_PBC to WPS_TYPE_PIN in
the case that you are using pin type
WPS
*/
#define ESP_WPS_MODE WPS_TYPE_PBC
#define ESP_MANUFACTURER "ESPRESSIF"
#define ESP_MODEL_NUMBER "ESP32"
#define ESP_MODEL_NAME "ESPRESSIF IOT"
#define ESP_DEVICE_NAME "ESP STATION"

#define LED_WHAITING 25
#define RELAYPIN_1 23

static esp_wps_config_t config;

void wpsInitConfig()
{
  config.crypto_funcs = &g_wifi_default_wps_crypto_funcs;
  config.wps_type = ESP_WPS_MODE;
  strcpy(config.factory_info.manufacturer, ESP_MANUFACTURER);
  strcpy(config.factory_info.model_number, ESP_MODEL_NUMBER);
  strcpy(config.factory_info.model_name, ESP_MODEL_NAME);
  strcpy(config.factory_info.device_name, ESP_DEVICE_NAME);
}

 String wpspin2string(uint8_t a[])
{
  char wps_pin[9];
  for (int i = 0; i < 8; i++)
  {
    wps_pin[i] = a[i];
  }
  wps_pin[8] = '\0';
  return (String)wps_pin;
} 

void WiFiEvent(WiFiEvent_t event, system_event_info_t info)
{
  switch (event)
  {
  case SYSTEM_EVENT_STA_START:
    Serial.println("Station Mode Started");
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    Serial.println("Connected to :" + String(WiFi.SSID()));
    Serial.print("Got IP: ");
    Serial.println(WiFi.localIP());
     Serial.print("Got IP: ");
    Serial.println(WiFi.macAddress());
    server.begin();

    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    Serial.println("Disconnected from station, attempting reconnection");
    WiFi.reconnect();
    break;
  case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
    Serial.println("WPS Successfull, stopping WPS and connecting to: " + String(WiFi.SSID()));
    esp_wifi_wps_disable();
    delay(10);
    WiFi.begin();
    break;
  case SYSTEM_EVENT_STA_WPS_ER_FAILED:
    Serial.println("WPS Failed, retrying");
    esp_wifi_wps_disable();
    esp_wifi_wps_enable(&config);
    esp_wifi_wps_start(0);
    break;
  case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
    Serial.println("WPS Timedout, retrying");
    esp_wifi_wps_disable();
    esp_wifi_wps_enable(&config);
    esp_wifi_wps_start(0);
    break;
   case SYSTEM_EVENT_STA_WPS_ER_PIN:
    Serial.println("WPS_PIN = " + wpspin2string(info.sta_er_pin.pin_code));
    break; 
  default:
    break;
  }
}

void setup()
{
  Serial.begin(115200);
  delay(10);
  Serial.println();
  // Initialize the output variables as outputs
  pinMode(LED_WHAITING, OUTPUT);
  pinMode(RELAYPIN_1, OUTPUT); 
  digitalWrite(RELAYPIN_1, HIGH);

  WiFi.onEvent(WiFiEvent);
  WiFi.mode(WIFI_MODE_STA);
  Serial.println("Starting WPS");
  wpsInitConfig();
  esp_wifi_wps_enable(&config);
  esp_wifi_wps_start(0);
}

void loop()
{
   while (WiFi.status() != WL_CONNECTED){
      loading();
   }
  ClientsListner();
  
}
void loading(){
   digitalWrite(LED_WHAITING, HIGH);
   delay(500); 
   digitalWrite(LED_WHAITING, LOW);
   delay(500); 
  }
void ClientsListner()
{
  WiFiClient client = server.available(); //or EthernetClient  Listen for incoming clients

  if (client)
  {                                // If a new client connects,
    Serial.println("New Client."); // print a message out in the serial port
    String currentLine = "";       // make a String to hold incoming data from the client
    while (client.connected())
    { // loop while the client's connected
      if (client.available())
      {                         // if there's bytes to read from the client,
        char c = client.read(); // read a byte, then
        Serial.write(c);        // print it out the serial monitor
        header += c;
        if (c == '\n')
        { // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0)
          {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            // Serial.println(header);
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:application/json");
            client.println("Connection: close");
            client.println();
            // turns the GPIOs on and off
            if (header.indexOf("GET /23/on") >= 0)
            {
              digitalWrite(RELAYPIN_1, LOW);
              client.print("{\"status\":\"on\"}");
            }
            else if (header.indexOf("GET /23/off") >= 0)
            {
               digitalWrite(RELAYPIN_1, HIGH);
              client.print("{\"status\":\"off\"}");
            }
           /*  else if (header.indexOf("GET /status") >= 0)
            {
              String status = "{";
              if(digitalRead(RELAYPIN_1) == LOW){
                 status+="\"26\":\"
              }
              status = "}";
              client.print("{\"26\":\"" + lamp26status + "\"}");
            } */
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          }
          else
          { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        }
        else if (c != '\r')
        {                   // if you got anything else but a carriage return character,
          currentLine += c; // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
