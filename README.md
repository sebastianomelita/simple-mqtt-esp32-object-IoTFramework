# simple-mqtt-esp32-object-IoTFramework

### **Single Page Application (SPA)**

Una SPA è sostanzialmente un'applicazione web (WEB app)  cioè un sito web che può essere usato o consultato su una singola pagina web con l'obiettivo di fornire una esperienza utente più fluida e simile alle applicazioni desktop dei sistemi operativi tradizionali. 

La pagina della web app è divisa sostanzialmente in **due parti** non necessariamente contigue:
- **sezioni statiche** comuni a tutti i contenuti che vengono caricate da un **server web** con una normale **richiesta HTTP** e sono composte da codice HTML, CSS e javascript statico. Costituiscono il **layout di base** della pagina.
- **sezioni dinamiche** specifiche i cui contenuti possono essere caricati da server diversi con **protocolli eterogenei** ed **assemblati localmente** dal client nel layout statico della pagina. Per recuperare i contenuti sono comuni richieste **HTTP** dinamiche in tecnologia **AJAX** (e suoi derivati quali JQUERY e Angular) ma anche richieste **MQTT** e **COAP**.

 ![spacicle](Traditional-vs-SPA-lifecycle.ppm)
 
Il **ciclo di vita** di una SPA tipicamente consiste in:
1) **Richiesta iniziale** del layout comune statico
2) **acquisizione degli input** utente mediante caselle di testo o pulsanti solitamente, in ambito domotico, gestiti tramite **ascoltatori di eventi** realizzati in javascript
3) **una o più richieste** dei contenuti specifici aventi come **parametri** gli input e veicolate via AJAX o MQTT o COAP
4) **interpretazione della risposta** in formato JSON o XML
5) **inserimento dei contenuti dinamici** nelle sezioni della pagina statica mediante codice lato client in javascript che modifica il **modello ad oggetti** della pagina (DOM) o **direttamente** mediante comandi di manipolazione della **rappresentazione ad oggetti** degli elementi HTML (getElementById() e appendChild()) o **indirettamente** mediante iniezione con document.write() **di tag html e istruzioni CSS** dinamici che si sovrappongono a quelli statici del layout.

Il **modello di applicazione** proposto nel **progetto** consiste in un una **costellazione di dispositivi** IOT dotati di client MQTT per operazioni di **pubblicazione** e **notifica** di contenuti e in un **server centrale** con funzione di **server web** per le pagine statiche delle web app e con funzione di **broker MQTT** per lo smistamento delle comunicazioni reciproche tra dispositivi IOT e tra dispositivi IOT e web apps.

