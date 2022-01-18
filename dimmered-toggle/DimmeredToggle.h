//#include <WiFiClientSecure.h>
//#define MAXQUEUE	4

/************** DEFINIZIONE USCITE **************************/
enum outs // indice uscite
{
	OUT1			=0, // btn1, btn2
	OUT3			=1, // toggle1
	OUT4			=2, // slider1
	NOUT			=3
};
/********* FINE DEFINIZIONE USCITE **************************/
/************** DEFINIZIONE STATI **************************/
enum btnstates // stato pulsanti 
{
	STBTN1			=0, 
	STBTN2			=1,
	STBTN3			=2,
	STTGL1			=3,
	NSTATES         =4
};
/************** DEFINIZIONE SEGNALI **************************/
enum signals // segnali tra timer callbacks e loop() (flags)
{
	SGNBTN1			=0, 
	SGNBTN2			=1,
	SGNTGL1			=2,
	SGNSLD1 		=3,
	SGNBTNRST1		=4,
	SGNBTNRST3		=5,
	SGNINIT			=6,
	SGNOP			=7,
	NSGN        	=8
};

typedef void (*SweepCallbackSimple)(int outr[], int cr[], bool stop[], uint8_t state[], byte n);
typedef void (*FeedbackCallbackSimple)(String buf);
//String v[MAXQUEUE];  // coda messaggi
	
class DimmeredToggle
{
    private:
		Ticker sweepTimer[NOUT];
		uint8_t k;
		String mqttid;
		unsigned long maxt[NOUT];
		unsigned nstep[NOUT];
		int outval[NOUT];
		int countr[NOUT];
		long t[NOUT];
		unsigned long tstep[NOUT];
		unsigned long prec_t[NOUT];
		long target_p[NOUT];
		long target_tt[NOUT];
		long targetbis[NOUT];
		long midtarget[NOUT];
		int direct[NOUT];
		bool stop[NOUT];
		uint8_t sharp[NOUT];
		uint8_t stato[NSTATES];
		bool signal[NSGN];
		SweepCallbackSimple sweepCallback = nullptr;
		FeedbackCallbackSimple feedbackCallback = nullptr;
		uint8_t sweeepParam;
		
