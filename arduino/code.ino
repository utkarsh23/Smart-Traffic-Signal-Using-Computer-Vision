#include <ESP8266WiFi.h>

// Wifi hotspot credentials
const char* ssid = "";
const char* password = "";

// PIN numbers
int GreenLED = 15;
int YellowLED = 12;
int RedLED = 13;
int isBlocked = 0;
int MAXOUT = 0;
int isItGreen = 1;

int firstIndex, spaceIndex, secondSpaceIndex, count;

long greenTime = 0;
int greenTimeout = 1;

float avg = -1;
int tempavg = 0;

WiFiServer server(80);

void setup() {
    Serial.begin(115200);
    delay(10);

    pinMode(GreenLED, OUTPUT);
    pinMode(YellowLED, OUTPUT);
    pinMode(RedLED, OUTPUT);
    digitalWrite(GreenLED, HIGH);

    // Connect to WiFi network
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");

    // Start the server
    server.begin();
    Serial.println("Server started");

    // Print the IP address
    Serial.print("Use this URL : ");
    Serial.print("http://");
    Serial.print(WiFi.localIP());
    Serial.println("/");

    greenTime = millis();
}

void loop() {
    // Check if a client has connected
    WiFiClient client = server.available();
    if (!client) {
        return;
    }

    // Wait until the client sends some data
    Serial.println("new client");
    while (!client.available()) {
        delay(1);
    }

    // Read the first line of the request
    String request = client.readStringUntil('\r');
    client.flush();

    // Match the request and implement optimal
    // signal change strategy based off change in
    // avg vehicle count
    if ((!greenTimeout && !isBlocked) && (request.indexOf("/?COUNT=") != -1)) {
        Serial.println(request);
        firstIndex = request.indexOf("/?COUNT=") + 8;
        spaceIndex = request.indexOf(" ");
        secondSpaceIndex = request.indexOf(" ", spaceIndex+1);
        count = request.substring(firstIndex, secondSpaceIndex).toInt();
        if (MAXOUT < 10) {
            tempavg += count;
        }
        if (MAXOUT == 10) {
            avg = tempavg / 10.0;
            tempavg = 0;
        }
        MAXOUT += 1;
        if (MAXOUT > 100) {
            MAXOUT = 0;
        }
        Serial.println(count);
        Serial.println(avg);
        if (avg != -1) {
            if ((count < (avg)) && isItGreen) {
                greenToRed();
            }
            else if ((count > (avg + (0.2 * avg))) && !isItGreen) {
                redToGreen();
            }
        }
    } else if (greenTimeout && ((millis() - greenTime) >= 10000)) {
        greenTimeout = 0;
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("");
    client.println("<!DOCTYPE HTML>");
    client.println("<html>Success");
    client.println("</html>");

    delay(1); 
}

void redToGreen() {
    isBlocked = 1;

    digitalWrite(RedLED, LOW);
    digitalWrite(YellowLED, HIGH);
    delay(1000);
    digitalWrite(YellowLED, LOW);
    digitalWrite(GreenLED, HIGH);

    greenTime = millis();
    greenTimeout = 1;

    isItGreen = 1;

    isBlocked = 0;
}

void greenToRed() {
    isBlocked = 1;

    digitalWrite(GreenLED, LOW);
    digitalWrite(YellowLED, HIGH);
    delay(1000);
    digitalWrite(YellowLED, LOW);
    digitalWrite(RedLED, HIGH);

    isItGreen = 0;

    isBlocked = 0;
}
