# simple-mqtt-esp32-object-IoTFramework

### **Single Page Application (SPA)**

Una SPA è sostanzialmente un'applicazione web (WEB app)  cioè un sito web che può essere usato o consultato su una singola pagina web con l'obiettivo di fornire una esperienza utente più fluida e simile alle applicazioni desktop dei sistemi operativi tradizionali. 

La pagina della web app è divisa sostanzialmente in **due parti** non necessariamente contigue:
- **sezioni statiche** comuni a tutti i contenuti che vengono caricate da un **server web** con una normale **richiesta HTTP** (POST o GET) e sono composte da codice HTML, CSS e javascript statico. Costituiscono il **layout di base** della pagina.
- **sezioni dinamiche** specifiche i cui contenuti possono essere caricati da server diversi con **protocolli eterogenei** ed **assemblati localmente** dal client nel layout statico della pagina. Per recuperare i contenuti sono comuni richieste **HTTP** dinamiche in tecnologia **AJAX** (e suoi derivati quali JQUERY e Angular) ma anche richieste **MQTT** e **COAP**.

Spesso le **WebApp** sono assimilabili alle tradizionali **applicazioni distribuite** desktop in virtù della **bidirezionalità del canale** tra le due componenti client e server. Nelle **applicazioni desktop** il **canale bidirezionale** è (a basso livello) basato sulle API standard dei **socket BSD**. Nella applicazioni web app il canale bidirezionale è ormai quasi sempre realizzato con le **API websocket** e raramente ormai con tecniche di **long polling** del server.

 ![spacicle](Traditional-vs-SPA-lifecycle.ppm)
 
Il **ciclo di vita** di una SPA tipicamente consiste in:
1) **Richiesta iniziale** del layout comune statico
2) **acquisizione degli input** utente mediante caselle di testo o pulsanti solitamente, in ambito SPA, gestiti tramite **ascoltatori di eventi** realizzati in javascript
3) **una o più richieste** dei contenuti specifici aventi come **parametri** gli input e veicolate via AJAX o MQTT o COAP (invece delle solite richieste POST dei form)
4) **interpretazione della risposta** in formato JSON o XML
5) **inserimento dei contenuti dinamici** nelle sezioni della pagina statica mediante codice lato client in javascript che modifica il **modello ad oggetti** della pagina (DOM) o **direttamente** mediante comandi di manipolazione della **rappresentazione ad oggetti** degli elementi HTML (getElementById() e appendChild()) o **indirettamente** mediante iniezione con document.write() **di tag html e istruzioni CSS** dinamici che si sovrappongono a quelli statici del layout.

**Gestione dello stato del dispositivo**

Lo **stato della pagina** è correlato allo stato del dispositivo. Per la **memorizzazione** e per la **gestione** dello stato del dispositivo sono possibili, in linea di principio, **3 possibilità**:
- lo stato del dispositivo è **gestito dal dispositivo** e dal codice che implementa le sue funzioni
- lo stato del dispositivo è **gestito dalla pagina** e dal codice che implementa le sue funzioni
- lo stato del dispositivo è **gestito sia dal codice del dispositivo che da quello della pagina** e sono tra loro **sincronizzati**

Al di la dei vantaggi e degli svantaggi di ciascuna soluzione, per scelta progettuale, si è stabilito di far calcolare e gestire l'**evoluzione temporale dello stato** dei controlli **al dispositivo**, in particolare al codice che, dentro un dispositivo, implementa quei controlli.
 
Quindi, il dispositivo è **stateful** mentre la pagina è di base **stateless** e si limita ad inviare i **comandi** al dispositivo che, li tratterà come degli **ingressi**. Il **dispositivo**, noti gli **ingressi** e il valore corrente dello **stato**, sarà in grado di calcolare le **nuove uscite** ed il **nuovo stato**. Se la **pagina** ha bisogno di conoscere lo stato del dispositivo per visualizzarlo allora utilizzera per questo scopo i messaggi di **feedback** provenienti dal dispositivo a conferma di ogni comando.

