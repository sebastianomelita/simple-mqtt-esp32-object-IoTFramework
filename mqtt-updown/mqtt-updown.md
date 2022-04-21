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
### **Parser dei comandi**

- ```cmdParser(str,payload,"cmd",MAXLEN)```. Ricerca un certo commando ```cmd``` all’interno di una stringa e ne restituisce il valore sotto forma di stringa sul parametro di out str. Ritorna ```true``` se ha trovato un'occorenza del comando, ```false``` altrimenti.
- ```processCmd(String id, String payload)```. Elabora la richiesta remota interpretando la stringa json del messaggio in base al tipo di dispositivo IOT.


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

### **Metodi**

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
		m2.processCmd(mqttid, payload, MAXLEN);
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
