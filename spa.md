>[Torna all'indice generale](README.md)


### **Pagine responsive**

Il **design responsivo**, o responsive web design (RWD), indica una tecnica di web design per la realizzazione di siti in grado di **adattarsi graficamente** in modo automatico al dispositivo coi quali vengono visualizzati (computer con diverse risoluzioni, tablet, smartphone, cellulari, web tv), riducendo al minimo la necessità dell'utente di ridimensionare e scorrere i contenuti.

Le **Media Queries** (conosciute anche come Media Query) sono dichiarazioni CSS con le quali è possibile identificare il tipo di dispositivo o una sua caratteristica allo scopo di applicare stili o condizioni differenti in base ad un elenco di regole.

**Media query**

```CSS
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
  ```

**Sitografia:**
- https://www.4next.eu/news/mqtt-capitolo-2/
- https://it.wikipedia.org/wiki/Design_responsivo

>[Torna all'indice generale](README.md)
