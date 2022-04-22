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
I messaggi sono raccolti in uno o più array e recuperati mediante un indice che rappresenta la loro posizion nell'arraye. Per quanto riguarda i **pulsanti** la scelta organizzativa adoperata è di tenere vicini i messaggi di un gruppo facendo seguire quelli di un eventuale altro gruppo. Ad esempio Le funzioni "up1" e "down1" del motore 1 sono raggruppate nelle locazion 0 e 1 dell'array, a seguire si troveranno le funzioni "up2" e "down2" del motore 2.

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



```javascript
function onRcv(d) {
	//document.getElementById('p').innerHTML = f.data;\n"
	var obj = JSON.parse(d);
	console.log('Arrived data');
	for(var x in obj){
		//console.log('x:'+x);
		var el = document.getElementById(x);
		//console.log('el:'+el);
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








>[Torna all'indice generale](README.md)
