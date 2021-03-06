
>[Torna all'indice generale](../README.md)

# **COMANDO TOGGLE**

### **Utilizzo**

La pagina ha **4 pulsanti** con funzioni di **toggle** divisi in **4 gruppi**. Un gruppo comanda l'accensione/spegnimento di una singola uscita. Le uscite **non** sono direttamente **attivate** dalla libreria perchè sono lasciate volutamente **generiche** in quanto potrebbero attivare, a seconda dei casi, una porta digitale, un comando MODBUS, un comando sulla seriale, ecc.

Il **toggle** è un pulsante con stato che viene invertito ad ogni pressione del pulsante stesso. Lo stato del pulsante **non** è conservato nella pagina ma solamente nel dispositivo. Lo stato viene recuperato per conferma dopo ogni pressione mediante un messaggio di feedback.

Fasi:
- **Premendo** un qualsiasi **pulsante Toggle** l'uscita commuta dal valore precedente, ```1``` se era ```0```, ```0``` se era ```1```.


### **Layout pagina**

La **pagina** ```mqttupdown.html``` può essere caricata sul browser da una cartella sul PC o può essere caricata su un server web di pagine statiche.

Il **layout** è statico, responsivo, diviso in colonne ed è definito mediante il seguente CSS di **tipo GRID**:

```css
<style>
	.grid-container {
		display: grid;
		grid-template-columns: 1fr;
	}

	table {
		width: 100%;
	}

	@media only screen and (min-width: 600px) {
	  /* For tablets: */
	  .grid-container {
		  grid-template-columns: 1fr 1fr 1fr 1fr 1fr 1fr 1fr 1fr 1fr 1fr 1fr 1fr;
	  }
	  .col-s-1 {grid-column: span 1;}
	  .col-s-2 {grid-column: span 2;}
	  .col-s-3 {grid-column: span 3;}
	  .col-s-4 {grid-column: span 4;}
	  .col-s-5 {grid-column: span 5;}
	  .col-s-6 {grid-column: span 6;}
	  .col-s-7 {grid-column: span 7;}
	  .col-s-8 {grid-column: span 8;}
	  .col-s-9 {grid-column: span 9;}
	  .col-s-10 {grid-column: span 10;}
	  .col-s-11 {grid-column: span 11;}
	  .col-s-12 {grid-column: span 12;}
	}

	@media only screen and (min-width: 768px) {
	  /* For desktop: */
	  .grid-container {
		  grid-template-columns: 1fr 1fr 1fr 1fr 1fr 1fr 1fr 1fr 1fr 1fr 1fr 1fr;
	  }
	  .col-1 {grid-column: span 1;}
	  .col-2 {grid-column: span 2;}
	  .col-3 {grid-column: span 3;}
	  .col-4 {grid-column: span 4;}
	  .col-5 {grid-column: span 5;}
	  .col-6 {grid-column: span 6;}
	  .col-7 {grid-column: span 7;}
	  .col-8 {grid-column: span 8;}
	  .col-9 {grid-column: span 9;}
	  .col-10 {grid-column: span 10;}
	  .col-11 {grid-column: span 11;}
	  .col-12 {grid-column: span 12;}
	}
	.luci{
		border: 1px solid black;
	}
</style>
```

![toggle](toggle.png)

### **Struttura HTML**

La **pagina statica** ha tre sezioni una sotto l'altra:
- un **header** con il titolo e le informazioni comuni alla pagina da visualizzare per prima.
- un **corpo** organizzato mediante un layout a colonne responsive di tipo grid che contiene gli elementi HTML di **comando** con gli input e le informazioni di output.
- un **footer**. che contiene informazioni generali da visualizzare per ultime.

La definizione della **gerarchia di elementi** contenitori e contenuti del corpo dei comandi:

```html
<div class="header">
	  <h1>Gestione luci soggiorno</h1>
</div>
<div class="grid-container">
	<div class="col-4 col-s-3 menu">
	</div>
	<div class="col-4 col-s-9 menu">
	 <h1>Controllo luci</h1>
	 <div id='form'>
		<form>
			<label for='to1'>Soggiorno<label>
			<input id='to1' name='to1' type='button' value='Toggle1' onmousedown='this.style.opacity="1"' onmouseup='this.style.opacity="0.6"' ontouchstart='this.style.opacity="1"' ontouchend='this.style.opacity="0.6"'>
			<br>
			<label for='to2'>Cucina<label>
			<input id='to2' name='to2' type='button' value='Toggle2' onmousedown='this.style.opacity="1"' onmouseup='this.style.opacity="0.6"' ontouchstart='this.style.opacity="1"' ontouchend='this.style.opacity="0.6"'>
			<br>
			<label for='to3'>Camera da letto<label>
			<input id='to3' name='to3' type='button' value='Toggle3' onmousedown='this.style.opacity="1"' onmouseup='this.style.opacity="0.6"' ontouchstart='this.style.opacity="1"' ontouchend='this.style.opacity="0.6"'>
			<br>
			<label for='to4'>Bagno<label>
			<input id='to4' name='to4' type='button' value='Toggle4' onmousedown='this.style.opacity="1"' onmouseup='this.style.opacity="0.6"' ontouchstart='this.style.opacity="1"' ontouchend='this.style.opacity="0.6"'>
		</form>
	 </div>
	</div>
	<div class="col-4 col-s-12">
	</div>
</div>
<div class="footer">
	  <p>Resize the browser window to see how the content respond to the resizing.</p>
</div>
```

### **Callback uscite**

- ```outr```. Stato del pulsante. Valore ```0``` o ```1``` 
- ```cr```. Stato del pulsante. Valore ```0``` o ```100``` 
- ```n```. Numero del pulsante (inizia da 0).
- 
```C++
void tglAction1(int outr, int cr, uint8_t n){
	digitalWrite(LED1,outr);
};
```

### **Formato JSON ingressi**

```C++
// ricarica della pagina
{"devid":"soggiorno-gruppo06","conf":"255"}
//pressione peulsante 1
{"devid":"soggiorno-gruppo06","to1":"255"}
//ripressione peulsante 1
{"devid":"soggiorno-gruppo06","to1":"255"}
```

### **Formato JSON feedback**

```C++
// feedback pulsante 1 acceso
{"devid":"soggiorno-gruppo06","to1":"1"}
// feedback pulsante 1 spento
{"devid":"soggiorno-gruppo06","to1":"0"}
// ricarica pagina o ritrasm. periodica stato
{"devid":"soggiorno-gruppo06","to1":"0"}
{"devid":"soggiorno-gruppo06","to2":"0"}
{"devid":"soggiorno-gruppo06","to3":"0"}
{"devid":"soggiorno-gruppo06","to4":"0"}
```
### **Metodi di base comuni a tutti gli oggetti IOT**

- ```cmdParser(str,payload,"cmd",MAXLEN)```. Ricerca un certo commando ```cmd``` all’interno di una stringa e ne restituisce il valore sotto forma di stringa sul parametro di out str. Ritorna ```true``` se ha trovato un'occorenza del comando, ```false``` altrimenti.
- ```processCmd(String id, String payload)```. Elabora la richiesta remota interpretando la stringa json del messaggio in base al tipo di dispositivo IOT.

### **Metodi specifici di Toggle**

- ```Toggle(String id,uint8_t startIndex)```. Costruttore. P1: devid univoco, P2: indice dispositivo nel gruppo (0,1,2,...)
- ```remoteToggle(void)```. Pulsante marcia avanti, attiva il motore nella 
- ```onAction(SweepCallbackSimple cb)```. Definisce la callback delle azioni esterne.

### **Struttura sketch Arduino**

Lo **sketch** ```mqtt-toggle.ino``` deve essere aperto con l'IDE di Arduino e caricato sul dispositivo ESP32 dopo aver selezionato correttamente la scheda e la porta della seriale e, chiaramente, dopo aver connesso il dispositivo alla porta usb del PC.

```C++
Toggle tgl1(mqttid,0);
Toggle tgl2(mqttid,1);
Toggle tgl3(mqttid,2);
Toggle tgl4(mqttid,3);

void setup() {
	tgl1.onAction(tglAction1);
	tgl2.onAction(tglAction2);
	tgl3.onAction(tglAction3);
	tgl4.onAction(tglAction4);
	tgl1.onFeedback(feedbackAction);
	tgl2.onFeedback(feedbackAction);
	tgl3.onFeedback(feedbackAction);
	tgl4.onFeedback(feedbackAction);
	.........................
	mqttClient.onMessage(messageReceived); 
}

void loop() {
	mqttClient.loop();
	//delay(10);  // <- fixes some issues with WiFi stability
	tgl1.remoteCntrlEventsParser();
	tgl2.remoteCntrlEventsParser();
	tgl3.remoteCntrlEventsParser();
	tgl4.remoteCntrlEventsParser();
	// schedulatore eventi dispositivo
	// pubblica lo stato dei pulsanti dopo un minuto
	if (millis() - lastMillis > STATEPERIOD) {
		lastMillis = millis();
		
		if(mqttClient.connected()){
			Serial.println("Ritrasm. periodica stato: ");
			tgl1.remoteConf();
			tgl2.remoteConf();
			tgl3.remoteConf();
			tgl4.remoteConf();
		}
	}
}

void tglAction1(int outr, int cr, uint8_t n){
	digitalWrite(LED1,outr);
};
void tglAction2(int outr, int cr, uint8_t n){
	digitalWrite(LED2,outr);
};
void tglAction3(int outr, int cr, uint8_t n){
	digitalWrite(LED3,outr);
};
void tglAction4(int outr, int cr, uint8_t n){
	digitalWrite(LED4,outr);
};
void feedbackAction(String buf){
	Serial.println("buf " + buf);
	mqttClient.publish(outtopic, buf);
};
void messageReceived(String &topic, String &payload) {
	Serial.println("incoming: " + topic + " - " + payload);
	// Note: Do not use the client in the callback to publish, subscribe or
	// unsubscribe as it may cause deadlocks when other things arrive while
	// sending and receiving acknowledgments. Instead, change a global variable,
	// or push to a queue and handle it in the loop after calling `client.loop()`.
	
	//if(topic == intopic){
		//String str;	
		tgl1.processCmd(mqttid, payload, MAXLEN);
		tgl2.processCmd(mqttid, payload, MAXLEN);
		tgl3.processCmd(mqttid, payload, MAXLEN);
		tgl4.processCmd(mqttid, payload, MAXLEN);
	//}
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
		tgl1.cmdParser(str,payload,"devid",MAXLEN);
		if(str == mqttid){		
		    if(tgl1.cmdParser(str,payload,"to1",MAXLEN)){
				tgl1.remoteToggle();
			}
			if(tgl2.cmdParser(str,payload,"to2",MAXLEN)){
				tgl2.remoteToggle();
			}
			if(tgl3.cmdParser(str,payload,"to3",MAXLEN)){
				tgl3.remoteToggle();
			}
			if(tgl4.cmdParser(str,payload,"to4",MAXLEN)){
				tgl4.remoteToggle();
			}
			if(payload.indexOf("\"conf\":\"255\"") >= 0){
				tgl1.remoteConf();
				tgl2.remoteConf();
				tgl3.remoteConf();
				tgl4.remoteConf();
			}
		}
	//}
};
```

>[Torna all'indice generale](../README.md)
