
#include <WiFi.h>
#include "wifi_console.h"

const char* ssid     = "SeaWolfNetwork";
const char* password = "sysoev1984";

//WiFiServer server(23);  // port must be 23
WiFiConsole g_wifiConsole;

bool WiFiConsole::on_init_process(void* param) {
    
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    
    m_pConsTask = GetConsoleInstance();
    m_pConsTask->init();
    m_taskFunction = &CConsole::processStream;

    
    return true;
}


bool WiFiConsole::run_task() {
 
    WiFiServer server(m_port);
    server.begin();
    while(true) {  
        taskDelayMs(1);
        WiFiClient client = server.available();   // listen for incoming clients
        if (client) {                             // if you get a client,
            m_pConsTask->setStreamInterface(&client);

            while (client.connected()) {            // loop while the client's connected
                (m_pConsTask->*m_taskFunction)();
                taskDelayMs(1);
            }
    // close the connection:
            client.stop();
        }
    }
    return true;
}
