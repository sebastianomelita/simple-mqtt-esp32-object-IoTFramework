### **Layout pagina**

![motor](motor.png)

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
