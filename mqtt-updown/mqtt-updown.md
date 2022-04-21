### **Layout pagina**

![motor](motor.png)


### **Struttura HTML**
```html
<div class="header">
	 <h1>Gestione luci soggiorno</h1>
</div>
<div class="grid-container">
	<div class="col-4 col-s-3 menu">
	</div>
	<div class="col-4 col-s-9 menu">
	<h1>Piantana</h1>
	<div id='form'>
		<form>
			<input id='up1' type='button' value='Button 1 UP' onmousedown='this.style.opacity="1"' onmouseup='this.style.opacity="0.6"' ontouchstart='this.style.opacity="1"' ontouchend='this.style.opacity="0.6"'>
			<br>
			<input id='down1' type='button' value='Button 1 DOWN' onmousedown='this.style.opacity="1"' onmouseup='this.style.opacity="0.6"' ontouchstart='this.style.opacity="1"' ontouchend='this.style.opacity="0.6"'>
			<br>
			<h1>Lampadario</h1>
			<input id='up2' type='button' value='Button 2 UP'  onmousedown='this.style.opacity="1"' onmouseup='this.style.opacity="0.6"' ontouchstart='this.style.opacity="1"' ontouchend='this.style.opacity="0.6"'>
			<br>
			<input id='down2' type='button' value='Button 2 DOWN' onmousedown='this.style.opacity="1"' onmouseup='this.style.opacity="0.6"' ontouchstart='this.style.opacity="1"' ontouchend='this.style.opacity="0.6"'>
			<br>
		</form>
	 </div>
	</div>
	<div class="col-3 col-s-12">
	</div>
</div>
<div class="footer">
	  <p>Resize the browser window to see how the content respond to the resizing.</p>
</div>
```

### **Libreria MQTT**

[Gateway mqtt](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/gateway.md)

### **Architettura di riferimento per IoT**

L'**architettura tipica del SW** a bordo di un dispositivo IoT è riassumibile:

<img src="iotarchitecture.png" alt="alt text" width="600">

Il **middleware** in genere è composto da **librerie di terze parti** da **compilare** all'interno di un **IDE** (ad es. Arduino) o all'interno di un **SDK** cioè un pacchetto di sviluppo per applicazioni che fornisce vari strumenti per compilazione, debug e documentazione (ad es. AWS IoT, Azure IoT, ESP-IDF). Oppure esistono **framework** specifici per IoT Open Source come RIOT che, una volta compilati su una macchina Linux, forniscono veri e propri **SO per IoT** con esattamente ciò che serve per la **comunicazione** di un certo dispositivo.

### **Client MQTT per ESP32, ESP8266 e Arduino con metodo di connessione bloccante**

In questo caso le librerie che realizzano lo stack TCP/IP e il livello fisico del WiFi sono implementati all'interno del chip che svolge le funzioni di modem, oppure, come nel caso delle MCU ESP8266 e ESP32, sono librerie SW eseguite su un sottosistema HW della stessa MCU con funzioni specifiche di comunicazione. Nel caso della MCU ESP8266 lo stack TCP/IP è eseguito dallo stesso core e all'interno dello stesso thread dell'applicazione principale. Nel caso della MCU ESP832 lo stack TCP/IP è eseguito in un core a parte e in un thread privilegiato rispetto a quello dell'applicazione principale. Il core applicativo è comunque responsabile della comunicazione di livello applicativo realizzata dal protocollo MQTT.  

Utilizza la libreria **arduino-mqtt** scaricabille da https://github.com/256dpi/arduino-mqtt e adatta sia per esp8266 che per esp32. Ha la particolarità (comune a quasi tutte le librerie MQTT) di avere la **funzione di connessione bloccante**, per cui, se il server è momentaneamente indisponibile, il client rimane fermo sulla funzione di connessione senza ritornare. Scaduto un **timeout** la connect() **ritorna** permettendo al task che la contiene di continuare l'esecuzione. Nell'esempio seguente la connect() è dentro la callback di un timer HW ed è attivata da una ISR associata ad un interrupt. Le ISR, per definizione, dovrebbero essere non bloccanti perchè spesso non sono interrompibili. **Bloccare una ISR** collegata ad **eventi core** della macchina può causare **instabilità** che determinano il **reset** del sistema.

E' adoperabile sulle seguenti schede:

- Arduino Yun & Yun-Shield (Secure)
- Arduino Ethernet Shield
- Arduino WiFi Shield
- Adafruit HUZZAH ESP8266 (Secure)
- Arduino/Genuino WiFi101 Shield (Secure)
- Arduino MKR GSM 1400 (Secure)
- ESP32 Development Board (Secure)

