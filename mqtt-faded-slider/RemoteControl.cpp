#include "RemoteControl.h"

//// REMOTE BASE ///////////////////////////////////////////////////////////////
RemoteBase::RemoteBase(String id, uint8_t startIndex, uint8_t nsgn, uint8_t nstates, uint8_t nout){
	mqttid = id;
	target_p = new long[nout];
	target_tt = new long[nout];
	targetbis = new long[nout];
	midtarget = new long[nout];
	stop = new bool[nout];
	stato = new uint8_t[nstates];
	signal = new bool[nsgn];
	nstep = new unsigned[nout];
	remoteCntrlInit(nsgn, nstates, nout);
	k = startIndex;
}
// json command parser: ricerca un certo commando all’interno di una stringa e ne restituisce il valore
		// json command parser: ricerca un certo commando all?interno di una stringa e ne restituisce il valore
bool RemoteBase::cmdParser(String &outstr, String instr, String attributo, unsigned maxlen){
	int start, ends=0;
		
	start = instr.indexOf("\""+attributo+"\":\"");

	if(start >= 0){
		start += (attributo).length() + 4;
		//scorri l'input finch? non trovi il tag di fine oppure il fine stringa
		for(ends=start+1; (ends < start + maxlen) && instr.charAt(ends)!='"' && ends < instr.length(); ends++);
		outstr = instr.substring(start, ends);
	}
	return (start >= 0);
}	
void RemoteBase::remoteCntrlInit(uint8_t nsgn, uint8_t nstates, uint8_t nout){
	for(int i=0; i<nsgn; i++){
	  signal[i] = false;
	}
	for(int i=0; i<nstates; i++){
	  stato[i] = 0;
	}
	for(int i=0; i<nout; i++){
		target_p[i] = 0;
		target_tt[i] = 0;
		targetbis[i] = 0;
		stop[i] = true; // lo scivolamento deve essere sbloccato da un tasto o da un cursore
	}
}
void RemoteBase::onFeedback(FeedbackCallbackSimple cb){
	this->feedbackCallback = cb;
}	
// gestore comandi singolo slider (logica senza stato)
void RemoteBase::remoteSlider(uint8_t targetval, uint8_t sgnsld, uint8_t n){
	stop[n] = false;
	target_p[n] = targetval;
	targetbis[n] = targetval;
	signal[sgnsld] = true;
}
// gestore comandi singolo pulsante toggle, calcola lo stato del pulsante toggle
void RemoteBase::remoteToggle(uint8_t targetval, uint8_t sttgl, uint8_t sgntgl, uint8_t n){
	if(targetval == 255){
		signal[sgntgl] = true;
		stop[n] = true;   // normalmente ? alternativo allo scivolamento per cui lo blocca
		stato[sttgl] = !stato[sttgl];		
	}
}
// gestore comandi coppia di pulsanti on/off o up/down (gestisce anche un'eventuale slider associato alla coppia)
// calcola lo stato dei due pulsanti 
void RemoteBase::remoteCntrl(uint8_t targetval, uint8_t stbtna, uint8_t stbtnb, uint8_t sgnbtna, uint8_t sgnsld, uint8_t n, uint8_t mult){
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
//// DIMMERED TOGGLE ///////////////////////////////////////////////////////////////
void DimmeredToggle::remoteCntrlSweepInit(){
	for(int i=0; i<NOUT; i++){
		outval[i] = 0;
		countr[i] = 0;
		t[i] = 0;
		tstep[i] = 0;
		sharp[i] = 2;
	}
}
void DimmeredToggle::sweep(uint8_t n) {
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
		int cr,out;
		if(stop[OUT1]){
			cr = countr[OUT3];	//condizionamento digitale input 1
			out = outval[OUT3];
		}else{
			cr = stato[STTGL1]*countr[OUT1];
			out = stato[STTGL1]*outval[OUT1];
		}
		(*sweepCallback)(out,cr,k);
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
// il target_tt si fornisce in percentuale intera di 100 (ad es. 80)
void DimmeredToggle::startSweep(unsigned nsteps,unsigned delay,unsigned long tmax,unsigned short n) {
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
		int cr, out;
		if(stop[OUT1]){
			cr = countr[OUT3];	//condizionamento digitale input 1
			out = outval[OUT3];
		}else{
			cr = stato[STTGL1]*countr[OUT1];
			out = stato[STTGL1]*outval[OUT1];
		}
		(*sweepCallback)(out,cr,k);
		stop[n]=true;
	}
	Serial.println("NEXT TO START");
}
// codifica stop/start e direzione della barra per la SPA
int DimmeredToggle::webdir(uint8_t n) {
	int val=0;
	if(!stop[n]){
		val = direct[n];
	}
	return val;
}
String DimmeredToggle::getDimmeredToggleFeedback(uint8_t on, uint8_t off, uint8_t to, long t, short dir, unsigned nstep, unsigned long maxt, uint8_t n){
	String str =  "{\"devid\":\""+String(mqttid)+"\",\"on"+String(n+1)+"\":\""+on+"\",\"off"+String(n+1)+"\":\""+off+"\",\"to"+String(n+1)+"\":\""+to+"\", \"sp"+String(n+1)+"\":\""+maxt+"\",\"dr"+String(n+1)+"\":\""+String(dir)+"\",\"nl"+String(n+1)+"\":\""+nstep+"\",\"tr"+String(n+1)+"\":\""+String(t)+"\"}";
	Serial.println("Str: " + str);
	return str;
}
String DimmeredToggle::getOnOffFeedback(uint8_t on, uint8_t off, long t, short dir, unsigned long maxt, uint8_t n){
	String str =  "{\"devid\":\""+String(mqttid)+"\",\"on"+String(n+1)+"\":\""+on+"\",\"off"+String(n+1)+"\":\""+off+"\", \"sp"+String(n+1)+"\":\""+maxt+"\",\"dr"+String(n+1)+"\":\""+String(dir)+"\",\"tr"+String(n+1)+"\":\""+String(t)+"\"}";
	Serial.println("Str: " + str);
	return str;
}
String DimmeredToggle::getOnOffFeedback2(uint8_t on, uint8_t off, long t, short dir, uint8_t n){
	String str =  "{\"devid\":\""+String(mqttid)+"\",\"on"+String(n+1)+"\":\""+on+"\",\"off"+String(n+1)+"\":\""+off+"\",\"dr"+String(n+1)+"\":\""+String(dir)+"\",\"tr"+String(n+1)+"\":\""+String(t)+"\"}";
	Serial.println("Str: " + str);
	return str;
}
String DimmeredToggle::getDimmeredToggleInitFeedback(uint8_t on, uint8_t off, uint8_t to, short dir, unsigned nstep, unsigned long maxt, long t,uint8_t n){
	String str =  "{\"devid\":\""+String(mqttid)+"\",\"on"+String(n+1)+"\":\""+on+"\",\"off"+String(n+1)+"\":\""+off+"\",\"to"+String(n+1)+"\":\""+to+"\", \"sp"+String(n+1)+"\":\""+maxt+"\",\"dr"+String(n+1)+"\":\""+String(dir)+"\",\"nl"+String(n+1)+"\":\""+nstep+"\",\"tr"+String(n+1)+"\":\""+String(t)+"\"}";
	Serial.println("Str: " + str);
	return str;
}
String DimmeredToggle::getSliderFeedback2(uint8_t target, uint8_t n){
	String str = "{\"devid\":\""+String(mqttid)+"\",\"sld"+String(n+1)+"\":\""+String(target)+"\"}";
	//Serial.println("Str: " + str);
	return str;
}
DimmeredToggle::DimmeredToggle(String id, uint8_t startIndex, uint8_t precision, unsigned nlevel, unsigned long maxtime):RemoteBase(id,startIndex,NSGN,NSTATES,NOUT){
	remoteCntrlSweepInit();
	sharp[OUT1] = sharp[OUT3] = precision;
	mqttid = id;
	nstep[OUT1] = nstep[OUT3] = nlevel;
	maxt[OUT1] = maxt[OUT3] = maxtime;
}
void DimmeredToggle::onSweep(SweepCallbackSimple cb){
	this->sweepCallback = cb;
}
void DimmeredToggle::remoteToggle(uint8_t targetval){
	RemoteBase::remoteToggle(targetval,STTGL1,SGNTGL1,OUT3);
}
void DimmeredToggle::remoteCntrlOn(uint8_t targetval){
	RemoteBase::remoteCntrl(targetval,STBTN1,STBTN2,SGNBTN1,SGNOP,OUT1);
}
void DimmeredToggle::remoteCntrlOff(void){
	RemoteBase::remoteCntrl(255,STBTN2,STBTN1,SGNBTN2,SGNOP,OUT1,0);
}
void DimmeredToggle::remoteSlider(uint8_t targetval){
	RemoteBase::remoteSlider(targetval,SGNSLD1,OUT4);
}
void DimmeredToggle::remoteConf(void){
	signal[SGNINIT] = true;
}
void DimmeredToggle::processCmd(String id, String payload, uint16_t maxlen){
	String str;	
	cmdParser(str,payload,"devid",maxlen);
	if(str == id){		
		if(cmdParser(str,payload,"to"+String(k+1),maxlen)){
			remoteToggle(255);
		}
		if(cmdParser(str,payload,"on"+String(k+1),maxlen)){
			remoteCntrlOn(atoi(str.c_str()));
		}
		if(payload.indexOf("\"off"+String(k+1)+"\":\"255\"") >= 0){
			remoteCntrlOff();
		}
		if(cmdParser(str,payload,"sld"+String(k+1),maxlen)){
			remoteSlider(atoi(str.c_str()));
		}
		if(payload.indexOf("\"conf\":\"255\"") >= 0){
			remoteConf();
		}
	}
}
bool DimmeredToggle::remoteCntrlEventsParser(){
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
	delay(0);
	return ismsg;
}
//// FADED SLIDER ///////////////////////////////////////////////////////////////
void FadedSlider::remoteCntrlSweepInit(){
	for(int i=0; i<NOUT; i++){
		outval[i] = 0;
		countr[i] = 0;
		t[i] = 0;
		tstep[i] = 0;
		sharp[i] = 2;
	}
}
void FadedSlider::sweep(uint8_t n) {
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
		int cr,out;
		cr = countr[OUT1];
		out = outval[OUT1];
		(*sweepCallback)(out,cr,k);
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
// il target_tt si fornisce in percentuale intera di 100 (ad es. 80)
void FadedSlider::startSweep(unsigned nsteps,unsigned delay,unsigned long tmax,unsigned short n) {
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
				sweeepParam = n;
				sweepTimer[n].attach_ms(tstep[n], &FadedSlider::staticSweep, this);
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
		int cr, out;
		cr = countr[OUT1];
		out = outval[OUT1];
		(*sweepCallback)(out,cr,k);
		stop[n]=true;
	}
	Serial.println("NEXT TO START");
}
// codifica stop/start e direzione della barra per la SPA
int FadedSlider::webdir(uint8_t n) {
	int val=0;
	if(!stop[n]){
		val = direct[n];
	}
	return val;
}
/////    GESTORE EVENTI (callback)    /////////////////////////////////////////////////////////////////////////////////
String FadedSlider::getFadedSliderActionFeedback(long targetp, short dir, long t, uint8_t n){
	String str = "{\"devid\":\""+String(mqttid)+"\",\"pr"+String(n+1)+"\":\""+String(targetp)+"\",\"dr"+String(n+1)+"\":\""+String(dir)+"\",\"tr"+String(n+1)+"\":\""+String(t)+"\"}";
	Serial.println("Str: " + str);
	return str;
}
String FadedSlider::getFadedSliderSweepStopFeedback(short dir, long t, uint8_t n){
	String str = "{\"devid\":\""+String(mqttid)+"\",\"dr"+String(n+1)+"\":\""+String(dir)+"\",\"tr"+String(n+1)+"\":\""+String(t)+"\"}";
	Serial.println("Str: " + str);
	return str;
}
String FadedSlider::getFadedSliderSweepInitFeedback(long t, short dir, unsigned long maxt, unsigned nstep, uint8_t n){
	String str =  "{\"devid\":\""+String(mqttid)+"\", \"sp"+String(n+1)+"\":\""+maxt+"\",\"dr"+String(n+1)+"\":\""+String(dir)+"\",\"nl"+String(n+1)+"\":\""+nstep+"\",\"tr"+String(n+1)+"\":\""+String(t)+"\"}";
	Serial.println("Str: " + str);
	return str;
}
FadedSlider::FadedSlider(String id, uint8_t startIndex, uint8_t precision, unsigned nlevel, unsigned long maxtime):RemoteBase(id,startIndex,NSGN,NSTATES,NOUT){
	remoteCntrlSweepInit();
	sharp[OUT1] = precision;
	mqttid = id;
	nstep[OUT1] = nlevel;
	maxt[OUT1] = maxtime;
};
void FadedSlider::onSweep(SweepCallbackSimple cb){
	this->sweepCallback = cb;
}
void FadedSlider::remoteSlider(uint8_t targetval){
	RemoteBase::remoteSlider(targetval,SGNSLD1,OUT1);
}
void FadedSlider::remoteConf(void){
	signal[SGNINIT] = true;
}
void FadedSlider::processCmd(String id, String payload, uint16_t maxlen){
	String str;	
	cmdParser(str,payload,"devid",maxlen);
	if(str == id){		
		if(cmdParser(str,payload,"sld"+String(k+1),maxlen)){
			remoteSlider(atoi(str.c_str()));
		}
		if(payload.indexOf("\"conf\":\"255\"") >= 0){
			remoteConf();
		}
	}
}
bool FadedSlider::remoteCntrlEventsParser(){
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Pi? SPA posono comandare uno stesso dispositivo
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
	
	if(signal[SGNSLD1]){	
		signal[SGNSLD1] = false;
		Serial.print("SLD1: ");
		Serial.println(countr[OUT1]);
		startSweep(nstep[OUT1],0,maxt[OUT1],OUT1);
		buf = getFadedSliderActionFeedback(target_p[OUT1], webdir(OUT1), t[OUT1], OUT1+k);
		(*feedbackCallback)(buf);
	}
	if(signal[SGNBTNRST1]){
		signal[SGNBTNRST1] = false;
		Serial.print("SGNBTNRST1: ");
		buf = getFadedSliderSweepStopFeedback(webdir(OUT1), t[OUT1], OUT1+k);
		(*feedbackCallback)(buf);
	}
	if(signal[SGNINIT]){
		signal[SGNINIT] = false;
		buf = getFadedSliderSweepInitFeedback(t[OUT1], webdir(OUT1), maxt[OUT1], nstep[OUT1], OUT1+k);
		(*feedbackCallback)(buf);
	}
	delay(0);
	return ismsg;
}
//// SLIDER ///////////////////////////////////////////////////////////////
String Slider::getSliderFeedback(uint8_t target, uint8_t n){
	String str = "{\"devid\":\""+String(mqttid)+"\",\"pr"+String(n+1)+"\":\""+String(target)+"\"}";
	//Serial.println("Str: " + str);
	return str;
}
void Slider::processCmd(String id, String payload, uint16_t maxlen){
	String str;	
	cmdParser(str,payload,"devid",maxlen);
	if(str == id){		
		if(cmdParser(str,payload,"sld"+String(k+1),maxlen)){
			remoteSlider(atoi(str.c_str()));
		}
		if(payload.indexOf("\"conf\":\"255\"") >= 0){
			remoteConf();
		}
	}
}
bool Slider::remoteCntrlEventsParser(){
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Pi? SPA posono comandare uno stesso dispositivo
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
	
	if(signal[SGNSLD1]){	
		signal[SGNSLD1] = false;
		Serial.print("SLD1: ");
		Serial.println(target_p[OUT1]);
		int cr, out;
		cr = (float) (target_p[OUT1]) / 100 * nstep[OUT1];
		out = target_p[OUT1];
		buf = getSliderFeedback(target_p[OUT1], OUT1+k);
		(*actionkCallback)(out,cr,k);
		(*feedbackCallback)(buf);
	}
	if(signal[SGNINIT]){
		signal[SGNINIT] = false;
		buf = getSliderFeedback(target_p[OUT1], OUT1+k);
		int cr, out;
		(*feedbackCallback)(buf);
	}
	delay(0);
	return ismsg;
}
Slider::Slider(String id, uint8_t startIndex, unsigned nlevels):RemoteBase(id,startIndex,NSGN,NSTATES,NOUT){
	nstep[OUT1] = nlevels;
}
void Slider::remoteSlider(uint8_t targetval){
	RemoteBase::remoteSlider(targetval,SGNSLD1,OUT1);
}
void Slider::remoteConf(void){
	signal[SGNINIT] = true;
}
void Slider::onAction(SweepCallbackSimple cb){
	this->actionkCallback = cb;
}
//// TOGGLE ///////////////////////////////////////////////////////////////
Toggle::Toggle(String id,uint8_t startIndex):RemoteBase(id,startIndex,NSGN,NSTATES,NOUT){
}
String Toggle::getToggleFeedback(uint8_t toggleState, uint8_t n){
	String str = "{\"devid\":\""+String(mqttid)+"\",\"to"+String(n+1)+"\":\""+String(toggleState)+"\"}";
	//Serial.println("Str: " + str);
	return str;
}	
void Toggle::processCmd(String id, String payload, uint16_t maxlen){
	String str;	
	cmdParser(str,payload,"devid",maxlen);
	if(str == id){		
		if(cmdParser(str,payload,"to"+String(k+1),maxlen)){
			remoteToggle();
		}
		if(payload.indexOf("\"conf\":\"255\"") >= 0){
			remoteConf();
		}
	}
}
bool Toggle::remoteCntrlEventsParser(){
	bool ismsg = false;
	String buf;
	
	if(signal[SGNTGL1]){	
		signal[SGNTGL1] = false;
		Serial.print("TOGL1: ");
		Serial.println(stato[STTGL1]);
		buf = getToggleFeedback(stato[STTGL1], OUT1+k);
		int cr, out;
		cr = stato[STTGL1];
		out = stato[STTGL1]*100;
		(*actionkCallback)(out,cr,k);
		(*feedbackCallback)(buf);
	}
	if(signal[SGNINIT]){
		signal[SGNINIT] = false;
		buf = getToggleFeedback(stato[STTGL1], OUT1+k);
		(*feedbackCallback)(buf);
	}
	delay(0);
	return ismsg;
}
void Toggle::remoteToggle(){
	RemoteBase::remoteToggle(255,STTGL1,SGNTGL1,OUT1);
}
void Toggle::remoteConf(void){
	signal[SGNINIT] = true;
}
void Toggle::onAction(SweepCallbackSimple cb){
	this->actionkCallback = cb;
}
//// DIMMERED TOGGLE ///////////////////////////////////////////////////////////////
String Motor::getUpDownFeedback(uint8_t up, uint8_t down, short dir, uint8_t n){
	String str =  "{\"devid\":\""+String(mqttid)+"\",\"up"+String(n+1)+"\":\""+up+"\",\"down"+String(n+1)+"\":\""+down+"\",\"dr"+String(n+1)+"\":\""+String(dir)+"\"}";
	Serial.println("Str: " + str);
	return str;
}
Motor::Motor(String id, uint8_t startIndex):RemoteBase(id,startIndex,NSGN,NSTATES,NOUT){
	mqttid = id;
}
void Motor::remoteCntrlUp(void){
	RemoteBase::remoteCntrl(255,STBTN1,STBTN2,SGNBTN1,SGNOP,OUT1);
}
void Motor::remoteCntrlDown(void){
	RemoteBase::remoteCntrl(255,STBTN2,STBTN1,SGNBTN2,SGNOP,OUT1);
}
void Motor::remoteConf(void){
	signal[SGNINIT] = true;
}
void Motor::onAction(SweepCallbackSimple cb){
	this->actionkCallback = cb;
}
void Motor::processCmd(String id, String payload, uint16_t maxlen){
	String str;	
	cmdParser(str,payload,"devid",maxlen);
	if(str == id){		
		if(cmdParser(str,payload,"up"+String(k+1),maxlen)){
			remoteCntrlUp();
		}
		if(cmdParser(str,payload,"down"+String(k+1),maxlen)){
			remoteCntrlDown();
		}
		if(payload.indexOf("\"conf\":\"255\"") >= 0){
			remoteConf();
		}
	}
}
bool Motor::remoteCntrlEventsParser(){
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
	
	if(signal[SGNBTN1]){
		signal[SGNBTN1] = false;
		Serial.print("BTN1: ");
		Serial.println(stato[STBTN1]);
		ismsg = true;
		if(stato[STBTN1]){
			direct[OUT1] = 1;
		}else if(stop[OUT1]){
			direct[OUT1] = 0;
		}
		buf =  getUpDownFeedback(stato[STBTN1],stato[STBTN2],direct[OUT1], OUT1+k);
		(*actionkCallback)(!stop[OUT1],direct[OUT1],k);
		(*feedbackCallback)(buf);
	}
	if(signal[SGNBTN2]){
		signal[SGNBTN2] = false;
		Serial.print("BTN2: ");
		Serial.println(stato[STBTN2]);
		ismsg = true;
		if(stato[STBTN2]){
			direct[OUT1] = -1;
		}else if(stop[OUT1]){
			direct[OUT1] = 0;
		}
		buf =  getUpDownFeedback(stato[STBTN1],stato[STBTN2],direct[OUT1], OUT1+k);
		(*actionkCallback)(!stop[OUT1],direct[OUT1],k);
		(*feedbackCallback)(buf);
	}
	if(signal[SGNINIT]){
		signal[SGNINIT] = false;
		//stato[STBTN1] = LOW;
		//stato[STBTN2] = LOW;
		ismsg = true;
		buf = getUpDownFeedback(stato[STBTN1],stato[STBTN2],direct[OUT1],OUT1+k);
		(*feedbackCallback)(buf);
	}
	delay(0);
	return ismsg;
}

