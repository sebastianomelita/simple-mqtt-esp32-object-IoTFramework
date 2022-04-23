//#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <MQTT.h>
#include <Ticker.h>
#include "RemoteControl.h"

#define WIFIRECONNECTIME  2000
#define MQTTRECONNECTIME  2000
#define NLEVEL1		9
#define NLEVEL2		100
#define NLEVEL3		9
#define NLEVEL4		100
#define MAXLEN		20
#define STATEPERIOD 60000
// the number of the LED pin
#define LED1 				16  // GPIO 16
#define LED2				17  // GPIO 17
#define LED3				5   // GPIO 5
#define LED4				18  // GPIO 10 ?
// setting PWM properties
#define FREQ 				5000
#define LEDCHANNEL1 		0
#define LEDCHANNEL2 		1
#define LEDCHANNEL3 		2
#define LEDCHANNEL4 		3
#define RESOLUTION 			8

Ticker mqttReconnectTimer;
Ticker wifiReconnectTimer;

/********* FINE DEFINIZIONE SEGNALI **************************/
const char ssid[] = "xxxxx";
const char pass[] = "yyyyy";
const char mqttserver[] = "broker.hivemq.com";
const int mqttport = 1883;
const char intopic[] = "soggiorno/in"; 
const char outtopic[] = "soggiorno/out"; 
const char mqttid[] = "soggiorno-gruppo06"; 

//WiFiClientSecure net;
WiFiClient wifi;
MQTTClient mqttClient(1024);

Slider sld1(mqttid,0,NLEVEL1);
Slider sld2(mqttid,1,NLEVEL2);
Slider sld3(mqttid,2,NLEVEL3);
Slider sld4(mqttid,3,NLEVEL4);

unsigned long lastMillis = 0;
byte count = 0;
String buf;
/////// FINE FUNZIONI DI GESTIONE DEI COMANDI REMOTI   /////////////////////////////////////////////////////////////
////   GESTIONE WIFI E MQTT    /////////////////////////////////////////////////////////////////////////////////////
void WiFiEvent(WiFiEvent_t event) {
	Serial.printf("[WiFi-event] event: %d\n", event);
	switch(event) {
	case SYSTEM_EVENT_STA_GOT_IP:
	  Serial.println("WiFi connected");
	  Serial.println("IP address: ");
	  Serial.println(WiFi.localIP());
	  connectToMqtt();
	  mqttReconnectTimer.attach_ms(MQTTRECONNECTIME, mqttConnTest);
	  break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
	  Serial.println("WiFi lost connection");
	  mqttReconnectTimer.detach(); // ensure we don"t reconnect to MQTT while reconnecting to Wi-Fi
	  wifiReconnectTimer.once_ms(WIFIRECONNECTIME, connectToWifi);
	  break;
	}
}

void mqttConnTest() {
    if (!mqttClient.connected() && WiFi.status() == WL_CONNECTED) {
		Serial.print("MQTT lastError: ");
		Serial.println(mqttClient.lastError());
		connectToMqtt();
    }
}

void connectToWifi() {
	Serial.println("Connecting to Wi-Fi...");
	WiFi.mode(WIFI_STA);
	//WiFi.disconnect();
	WiFi.begin(ssid, pass);
}

void connectToMqtt() {
	Serial.print("Connecting to MQTT...");
	Serial.print("with client id: ");
	Serial.println(mqttid);
	mqttClient.connect(mqttid, "tryy", "tryy");
	if(mqttClient.connected()){
		Serial.print("Subscribing: ");
		mqttClient.subscribe(intopic);
		Serial.println(intopic);
	}
	Serial.println("...end");
	// client.unsubscribe("/hello");
}
////  FINE GESTIONE WIFI E MQTT  //////////////////////////////////////////////////////////////////////////////////
void PWMInit(){
  // configure LED PWM functionalitites
  ledcSetup(LEDCHANNEL1, FREQ, RESOLUTION);
  ledcSetup(LEDCHANNEL2, FREQ, RESOLUTION);
  ledcSetup(LEDCHANNEL3, FREQ, RESOLUTION);
  ledcSetup(LEDCHANNEL4, FREQ, RESOLUTION);
  
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(LED1, LEDCHANNEL1);
  ledcAttachPin(LED2, LEDCHANNEL2);
  ledcAttachPin(LED3, LEDCHANNEL3);
  ledcAttachPin(LED4, LEDCHANNEL4);
}

void setup() {
	PWMInit();
	sld1.onAction(sldAction);
	sld2.onAction(sldAction);
	sld3.onAction(sldAction);
	sld4.onAction(sldAction);
	sld1.onFeedback(feedbackAction);
	sld2.onFeedback(feedbackAction);
	sld3.onFeedback(feedbackAction);
	sld4.onFeedback(feedbackAction);
	Serial.begin(115200);
	Serial2.begin(9600);
	WiFi.begin(ssid, pass);
	WiFi.onEvent(WiFiEvent);
	// MQTT brokers usually use port 8883 for secure connections.
	mqttClient.begin(mqttserver, mqttport, wifi);
	mqttClient.onMessage(messageReceived); 
	connectToWifi();
	count = 0;
	while (WiFi.status() != WL_CONNECTED && count < 10) {
		delay(500);
		count++;
		Serial.print(".");
	}
}

