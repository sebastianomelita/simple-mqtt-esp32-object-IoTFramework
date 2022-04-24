//#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <MQTT.h>
#include <Ticker.h>
#include "RemoteControl.h"

#define WIFIRECONNECTIME  2000
#define MQTTRECONNECTIME  2000
#define LEDSOGGIORNO1 13
#define LEDSOGGIORNO2 12
#define MAXT1 		10000
#define MAXT2 		10000
#define NLEVEL1		9
#define NLEVEL2		100
#define MAXLEN		20
#define STATEPERIOD 60000
#define SHARPNESS1	20
#define SHARPNESS2	20

Ticker mqttReconnectTimer;
Ticker wifiReconnectTimer;

/********* FINE DEFINIZIONE SEGNALI **************************/
const char ssid[] = "xxxx";
const char pass[] = "yyyy";
const char mqttserver[] = "broker.hivemq.com";
const int mqttport = 1883;
const char intopic[] = "soggiorno/in"; 
const char outtopic[] = "soggiorno/out"; 
const char mqttid[] = "soggiorno-gruppo06"; 

//WiFiClientSecure net;
WiFiClient wifi;
MQTTClient mqttClient(1024);
//DimmeredToggle sw1(mqttid,0,SHARPNESS1,NLEVEL1,MAXT1);
//DimmeredToggle sw2(mqttid,1,SHARPNESS2,NLEVEL2,MAXT2);

DimmeredToggle sw1(mqttid,0,SHARPNESS1,NLEVEL1,MAXT1);
DimmeredToggle sw2(mqttid,1,SHARPNESS2,NLEVEL2,MAXT2);

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
void setup() {
	//inizializza la coda con il valore 0 su tutte le celle
	sw1.onSweep(sweepAction1);
	sw2.onSweep(sweepAction2);
	sw1.onFeedback(feedbackAction);
	sw2.onFeedback(feedbackAction);
	pinMode(LEDSOGGIORNO1, OUTPUT);
	pinMode(LEDSOGGIORNO2, OUTPUT);
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
	sw1.remoteCntrlEventsParser();
	sw2.remoteCntrlEventsParser();
	// schedulatore eventi dispositivo
	// pubblica lo stato dei pulsanti dopo un minuto
	if (millis() - lastMillis > STATEPERIOD) {
		lastMillis = millis();
		
		if(mqttClient.connected()){
			Serial.println("Ritrasm. periodica stato: ");
			sw1.remoteConf();
			sw2.remoteConf();
		}
	}
}
/// INIZIO CALLBACKS UTENTE  /////////////////////////////////////////////////////////////////////////////////////
/// CALCOLO USCITE DELLA FUNZIONE DI SCIVOLAMENTO (callback) 
void feedbackAction(String buf){
	mqttClient.publish(outtopic, buf);
};
void sweepAction1(int outr, int cr, uint8_t n){	
	Serial.println((String) "r: "+cr);
	if(cr > 0){
		Serial2.println((String)"@l"+cr);
		Serial.println((String)"@l"+cr);
	}else{
		Serial2.println((String)"@lo");
		Serial.println((String)"@lo");
	}
};
void sweepAction2(int outr, int cr, uint8_t n){	
	Serial.println((String) "r: "+cr);
	if(cr > 0){
		//Serial2.println((String)"@l"+cr);
		Serial.println((String)"@l"+cr);
	}else{
		//Serial2.println((String)"@lo");
		Serial.println((String)"@lo");
	}
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
		sw1.processCmd(mqttid, payload, MAXLEN);
		sw2.processCmd(mqttid, payload, MAXLEN);
		// COMMANDS PARSER /////////////////////////////////////////////////////////////////////////////////////////////
		// ricerca all'interno del payload l'eventuale occorrenza di un comando presente in un set predefinito 
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/*sw1.cmdParser(str,payload,"devid",MAXLEN);
		if(str == mqttid){		
		    if(sw1.cmdParser(str,payload,"to1",MAXLEN)){
				sw1.remoteToggle(255);
			}
			if(sw2.cmdParser(str,payload,"to2",MAXLEN)){
				sw2.remoteToggle(255);
			}
			if(sw1.cmdParser(str,payload,"on1",MAXLEN)){
				sw1.remoteCntrlOn(atoi(str.c_str()));
			}
			if(payload.indexOf("\"off1\":\"255\"") >= 0){
				sw1.remoteCntrlOff();
			}
			if(sw2.cmdParser(str,payload,"on2",MAXLEN)){
				sw2.remoteCntrlOn(atoi(str.c_str()));
			}
			if(payload.indexOf("\"off2\":\"255\"") >= 0){
				sw2.remoteCntrlOff();
			}
			if(sw1.cmdParser(str,payload,"sld1",MAXLEN)){
				sw1.remoteSlider(atoi(str.c_str()));
			}
			if(sw2.cmdParser(str,payload,"sld2",MAXLEN)){
				sw2.remoteSlider(atoi(str.c_str()));
			}
			if(payload.indexOf("\"conf\":\"255\"") >= 0){
				sw1.remoteConf();
				sw2.remoteConf();
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
