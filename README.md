# simple-mqtt-esp32-object-IoTFramework

### **Single Page Application (SPA)**

Una SPA è sostanzialmente un'applicazione web (WEB app)  cioè un sito web che può essere usato o consultato su una singola pagina web con l'obiettivo di fornire una esperienza utente più fluida e simile alle applicazioni desktop dei sistemi operativi tradizionali. 

La pagina della web app è divisa sostanzialmente in due parti:
- **sezioni statiche** comuni a tutti i contenuti che vengono caricate da un **server web** con una normale **richiesta HTTP** e sono composte da codice HTML, CSS e javascript statico. Costituiscono il **layout di base** della pagina.
- **sezioni dinamiche** specifiche i cui contenuti possono essere caricati da server diversi con **protocolli eterogenei** ed **assemblati localmente** dal client nel layout statico della pagina. Per recuperare i contenuti sono comuni richieste **HTTP** dinamiche in tecnologia **AJAX** (e suoi derivati quali JQUERY e Angular) ma anche richieste **MQTT** e **COAP**.

 ![spacicle](Traditional-vs-SPA-lifecycle.ppm)
 
Il ciclo di vita di una SPA è:
1) Richiesta iniziale del layout comune
2) una o più richieste dei contenuti specifici via AJAX o MQTT o COAP
3) interpretazione della risposta in formato JSON o XML
4) modifica, mediante programmazione lato client in javascript, del modello ad oggetti della pagina (DOM), direttamente mediante comandi di manipolazione degli elementi HTML (getElementById() e appendChild()) o indirettamente mediante stampa condocument.write(), per la l'inserimento dei contenuti dinamici nelle sezioni della pagina statica.