void loop() {
	mqttClient.loop();
	//delay(10);  // <- fixes some issues with WiFi stability
	sld1.remoteCntrlEventsParser();
	sld2.remoteCntrlEventsParser();
	sld3.remoteCntrlEventsParser();
	sld4.remoteCntrlEventsParser();
	// schedulatore eventi dispositivo
	// pubblica lo stato dei pulsanti dopo un minuto
	if (millis() - lastMillis > STATEPERIOD) {
		lastMillis = millis();
		
		if(mqttClient.connected()){
			Serial.println("Ritrasm. periodica stato: ");
			sld1.remoteConf();
			sld2.remoteConf();
			sld3.remoteConf();
			sld4.remoteConf();
		}
	}
}
/// INIZIO CALLBACKS UTENTE  /////////////////////////////////////////////////////////////////////////////////////
/// CALCOLO USCITE DELLA FUNZIONE DI SCIVOLAMENTO (callback) 
void sldAction(int outr, int cr, uint8_t n){
	//mqttClient.publish(outtopic, buf);
	Serial.println("Out " + String(n) + " - cr: " +  String(cr)+ " - n: " +  String(n));
	ledcWrite(n, cr);
};
void feedbackAction(String buf){
	Serial.println("buf " + buf);
	mqttClient.publish(outtopic, buf);
};
/////// gestore messaggi MQTT in ricezione (callback)     
void messageReceived(String &topic, String &payload) {
	Serial.println("incoming: " + topic + " - " + payload);
	// Note: Do not use the client in the callback to publish, subscribe or
	// unsubscribe as it may cause deadlocks when other things arrive while
	// sending and receiving acknowledgments. Instead, change a global variable,
	// or push to a queue and handle it in the loop after calling `client.loop()`.
	
	//if(topic == intopic){
		//String str;		
		sld1.processCmd(mqttid, payload, MAXLEN);
		sld2.processCmd(mqttid, payload, MAXLEN);
		sld3.processCmd(mqttid, payload, MAXLEN);
		sld4.processCmd(mqttid, payload, MAXLEN);
		// COMMANDS PARSER /////////////////////////////////////////////////////////////////////////////////////////////
		// ricerca all'interno del payload l'eventuale occorrenza di un comando presente in un set predefinito 
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/*sld1.cmdParser(str,payload,"devid",MAXLEN);
		if(str == mqttid){		
		    if(sld1.cmdParser(str,payload,"sld1",MAXLEN)){
				sld1.remoteSlider(atoi(str.c_str()));
			}
			if(sld2.cmdParser(str,payload,"sld2",MAXLEN)){
				sld2.remoteSlider(atoi(str.c_str()));
			}
			if(sld3.cmdParser(str,payload,"sld3",MAXLEN)){
				sld3.remoteSlider(atoi(str.c_str()));
			}
			if(sld4.cmdParser(str,payload,"sld4",MAXLEN)){
				sld4.remoteSlider(atoi(str.c_str()));
			}
			if(payload.indexOf("\"conf\":\"255\"") >= 0){
				sld1.remoteConf();
				sld2.remoteConf();
				sld3.remoteConf();
				sld4.remoteConf();
			}
		}*/
	//}
};


////   FINE CALLBACKS UTENTE   ////////////////////////////////////////////////////////////////////////////////////////////////
/*
WL_CONNECTED: assigned when connected to a WiFi network;
WL_NO_SHIELD: assigned when no WiFi shield is present;
WL_IDLE_STATUS: it is a temporary status assigned when WiFi.begin() is called and remains active until the number of attempts expires (resulting in WL_CONNECT_FAILED) or a connection is established (resulting in WL_CONNECTED);
WL_NO_SSID_AVAIL: assigned when no SSID are available;
WL_SCAN_COMPLETED: assigned when the scan networks is completed;
WL_CONNECT_FAILED: assigned when the connection fails for all the attempts;
WL_CONNECTION_LOST: assigned when the connection is lost;
WL_DISCONNECTED: assigned when disconnected from a network;

typedef enum {
  LWMQTT_SUCCESS = 0,
  LWMQTT_BUFFER_TOO_SHORT = -1,
  LWMQTT_VARNUM_OVERFLOW = -2,
  LWMQTT_NETWORK_FAILED_CONNECT = -3,
  LWMQTT_NETWORK_TIMEOUT = -4,
  LWMQTT_NETWORK_FAILED_READ = -5,
  LWMQTT_NETWORK_FAILED_WRITE = -6,
  LWMQTT_REMAINING_LENGTH_OVERFLOW = -7,
  LWMQTT_REMAINING_LENGTH_MISMATCH = -8,
  LWMQTT_MISSING_OR_WRONG_PACKET = -9,
  LWMQTT_CONNECTION_DENIED = -10,
  LWMQTT_FAILED_SUBSCRIPTION = -11,
  LWMQTT_SUBACK_ARRAY_OVERFLOW = -12,
  LWMQTT_PONG_TIMEOUT = -13,
\*/
