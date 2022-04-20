### **Metodi**

```Motor(String id, uint8_t startIndex)```. Costruttore. P1: id_mqtt univoco, P2: indice dispositivo nel gruppo (0,1,2,...)
```remoteCntrlUp(void)```. Pulsante marcia avanti, attiva il motore nella direzione 1 se il motore è fermo, lo blocca se il motore è in marcia.
```remoteCntrlDown(void)```. Pulsante marcia indietro, attiva il motore nella direzione -1 se il motore è fermo, lo blocca se il motore è in marcia.
```remoteConf(void)```. Interroga lo stato corrente del dispositivo.
```remoteCntrlEventsParser()```. Genratore di eventi di feedback ed azioni esterne a partire da ingressi e stato corrente.
```onAction(SweepCallbackSimple cb)```. Definisce la callback delle azioni esterne.


### **Callback MQTT**

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
