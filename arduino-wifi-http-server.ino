#include "WiFi.h"

const char *ssid = "HG531V1-J2SFVE";
const char *password = "01234567";
// Set web server port number
WiFiServer server(80); //EthernetServer server(80);
// Variable to store the HTTP request
String header;
String lamp26status="off";
void ConnectToWiFi()
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }

  Serial.print("Connected to the WiFi network , your ip adress is "); //+WiFi.localIP()
  Serial.println(WiFi.localIP());
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
            if (header.indexOf("GET /26/on") >= 0)
            {
              Serial.println("26 torned on");
              digitalWrite(26, HIGH);
              lamp26status="on";
              client.print(lamp26status);
            }
            else if (header.indexOf("GET /26/off") >= 0)
            {
              Serial.println(" 26 torned  off");
              digitalWrite(26, LOW);
              lamp26status="off";
              client.print(lamp26status);
            } 
            else if (header.indexOf("GET /status") >= 0)
            {
              client.print("{\"26\":\""+lamp26status+"\"}");
            } 
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
void setup()
{

  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(26, OUTPUT);
  // pinMode(27, OUTPUT);
  // Set outputs to LOW
  digitalWrite(26, LOW);
  // digitalWrite(27, LOW);
  ConnectToWiFi();
  server.begin();
}

void loop()
{

  ClientsListner();
}