		void remoteCntrlInit() {
			for(int i=0; i<NSGN; i++){
			  signal[i] = false;
			}
			for(int i=0; i<NSTATES; i++){
			  stato[i] = 0;
			}
			for(int i=0; i<NOUT; i++){
				outval[i] = 0;
				countr[i] = 0;
				t[i] = 0;
				tstep[i] = 0;
				target_p[i] = 0;
				target_tt[i] = 0;
				targetbis[i] = 0;
				direct[i] = 0;
				stop[i] = true; // lo scivolamento deve essere sbloccato da un tasto o da un cursore
				sharp[i] = 2;
			}
		}	
		void sweep(uint8_t n) {
			float half = tstep[n] / sharp[n];
			if(stop[n]==false && (direct[n]>0 && t[n] < (target_tt[n]-half) || direct[n]<0 && t[n] >= (target_tt[n]+half))){
				//t[n]=t[n]+direct[n]*tstep[n];
				unsigned long tnow = millis();
				t[n]=t[n]+direct[n]*(tnow-prec_t[n]);
				prec_t[n] = tnow; 
				//Serial.println("t:"+t[n]);
				//Serial.print("target_tt+:");
				//Serial.println(target_tt[n]+half);
				//Serial.print("target_tt-:");
				//Serial.println(target_tt[n]-half);
				outval[n] = (float) t[n]/maxt[n]*100;
				countr[n] = (float) t[n]/tstep[n];
				(*sweepCallback)(outval,countr,stop,stato,n);
				//Serial.println("++++++++++++++");
				//Serial.println((String) "t:"+t[n]+" pr.value:"+outval[n]+" stop: "+String(stop[n])+" dir:"+direct[n]+" target:"+target_tt[n]+" tmax:"+maxt[n]+" tstep:"+tstep[n]+" n:"+n+" countr:"+countr[n]);
			}else{
				sweepTimer[n].detach();
				Serial.println("STOP WWEEP");
				direct[n]=0;
				stop[n]=true;
				signal[SGNBTNRST1+n] = true;
			}
			sleep(0);
		}
		static void staticSweep(DimmeredToggle *pThis) {
			pThis->sweep(pThis->sweeepParam);
		}
		// il target_tt si fornisce in percentuale intera di 100 (ad es. 80)
		void startSweep(unsigned nsteps,unsigned delay,unsigned long tmax,unsigned short n) {
			nstep[n]=nsteps;
			maxt[n]=tmax;
			prec_t[n]=millis();
			tstep[n] = (float) tmax/nstep[n]; //durata di uno step
			if(tstep[n] > 0){
				target_tt[n] = (float) target_p[n]/100*tmax;	
				if(!stop[n]){
					if(target_tt[n] >= t[n]){
						direct[n] = 1;
					}else{
						direct[n] = -1;
					}
					if(t[n]<=0)t[n]=1;
					if(t[n]>0){
						tstep[n]=tmax/nstep[n]; //durata di uno step
						Serial.println((String) "tstep0: "+tstep[n]+" stop0: "+String(stop[n])+" target_t0: "+String(target_tt[n])+" maxt0: "+maxt[n]+" dir0: "+direct[n]+" tnow0: "+t[n]+" nstep0: "+nstep[n]+" n0: "+n);
						sweepTimer[n].detach();
						//sweepAction(n);
						Serial.println("START SWEEP");
						//sweepTimer[n].attach_ms<uint8_t>(tstep[n], sweep, n);
						//sweepTimer[n].attach_ms<uint8_t>(tstep[n], +[](DimmeredToggle* obj) { o->sweep(); }, this);
						sweeepParam = n;
						sweepTimer[n].attach_ms(tstep[n], &DimmeredToggle::staticSweep, this);
					};
				}
			}else{
				//t[n] = target[n];
				if(target_tt[n] > 0){
					direct[n] = 1;
				}else{
					direct[n] = -1;
				}
				//direct[n]=0;
				Serial.println("Special sweep");
				Serial.println(direct[n]);
				outval[n] = target_p[n];
				countr[n] = (float) (target_p[n] + 1) / 100 * nstep[n];
				(*sweepCallback)(outval,countr,stop,stato,n);
				stop[n]=true;
			}
			Serial.println("NEXT TO START");
		};
		// codifica stop/start e direzione della barra per la SPA
		int webdir(uint8_t n) {
			int val=0;
			if(!stop[n]){
				val = direct[n];
			}
			return val;
		}
		// gestore comandi singolo slider (logica senza stato)
		void remoteSlider(uint8_t targetval, uint8_t sgnsld, uint8_t n){
			stop[n] = false;
			target_p[n] = targetval;
			targetbis[n] = targetval;
			signal[sgnsld] = true;
		}
		// gestore comandi singolo pulsante toggle, calcola lo stato del pulsante toggle
		void remoteToggle(uint8_t targetval, uint8_t sttgl, uint8_t sgntgl, uint8_t n){
			if(targetval == 255){
				signal[sgntgl] = true;
				stop[n] = true;   // normalmente ? alternativo allo scivolamento per cui lo blocca
				stato[sttgl] = !stato[sttgl];		
			}
		}
		// gestore comandi coppia di pulsanti on/off o up/down (gestisce anche un'eventuale slider associato alla coppia)
		// calcola lo stato dei due pulsanti 
		void remoteCntrl(uint8_t targetval, uint8_t stbtna, uint8_t stbtnb, uint8_t sgnbtna, uint8_t sgnsld, uint8_t n, uint8_t mult=1){
			
			if(targetval == 255){
				targetval = 100;
				signal[sgnbtna] = true;
				if(stop[n]){
					stop[n] = false;
					target_p[n] = targetval*mult;
					targetbis[n] = targetval;
					stato[stbtna] = 255;
				}else{
					stop[n] = true;
					stato[stbtna] = LOW;
					stato[stbtnb] = LOW;
				}
			}else{
				if(stop[n] == false){
					stop[n] = true;
					stato[stbtna] = LOW;
					stato[stbtnb] = LOW;
				}else{
					stop[n] = false;
					target_p[n] = targetval;
					targetbis[n] = targetval;
					signal[sgnsld] = true;
				}
			}
		}
		/////    GESTORE EVENTI (callback)    /////////////////////////////////////////////////////////////////////////////////
		String getToggleFeedback(uint8_t toggleState, uint8_t n){
			String str = "{\"devid\":\""+String(mqttid)+"\",\"to"+String(n+1)+"\":\""+String(toggleState)+"\"}";
			//Serial.println("Str: " + str);
			return str;
		}
		String getSliderFeedback(uint8_t target, uint8_t n){
			String str = "{\"devid\":\""+String(mqttid)+"\",\"pr"+String(n+1)+"\":\""+String(target)+"\"}";
			//Serial.println("Str: " + str);
			return str;
		}
		String getFadedSliderActionFeedback(long targetp, short dir, long t, uint8_t n){
			String str = "{\"devid\":\""+String(mqttid)+"\",\"pr"+String(n+1)+"\":\""+String(targetp)+"\",\"dr"+String(n+1)+"\":\""+String(dir)+"\",\"tr"+String(n+1)+"\":\""+String(t)+"\"}";
			Serial.println("Str: " + str);
			return str;
		}
		String getFadedSliderSweepStopFeedback(short dir, long t, uint8_t n){
			String str = "{\"devid\":\""+String(mqttid)+"\",\"dr"+String(n+1)+"\":\""+String(dir)+"\",\"tr"+String(n+1)+"\":\""+String(t)+"\"}";
			Serial.println("Str: " + str);
			return str;
		}
		String getFadedSliderSweepInitFeedback(long t, short dir, unsigned long maxt, unsigned nstep, uint8_t n){
			String str =  "{\"devid\":\""+String(mqttid)+"\", \"sp"+String(n+1)+"\":\""+maxt+"\",\"dr"+String(n+1)+"\":\""+String(dir)+"\",\"nl"+String(n+1)+"\":\""+nstep+"\",\"tr"+String(n+1)+"\":\""+String(t)+"\"}";
			Serial.println("Str: " + str);
			return str;
		}
		String getDimmeredToggleFeedback(uint8_t on, uint8_t off, uint8_t to, long t, short dir, unsigned nstep, unsigned long maxt, uint8_t n){
			String str =  "{\"devid\":\""+String(mqttid)+"\",\"on"+String(n+1)+"\":\""+on+"\",\"off"+String(n+1)+"\":\""+off+"\",\"to"+String(n+1)+"\":\""+to+"\", \"sp"+String(n+1)+"\":\""+maxt+"\",\"dr"+String(n+1)+"\":\""+String(dir)+"\",\"nl"+String(n+1)+"\":\""+nstep+"\",\"tr"+String(n+1)+"\":\""+String(t)+"\"}";
			Serial.println("Str: " + str);
			return str;
		}
		String getOnOffFeedback(uint8_t on, uint8_t off, long t, short dir, unsigned long maxt, uint8_t n){
			String str =  "{\"devid\":\""+String(mqttid)+"\",\"on"+String(n+1)+"\":\""+on+"\",\"off"+String(n+1)+"\":\""+off+"\", \"sp"+String(n+1)+"\":\""+maxt+"\",\"dr"+String(n+1)+"\":\""+String(dir)+"\",\"tr"+String(n+1)+"\":\""+String(t)+"\"}";
			Serial.println("Str: " + str);
			return str;
		}
		String getOnOffFeedback2(uint8_t on, uint8_t off, long t, short dir, uint8_t n){
			String str =  "{\"devid\":\""+String(mqttid)+"\",\"on"+String(n+1)+"\":\""+on+"\",\"off"+String(n+1)+"\":\""+off+"\",\"dr"+String(n+1)+"\":\""+String(dir)+"\",\"tr"+String(n+1)+"\":\""+String(t)+"\"}";
			Serial.println("Str: " + str);
			return str;
		}
		String getSliderFeedback2(uint8_t target, uint8_t n){
			String str = "{\"devid\":\""+String(mqttid)+"\",\"sld"+String(n+1)+"\":\""+String(target)+"\"}";
			//Serial.println("Str: " + str);
			return str;
		}
		String getDimmeredToggleInitFeedback(uint8_t on, uint8_t off, uint8_t to, short dir, unsigned nstep, unsigned long maxt, long t,uint8_t n){
			String str =  "{\"devid\":\""+String(mqttid)+"\",\"on"+String(n+1)+"\":\""+on+"\",\"off"+String(n+1)+"\":\""+off+"\",\"to"+String(n+1)+"\":\""+to+"\", \"sp"+String(n+1)+"\":\""+maxt+"\",\"dr"+String(n+1)+"\":\""+String(dir)+"\",\"nl"+String(n+1)+"\":\""+nstep+"\",\"tr"+String(n+1)+"\":\""+String(t)+"\"}";
			Serial.println("Str: " + str);
			return str;
		}
		