In realtà, il task principale che contiene il loop() su **ESP32** apparentemente **non si blocca** probabilmente perchè sta su un thread diverso da quello in cui sta la connect MQTT (forse anche un'altro core della CPU rispetto a quello del loop()). Nel caso di una CPU single core come ESP8266 il codice potrebbe essere modificato inserendo nella callback del timer un **flag** al posto della connect e nel loop un **check del flag** che chiama la connect se questo è vero. In questo modo verrebbe bloccato solo il loop principale per qualche secondo, tempo in cui il sistema è non responsivo ma comunque funzionante. Oppure si potrebbe sostituire il timer HW che comanda la riconnessione periodica del client MQTT con un timer SW basato sul polling della millis() nel loop.

Le **librerie** vanno scaricate e scompattate dentro la cartella **libraries** all'interno della **cartella di lavoro** di Arduino. Le librerie da scaricare devono avere il nome (eventualmente rinominandole): 
- arduino-mqtt

Si tratta di una libreria **molto leggera** che non occupa spazio eccessivo in memoria istruzioni. 

Due **timer HW** ([Gestione dei tempi con i timer HW](timersched.md)) sono stati utilizzati in **combinazione** con gli **eventi**  **onConnection** e **onDisconnection** del **WiFi** (ma **non** del **MQTT** per il quale si usa il **polling** dello **stato**), per realizzare i **tentativi periodici** di **riconnessione** in **assenza** di connessione **WiFi** e in assenza di un server **MQTT** raggiungibile:
- **wifiReconnectTimer** è un timer che viene avviato in concomitanza di un evento di perdita di connessione del WiFi (SYSTEM_EVENT_STA_DISCONNECTED). Il timer viene attivato in modo **once**, cioè una sola volta. Alla sua **scadenza** parte la **funzione di riconnessione** del WiFi. Se essa va a **buon fine**, genera l'evento SYSTEM_EVENT_STA_GOT_IP (onConnection) che attiva il timer del **polling periodico dello stato MQTT** tramite l'istruzione **mqttReconnectTimer.attach_ms(...)**. Se invece la funzione di riconnessione del WiFi **non va a buon fine** essa genera l'evento SYSTEM_EVENT_STA_DISCONNECTED (onDisconnection) che riattiva il timer di riconnessione WiFi ancora per una **sola volta** tramite l'istruzione **wifiReconnectTimer.once_ms(...)**. La **combinazione** di callback e attivazione del timer di ritrasmissione WiFi genera periodicamente nuovi **tentativi di riconnessione** fintanto che la connessione WiFi è assente. In caso di **onDisconnection** del WiFi, l'istruzione  **mqttReconnectTimer.detach()** serve ad evitare che nuovi tentativi di polling della connessione MQTT vengano eseguiti durante i periodi di disservizio del WiFi dato che, in questo caso, il server MQTT sarebbe comunque irraggiungibile.
- **mqttReconnectTimer** sempre attivo **quando è presente il WiFi**, serve a fare il **polling** dello **stato** della connessione **MQTT** tramite **mqttClient.connected()**. Se MQTT **è connesso** (quando mqttClient.connected() in mqttConnTest() restituisce true) non accade nulla, altrimenti viene lanciato un nuovo tentativo di riconnessione tramite connectToMqtt().



```C++
//#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <MQTT.h>
#include <Ticker.h>

#define WIFIRECONNECTIME  2000
#define MQTTRECONNECTIME  2000

Ticker mqttReconnectTimer;
Ticker wifiReconnectTimer;

const char ssid[] = "myssd";
const char pass[] = "mypsw";
const char mqttserver[] = "test.mosquitto.org";

//WiFiClientSecure net;
WiFiClient net;
MQTTClient mqttClient(1024);

unsigned long lastMillis = 0;
byte count = 0;

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

  // Note: Do not use the client in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `client.loop()`.
}

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
      mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
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
  Serial.println("Connecting to MQTT...");
  mqttClient.connect("bside2botham2", "try", "try");
  if(mqttClient.connected()){
	mqttClient.subscribe("/hello");
  }
  Serial.println("...end");
  // client.unsubscribe("/hello");
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  WiFi.onEvent(WiFiEvent);
  // MQTT brokers usually use port 8883 for secure connections.
  mqttClient.begin(mqttserver, 1883, net);
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

  // publish a message roughly every 2 second.
  if (millis() - lastMillis > 2000) {
        lastMillis = millis();
	
	Serial.println("Tick");
	// pubblicazione periodica di test
        mqttClient.publish("/hello", "world");
  }
}

/*
WL_CONNECTED: assigned when connected to a WiFi network;
WL_NO_SHIELD: assigned when no WiFi shield is present;
WL_IDLE_STATUS: it is a temporary status assigned when WiFi.begin() is called and remains active until the number of attempts expires (resulting in WL_CONNECT_FAILED) or a connection is established (resulting in WL_CONNECTED);
WL_NO_SSID_AVAIL: assigned when no SSID are available;
WL_SCAN_COMPLETED: assigned when the scan networks is completed;
WL_CONNECT_FAILED: assigned when the connection fails for all the attempts;
WL_CONNECTION_LOST: assigned when the connection is lost;
WL_DISCONNECTED: assigned when disconnected from a network;
\*/
```


### **Formato JSON ingressi**

```C++
{"devid":"soggiorno-gruppo06","up1":"255"}
{"devid":"soggiorno-gruppo06","down1":"255"}

```

### **Formato JSON feedback**

```C++
{"devid":"soggiorno-gruppo06","up1":"255","down1":"0","dr1":"1"}
{"devid":"soggiorno-gruppo06","up1":"0","down1":"255","dr1":"-1"}
{"devid":"soggiorno-gruppo06","up2":"0","down2":"0","dr2":"0"}
```
### **Metodi di base comuni a tutti gli oggetti IOT**

- ```cmdParser(str,payload,"cmd",MAXLEN)```. Ricerca un certo commando ```cmd``` all’interno di una stringa e ne restituisce il valore sotto forma di stringa sul parametro di out str. Ritorna ```true``` se ha trovato un'occorenza del comando, ```false``` altrimenti.
- ```processCmd(String id, String payload)```. Elabora la richiesta remota interpretando la stringa json del messaggio in base al tipo di dispositivo IOT.

### **Metodi specifici di Motor**

- ```Motor(String id, uint8_t startIndex)```. Costruttore. P1: devid univoco, P2: indice dispositivo nel gruppo (0,1,2,...)
- ```remoteCntrlUp(void)```. Pulsante marcia avanti, attiva il motore nella direzione 1 se il motore è fermo, lo blocca se il motore è in marcia.
- ```remoteCntrlDown(void)```. Pulsante marcia indietro, attiva il motore nella direzione -1 se il motore è fermo, lo blocca se il motore è in marcia.
- ```remoteConf(void)```. Interroga lo stato corrente del dispositivo.
- ```remoteCntrlEventsParser()```. Genratore di eventi di feedback ed azioni esterne a partire da ingressi e stato corrente.
- ```onAction(SweepCallbackSimple cb)```. Definisce la callback delle azioni esterne.

### **Struttura sketch Arduino**

```C++
Motor m1(mqttid,0);

void setup() {
	//inizializza la coda con il valore 0 su tutte le celle
	m1.onFeedback(feedbackAction);
	m1.onAction(motorAction1);
	.........................
	mqttClient.onMessage(messageReceived); 
}

void loop() {
	mqttClient.loop();
	//delay(10);  // <- fixes some issues with WiFi stability
	m1.remoteCntrlEventsParser();
	// schedulatore eventi dispositivo
	// pubblica lo stato dei pulsanti dopo un minuto
	if (millis() - lastMillis > STATEPERIOD) {
		lastMillis = millis();
		
		if(mqttClient.connected()){
			Serial.println("Ritrasm. periodica stato: ");
			m1.remoteConf();
		}
	}
}

void feedbackAction(String buf){
	mqttClient.publish(outtopic, buf);
};
void motorAction1(int enabled, int dir, uint8_t n){
	Serial.println("Enabled " + String(enabled) + " - dir: " +  String(dir)+ " - n: " +  String(n));
};
void messageReceived(String &topic, String &payload) {
	Serial.println("incoming: " + topic + " - " + payload);
	// Note: Do not use the client in the callback to publish, subscribe or
	// unsubscribe as it may cause deadlocks when other things arrive while
	// sending and receiving acknowledgments. Instead, change a global variable,
	// or push to a queue and handle it in the loop after calling `client.loop()`.
	
	if(topic == intopic){
		m1.processCmd(mqttid, payload, MAXLEN);
		m2.processCmd(mqttid, payload, MAXLEN);
	}
};
```

### **Callback MQTT corta**

```C++
void messageReceived(String &topic, String &payload) {
	Serial.println("incoming: " + topic + " - " + payload);
	// Note: Do not use the client in the callback to publish, subscribe or
	// unsubscribe as it may cause deadlocks when other things arrive while
	// sending and receiving acknowledgments. Instead, change a global variable,
	// or push to a queue and handle it in the loop after calling `client.loop()`.
	
	//if(topic == intopic){
		m1.processCmd(mqttid, payload, MAXLEN);
	//}
};
```
    

### **Callback MQTT lunga**

```C++
/////// gestore messaggi MQTT in ricezione (callback)     
void messageReceived(String &topic, String &payload) {
	Serial.println("incoming: " + topic + " - " + payload);
	// Note: Do not use the client in the callback to publish, subscribe or
	// unsubscribe as it may cause deadlocks when other things arrive while
	// sending and receiving acknowledgments. Instead, change a global variable,
	// or push to a queue and handle it in the loop after calling `client.loop()`.
	
	//if(topic == intopic){
		String str;	
		// COMMANDS PARSER /////////////////////////////////////////////////////////////////////////////////////////////
		// ricerca all'interno del payload l'eventuale occorrenza di un comando presente in un set predefinito 
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		m1.cmdParser(str,payload,"devid",MAXLEN);
		if(str == mqttid){		
		    if(m1.cmdParser(str,payload,"up1",MAXLEN)){
				m1.remoteCntrlUp();
			}
			if(m1.cmdParser(str,payload,"down1",MAXLEN)){
				m1.remoteCntrlDown();
			}
			if(m2.cmdParser(str,payload,"up2",MAXLEN)){
				m2.remoteCntrlUp();
			}
			if(m2.cmdParser(str,payload,"down2",MAXLEN)){
				m2.remoteCntrlDown();
			}
			if(payload.indexOf("\"conf\":\"255\"") >= 0){
				m1.remoteConf();
				m2.remoteConf();
			}
		}
	//}
};
```
