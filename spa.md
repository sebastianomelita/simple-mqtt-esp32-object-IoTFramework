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

Esempio ```soggiorno.html```

```html
<!DOCTYPE html>
<html>
<head>
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
</head>
<body>	
	<div class="header"><h1>Gestione luci soggiorno</h1></div>
	<div class="grid-container">
		<div class="col-4 col-s-3 luci"><h1>Lume</h1></div>
		<div class="col-4 col-s-9 luci"><h1>Piantana</h1></div>
		<div class="col-4 col-s-12 luci"><h1>Lampadario</h1</div>
	</div>
	<div class="footer"><p>Pagina responsiva.</p>
	</div>
</html>
```
[soggiorno](https://sebastianomelita.github.io/simple-mqtt-esp32-object-IoTFramework/soggiorno.html)



Lo spazio in questo contesto conviene misurarlo in unità frazionarie, dove 1fr rappresenta una parte dello spazio disponibile. 

**Sitografia:**
- https://www.4next.eu/news/mqtt-capitolo-2/
- https://it.wikipedia.org/wiki/Design_responsivo

>[Torna all'indice generale](README.md)