	public:

		DimmeredToggle(String id, uint8_t startIndex = 1, uint8_t precision = 2, unsigned nlevel = 100, unsigned long maxtime = 10000){
			remoteCntrlInit();
			k = startIndex;
			sharp[OUT1] = sharp[OUT3] = precision;
			mqttid = id;
			nstep[OUT1] = nstep[OUT3] = nlevel;
			maxt[OUT1] = maxt[OUT3] = maxtime;
		}
		void onSweep(SweepCallbackSimple cb){
			this->sweepCallback = cb;
		}
		void onFeedback(FeedbackCallbackSimple cb){
			this->feedbackCallback = cb;
		}
		void remoteToggle(uint8_t targetval){
			remoteToggle(targetval,STTGL1,SGNTGL1,OUT3);
		}
		void remoteCntrlOn(uint8_t targetval){
			remoteCntrl(targetval,STBTN1,STBTN2,SGNBTN1,SGNOP,OUT1);
		}
		void remoteCntrlOff(void){
			remoteCntrl(255,STBTN2,STBTN1,STBTN1,SGNOP,OUT1,0);
		}
		void remoteSlider(uint8_t targetval){
			remoteSlider(targetval,SGNSLD1,OUT4);
		}
		void remoteConf(void){
			signal[SGNINIT] = true;
		}		
		bool remoteCntrlEventsParser(){  // va dentro il loop()		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Più SPA posono comandare uno stesso dispositivo
		// lo stato dei pulsanti viene calcolato sul dispositivo ed inviato alle SPA 
		// lo stato di un pulsante viene inviato alle SPA come feedback 
		// immediatamente dopo la ricezione di un camando su quel pulsante
		// Ogni feedback certifica: l'avvenuta ricezione di un comando, lo stato del dispositivo dopo la ricezione del comando.
		// Una SPA non ricalcola in locale lo stato dei pulsanti ma si limita a copiarlo dal dispositivo.
		// Ogni SPA emula in locale la funzione di spostamento della barra (sweep) utilizzando lo stesso algoritmo di scivolamento 
		// temporale (sweepAction()) utilizzato dal dispositivo al fine di mantenere un perfetto sincronismo tra
		// disposititvo e SPA e tra le diverse SPA che comandano uno stesso dispositivo.
		// Sono previsti aggiornamenti periodici dello stato dei pulsanti inviati alle SPA su iniziativa del dispositivo per ovviare // alla eventuale perdita di un feedback a causa di un problema di connessione.
		// Le SPA possono richiedere lo stato completo dei pulsanti ad ogni connessione/riconnessione inviando il comando conf.
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// gestori dei segnali inviati da parte dei moduli di calcolo dello stato dei pulsanti
		// possono non essere raccolti tutti a seconda di quali pulsanti sono implemntati dalla SPA 
		// (potrebbe, ad es., mancare uno slider o un toggle)
		// generano i feedback dello stato dei pulsanti
		// possono essere parziali a seconda di quali pulsanti sono implemntati dalla SPA
		// se sono completi non succede nulla di anomalo in uanto la SPA potrebbe ignorare i feedback a pulsanti non implementati
		bool ismsg = false;
		String buf;
		
		if(signal[SGNTGL1]){	
			signal[SGNTGL1] = false;
			if(stop[OUT1]){ // se up e down della scelta livello non sono attivi (no configurazione livello in corso)
				stop[OUT3] = false;
				if(stato[STTGL1]){
					target_p[OUT3] = targetbis[OUT3]; // 3) memorizzo il valore temporale in percentuale OK
					Serial.print("targetbis: ");
					Serial.println(targetbis[OUT3]);
					Serial.print("maxt3: ");
					Serial.println(maxt[OUT3]);
					Serial.print("maxt1: ");
					Serial.println(maxt[OUT1]);
					Serial.print("target: ");
					Serial.println(target_p[OUT3]);
					startSweep(nstep[OUT3],0,maxt[OUT3],OUT3);
				}else{
					target_p[OUT3] = 0;
					/*Serial.print("target: ");
					Serial.println(target_p[OUT3]);*/
					startSweep(nstep[OUT3],0,maxt[OUT3],OUT3);
				}
				//digitalWrite(LEDSOGGIORNO1, stato[BTN1]);
				Serial.print("TOGL1: ");
				Serial.println(stato[STTGL1]);	
				ismsg = true;
				buf = getDimmeredToggleFeedback(stato[STTGL1]*targetbis[OUT3],stato[STTGL1]*targetbis[OUT3]*0,stato[STTGL1],t[OUT3],webdir(OUT3),nstep[OUT3],maxt[OUT3],OUT1+k);
				(*feedbackCallback)(buf);
			}
		}
		if(signal[SGNBTN1] || signal[SGNBTN2]){
			signal[SGNBTN1] = false;
			signal[SGNBTN2] = false;
			startSweep(nstep[OUT1],0,maxt[OUT1],OUT1);
			Serial.print("BTN1: ");
			Serial.println(stato[STBTN1]);
			Serial.println(stato[STBTN2]);
			ismsg = true;
			buf =  getOnOffFeedback(stato[STBTN1],stato[STBTN2],t[OUT1],webdir(OUT1),maxt[OUT1], OUT1+k);
			(*feedbackCallback)(buf);
		}
		if(signal[SGNSLD1]){ 
			signal[SGNSLD1] = false;
			Serial.print("stop[OUT3]: ");
			Serial.println(stop[OUT3]);
			if(stop[OUT3]){ // se il toggle non è in scivolamento
				maxt[OUT3] = (float) target_p[OUT4]/100*maxt[OUT1]; // 1) target slider ---> max sweep toggle OK
				Serial.print("maxt[OUT3]: ");
				Serial.println(maxt[OUT3]);
				ismsg = true;
				buf = getSliderFeedback2(target_p[OUT4], OUT1);
				(*feedbackCallback)(buf);
			}
		}
		if(signal[SGNBTNRST1]){
			signal[SGNBTNRST1] = false;
			stato[STBTN1] = LOW;
			stato[STBTN2] = LOW;
			targetbis[OUT3] = (float) t[OUT1]/maxt[OUT1]*100; // 2) memorizzo il valore temporale in percentuale OK
			Serial.print("targetbis[OUT3]: ");
			Serial.println(targetbis[OUT3]);
			ismsg = true;
			buf = getOnOffFeedback2(stato[STBTN1],stato[STBTN2],t[OUT1],webdir(OUT1), OUT1+k);
			(*feedbackCallback)(buf);
		}
		if(signal[SGNBTNRST3]){
			signal[SGNBTNRST3] = false;
			stato[STBTN3] = LOW;
			//stato[STTGL2] = 1; //setta il toggle
			ismsg = true;
			buf = getOnOffFeedback2(stato[STBTN1],stato[STBTN2],t[OUT3],webdir(OUT3), OUT1+k);
			(*feedbackCallback)(buf);
		}
		if(signal[SGNINIT]){
			signal[SGNINIT] = false;
			stato[STBTN1] = LOW;
			stato[STBTN2] = LOW;
			ismsg = true;
			buf = getDimmeredToggleFeedback(stato[STTGL1]*targetbis[OUT3],stato[STTGL1]*targetbis[OUT3]*0,stato[STTGL1],t[OUT3],webdir(OUT3),nstep[OUT3],maxt[OUT3],OUT1+k);
			(*feedbackCallback)(buf);
		}
		return ismsg;
	}
};