Sono possibili **situazioni particolari** in cui la pagina è costretta a calcolare localmente, in maniera autonoma, una **replica** dello stato del dispositivo. Ciò accade quando le **uscite** assumono un **andamento variabile** nel tempo **prestabilito** (sweep o scivolamento) che deve essere **riprodotto** nella pagina per evitare di effettuare un **polling** continuo dello stato delle uscite. In questo caso:
- la **pagina** calcola lo stato delle uscite con la **stessa funzione temporale** utilizzata dal dispositivo, in pratica realizza una vera e propria **emulazione locale** del comportamento del dispositivo.
- il **dispositivo** invia almeno **due feedback**: uno **iniziale** con il quale parte l'evoluzione locale dell'uscita (sweep) ed uno **finale** con il quale si interrompe l'evoluzione. Il **feedback finale** porta con se anche il **valore vero finale** dell'uscita con il quale viene eventualmente corretto il valore **stimato localmente** dalla pagina. Il feedback ha quindi due funzioni: **sincronizzazione** di emulatore e dispositivo e **verifica** dello stato finale.

**Persistenza dello stato del dispositivo**

Lo stato di un dispositivo benchè sotto la responsabilità del codice in esso implementato, **non è persistente** perchè memorizzazto in **memoria RAM**. La RAM è **volatile** e, in qualsiasi, situazione che porti ad uno spegnimento involontario del dispositivo o ad un suo riavvio perde il suo contenuto. Un'alterntiva potrebbe essere memorizzare ogni nuovo stato nella memoria persistente del dispositivo, cioè la **EEPROM**.

Per scelta progettuale, si è stabilito di lasciare i dispositivi **senza persistenza** dello stato al fine di massimizzare la loro **semplicità**. La **persistenza** dello stato per cui è delegata ad un **dispositivo unico cetralizzato** per **tutti** i dispositivi che ha il compito di memorizzarlo su un **datatbase locale** ed, eventualmente, per maggiore sicurezza, anche su un servizio di **storage in cloud**. 

**Ogni dispositivo**, allorquando calcola il valore nuovo dello stato, è tenuto a **notificare** la modifica al server di persistenza **via MQTT** in maniera tale che questo possa conservarlo usando come chiave di identificazione il MQTT_ID del dispositivo. Se un dispossitivo, viene **sostiyuito** o viene **riavviato** si **inizializza** caricando l'**ultimo stato valido** presente nel database centrale.

**Architettura dell'applicazione**

Il **modello di applicazione** proposto nel **progetto** consiste in un una **costellazione di dispositivi** IOT dotati di client MQTT per operazioni di **pubblicazione** e **notifica** di contenuti e in un **server centrale** con funzione di **server web** per le pagine statiche delle web app e con funzione di **broker MQTT** per lo **smistamento** delle comunicazioni reciproche tra dispositivi IOT e tra dispositivi IOT e web apps.

### **Broker MQTT**

MQTT Fornisce un **canale multicast** di livello applicativo (L7) tramite cui un utente col ruolo di publisher è in grado di notificare una replica dello stesso messaggio a più subscribers.

<img src="broker.png" alt="alt text" width="700">

Il canale multicast è realizzato da un dispositivo intermedio detto **broker** che realizza un routing di livello applicativo (L7) delle informazioni, smistandole in base ad una **etichetta** di destinazione detta **topic**.

L’**indirizzamento** fisico dei dispositivi può essere o solo L2 (MAC) o sia L2 che L3 (IP) ma è **implicito** e noto solo al broker. I client per inviare un messaggio non sono tenuti a conoscere l’indirizzo fisico degli altri interlocutori ma solo quello del **broker** ed il **topic**. 

Gli utenti si comportano tutti come **client** poiché sono loro che **iniziano la connessione** con il broker e non il viceversa, ma si dividono in **due ruoli**:
- **PUBLISHER** coloro che inviano le informazioni. Un publisher deve solo connettersi, **decidere il topic** e **inviare** l’informazione.
- **SUBSCRIBERS** coloro che ricevono le informazioni. Un subscriber deve preventivamente **registrarsi** ad uno o più topic presso il server affinchè possa **ricevere** l’informazione quando questa verrà pubblicata.

### **Fasi MQTT**
<img src="sequenza_mqtt.png" alt="alt text" width="700">

Le **fasi** scambio delle informazioni via MQTT tipicamente sono:
1) Il **Subscriber** dichiara presso il broker il proprio interesse a ricevere notifiche riguardo ad un certo argomento (topic)
2) il **publisher** pubblica un messaggio che riguarda un certo topic
3) Il **broker** inoltra il messaggio a tutti i subscriber interessati al suo topic

Realizazione componenti:
- [SPA di comando: parte statica](spa.md)
- [SPA di comando: parte dinamica](spa2.md)
- [Gateway MQTT](gateway.md)

Definizione oggetti IOT:
- [mqtt-updown](mqtt-updown/mqtt-updown.md)




