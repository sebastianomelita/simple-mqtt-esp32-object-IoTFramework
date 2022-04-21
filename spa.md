>[Torna all'indice generale](README.md)


### **Pagine responsive**

Il **design responsivo**, o responsive web design (RWD), indica una tecnica di web design per la realizzazione di siti in grado di **adattarsi graficamente** in modo automatico al dispositivo coi quali vengono visualizzati (computer con diverse risoluzioni, tablet, smartphone, cellulari, web tv), riducendo al minimo la necessità dell'utente di ridimensionare e scorrere i contenuti.

**Media query**

Le **Media Queries** (conosciute anche come Media Query) sono dichiarazioni CSS con le quali è possibile identificare il tipo di dispositivo o una sua caratteristica allo scopo di applicare stili o condizioni differenti in base ad un elenco di regole.

```CSS
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
body {
  background-color: yellow;
}

@media only screen and (min-width: 600px) {
  body {
    background-color: lightblue;
  }
}

@media only screen and (min-width: 768px) {
  body {
    background-color: red;
  }
}
</style>
</head>
<body>

<h1>The @media Rule</h1>

</body>
</html>
  ```

### **Grid layout**


**Sitografia:**
- https://www.4next.eu/news/mqtt-capitolo-2/
- https://it.wikipedia.org/wiki/Design_responsivo

>[Torna all'indice generale](README.md)
