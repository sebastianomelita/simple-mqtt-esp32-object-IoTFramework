>[Torna all'indice generale](README.md)

## **PARTE DINAMICA DELLA PAGINA**

E' la porzione della pagina che viene modificata **più volte** recuperando i vari contenuti da visualizzare tramite **richieste specifiche** (utilizzando il protocollo MQTT) senza ricaricare nuovamente il contenuto statico della pagina.

Consiste generalmente in codice **javascript** che riguarda principalmente:
- **messaggi** JSON da inviare per ogni input
- gli **ascoltatori degli eventi** di input (click su tasti o inserimento su caselle di testo, ecc.)
- **callback del protocollo di comunicazione** adottato per il recupero dei singoli dati, nel nostro caso MQTT.
- **codice di impostazione** dei parametri del canale di comunicazione quali indirizzi IP, indirizzi di porta o topic MQTT.
- eventuali timer per la **simulazione** dell'evoluzione del tempo delle uscite variabili nel tempo (ad es. apertura di un cancello)

### **Messaggi JSON**

Per scelta progettuale, ogni messaggio di comando deve riportare l'**indirizzo applicativo MQTT** del dispositivo più la coppia **```tag_comando:valore_comando```**.

```javascript
var vls = ['{"devid":"'+mqttid+'","up1":"255"}','{"devid":"'+mqttid+'","down1":"255"}','{"devid":"'+mqttid+'","conf":"255"}'];
```
I messaggi sono raccolti in uno o più array e recuperati mediante un indice che rappresenta la loro posizion nell'arraye. Per quanto riguarda i **pulsanti** la scelta organizzativa adoperata è di tenere vicini i messaggi di un gruppo facendo seguire quelli di un eventuale altro gruppo. Ad esempio, le funzioni "up1" e "down1" del motore 1 sono raggruppate nelle locazion 0 e 1 dell'array, a seguire si troveranno le funzioni "up2" e "down2" del motore 2.

### **Ascoltatori di eventi di input**

Per scelta progettuale, le **callback** degli eventi di input non sono inserite nei tag degli elementi HTML della pagina ad esempio con codice tipo ```<button onclick="myFunction()">Click me</button>``` ma si è preferito registrare delle callback inline come **ascoltatori** di eventi specifici di un certo oggetto del DOM della pagina HTML (stile java). 

Il **riferimento** alla rappresentazione ad oggetti di un elemento HTML si recupera tramite la funzione ```document.getElementById('elem_id')``` dove ```elem_id``` è l'attributo univoco **id** del tag HTML di cui si vuole "ascoltare" un certo evento.


```javascript
var up1=document.getElementById('up1');
var dw1=document.getElementById('down1');
up1.addEventListener('click', function(){press(vls[0]);});
dw1.addEventListener('click', function(){press(vls[1]);});
```

### **Callback di ricezione feedback**

I messaggi di feedback sono **obbligatori** da parte del dispositivo perchè hanno l'importante funzione di **confermare**, a livello applicativo, l'effettiva **applicazione del comando**. Per quanto è possibile, dovrebbero riportare lo **stato effettivo delle porte** di uscita del dispositivo, possibilmente leggendone il reale valore. Al ricevimento delle informazioni di feedback queste devono essere **smistate** all' elemento HTML di input corrispondente al comando a cui esse si riferiscono. Il feedback può coambiare un elemento decorativo quale **colore** di un tasto o **posizione** di un cursore, oppure riportare un **valore numerico o testuale** in una **casella di testo**.

I messaggi di feddback, come quelli di comando, sono codificati in JSON e possono essere interpretati da un **parser** fornito dalla classe statica javascript JSON. 

```javascript
function onRcv(d) {
	var obj = JSON.parse(d);
	for(var x in obj){
		var el = document.getElementById(x);
		if(el != null){  //controlla se il campo esiste nel DOM della pagina
			//console.log(x);
			if(x=='up1' || x=='down1' || x=='up2' || x=='down2'){
				console.log('stato:'+obj[x]); 
				if(Number(obj[x]) == 255){
					el.style.backgroundColor = "#b30000";
				}else{
					el.style.backgroundColor = "#00ccff";
				}
			}
			//{"devid":"pippo","up1":"0","sp1":"10000","tr1":"50"}
			//{"devid":"pippo","up1":"1","sp1":"10000","tr1":"10","pr1":"10"}
			//errori nel json staccano la connessione!!
		};
	}
}
```
L'istruzione 
```javascript 
var obj = JSON.parse(d);
```
recupera un oggetto javascript avente per campi i campi della stringa JSON. I **campi** possono essere:
- nomi corrispondenti ed uguali ad **elementi HTML di input** esistenti nella pagina quando il feedback si riderisce ad uno di questi
- nomi non corrispondenti a nessun elemento HTML della pagina che, piuttosto che rappresentare il feedback di un input, rappresentano il feedback di qualche **oggetto di simulazione**.

Il ciclo
```javascript 
for(var x in obj){
	var el = document.getElementById(x);
	if(x=='up1' || x=='down1' || x=='up2' || x=='down2'){
		if(Number(obj[x]) == 255){
			el.style.backgroundColor = "#b30000";
		}else{
			el.style.backgroundColor = "#00ccff";
		}
	}
}
```
- **scorre** i campi dell'oggetto
- ne **recupera il nome** nella variabile ```x```
- **filtra** in base al nome del campo quelli che devono produrre un qualche effetto nella pagina tramite un costrutto ```if-then-else```
- **recupera il valore** del campo trasformandolo in numerico tramite ```Number(obj[x])```
- **recupera il riferimento** dell'elemento HTML in base al nome del campo tramite var ```el = document.getElementById(x);```
- **modifica la grafica** dell'elemento HTML accedendo alle proprietà dell'oggetto mediante il riferimento, ad esempio cambiando lo sfondo di un tasto ```el.style.backgroundColor = "#b30000";```

### **Codice di impostazione canale**

Serve ad impostare i parametri della comunicazione tra la pagina e il broker MQTT:
```javascript 
var mqttid = "soggiorno-gruppo06";
var mqttAddr = "broker.hivemq.com";
var wsport = "8000";
var mqttProto ="/mqtt"; //default
var intopic = "soggiorno/in";
var outtopic = "soggiorno/out";

start();
		function start(){
			var clientID = mqttid+'_' + parseInt(Math.random() * 100);
			conn = new Paho.MQTT.Client(mqttAddr, Number(wsport), mqttProto,clientID);
			//client = new Paho.MQTT.Client("iot.eclipse.org", Number(443), "/wss");
			//"var conn = new Paho.MQTT.Client('{MA}', Number('{MT}'), clientID);"
			console.log(conn);
			console.log('broker:'+mqttAddr+', port:'+wsport+', path:'+mqttProto+', id:'+clientID);
			// connect the client
			conn.connect(
				{
					cleanSession : false, 
					onSuccess : onConnect,
					onFailure : onFailed,
					keepAliveInterval: 30
				});
			function onFailed(err) {
				//"conn.send('Connect ' + new Date());"
				console.log('Initial connect request failed. Error message : ' + err.errorMessage); 
			};
			function onConnect(resObj) {
				//"conn.send('Connect ' + new Date());"
				console.log('onConnect');
				conn.subscribe(outtopic);
				console.log('Connected to ' + resObj.uri);
				if (resObj.reconnect){
					console.log('It was a result of automatic reconnect.');
				}
				// carica parametri di configurazione
				press(vls[4]);
			};
			conn.onMessageArrived = function (e) {
				console.log('Received: '+e.payloadString);
				if(e.payloadString){
					var obj = JSON.parse(e.payloadString);
					//controlla se è indirizzato al dispositivo associato alla pagina
					if(obj.devid==''+mqttid+''||obj.devid=='FF'){//FF id broadcast valido per tutti i dispositivi
						console.log('Received: '+e.payloadString);
						onRcv(e.payloadString);
					}
				}else{
					console.log('Message received error: '+e.payloadString);
				}
			};
			conn.onConnectionLost  = function (resObj) {		
				//console.log('Lost connection to ' + resObj.uri + ' Error code: ' + resObj.errorCode + ' Error text: ' + resObj.errorMessage);
				if (resObj.reconnect){
					console.log('Automatic reconnect is currently active.');
				}else{
					console.log('Lost connection to host, reconnect in 5 seconds');
					// Try to reconnect in 5 seconds
					setTimeout(function(){start()}, 5000);
				}
			};
		};
		function send(str){
		    console.log('send: '+str);
			var msg = new Paho.MQTT.Message(str);
			msg.destinationName = intopic;
			console.log('topic: '+msg.destinationName);
			conn.send(msg);
		};
		function press(s){
			send(s);
		};
```

>[Torna all'indice generale](README.md)
