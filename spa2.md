>[Torna all'indice generale](README.md)

## **PARTE DINAMICA DELLA PAGINA**

E' la porzione della pagina che viene modificata **più volte** recuperando i vari contenuti da visualizzare tramite **richieste specifiche** (utilizzando il protocollo MQTT) senza ricaricare nuovamente il contenuto statico della pagina.

Consiste generalmente in codice **javascript** che riguarda principalmente:
- definizione dei messaggi JSON da inviare per ogni input
- gli **ascoltatori degli eventi** di input (click su tasti o inserimento su caselle di testo, ecc.)
- **callback del protocollo di comunicazione** adottato per il recupero dei singoli dati, nel nostro caso MQTT.
- eventuali timer per la **simulazione** dell'evoluzione del tempo delle uscite variabili nel tempo (ad es. apertura di un cancello)















>[Torna all'indice generale](README.md)
