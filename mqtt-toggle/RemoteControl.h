#ifndef __REMOTECNTRL_H__
#define __REMOTECNTRL_H__
#include <Arduino.h>
#include <Ticker.h>

typedef void (*SweepCallbackSimple)(int outr, int cr, uint8_t k);
typedef void (*FeedbackCallbackSimple)(String buf);

class RemoteBase
{
    protected:
		String mqttid;
		long *target_p;
		long *target_tt;
		long *targetbis;
		long *midtarget;
		bool *stop;
		uint8_t *stato;
		bool *signal;
		uint8_t k;
		unsigned* nstep;
		FeedbackCallbackSimple feedbackCallback = nullptr;
		
		// json command parser: ricerca un certo commando all’interno di una stringa e ne restituisce il valore
		void remoteCntrlInit(uint8_t nsgn, uint8_t nstates, uint8_t nout);
		// gestore comandi singolo slider (logica senza stato)
		void remoteSlider(uint8_t targetval, uint8_t sgnsld, uint8_t n);
		// gestore comandi singolo pulsante toggle, calcola lo stato del pulsante toggle
		void remoteToggle(uint8_t targetval, uint8_t sttgl, uint8_t sgntgl, uint8_t n);
		// gestore comandi coppia di pulsanti on/off o up/down (gestisce anche un'eventuale slider associato alla coppia)
		// calcola lo stato dei due pulsanti 
		void remoteCntrl(uint8_t targetval, uint8_t stbtna, uint8_t stbtnb, uint8_t sgnbtna, uint8_t sgnsld, uint8_t n, uint8_t mult=1);
		/////    GESTORE EVENTI (callback)    /////////////////////////////////////////////////////////////////////////////////
		String getToggleFeedback(uint8_t toggleState, uint8_t n);
		String getSliderFeedback(uint8_t target, uint8_t n);
		String getSliderFeedback2(uint8_t target, uint8_t n);
	public:
		RemoteBase(String id, uint8_t startIndex, uint8_t nsgn, uint8_t nstates, uint8_t nout);
		void onFeedback(FeedbackCallbackSimple cb);
		bool cmdParser(String &outstr, String instr, String attributo, unsigned maxlen);
};

class DimmeredToggle : public RemoteBase
{
    private:	
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
		Ticker sweepTimer[NOUT];
		unsigned long maxt[NOUT];
		int outval[NOUT];
		int countr[NOUT];
		long t[NOUT];
		unsigned long tstep[NOUT];
		unsigned long prec_t[NOUT];
		int direct[NOUT];
		uint8_t sharp[NOUT];
		SweepCallbackSimple sweepCallback = nullptr;
		uint8_t sweeepParam;
		
		void remoteCntrlSweepInit();
		// il target_tt si fornisce in percentuale intera di 100 (ad es. 80)
		void startSweep(unsigned nsteps,unsigned delay,unsigned long tmax,unsigned short n);
		// codifica stop/start e direzione della barra per la SPA
		int webdir(uint8_t n);
		/////    GESTORE EVENTI (callback)    /////////////////////////////////////////////////////////////////////////////////
		String getDimmeredToggleFeedback(uint8_t on, uint8_t off, uint8_t to, long t, short dir, unsigned nstep, unsigned long maxt, uint8_t n);
		String getOnOffFeedback(uint8_t on, uint8_t off, long t, short dir, unsigned long maxt, uint8_t n);
		String getOnOffFeedback2(uint8_t on, uint8_t off, long t, short dir, uint8_t n);
		String getDimmeredToggleInitFeedback(uint8_t on, uint8_t off, uint8_t to, short dir, unsigned nstep, unsigned long maxt, long t,uint8_t n);
		
	public:

		DimmeredToggle(String id, uint8_t startIndex, uint8_t precision = 2, unsigned nlevel = 100, unsigned long maxtime = 10000);//:RemoteBase(id,startIndex,NSGN,NSTATES,NOUT){};
		void sweep(uint8_t n);
		void onSweep(SweepCallbackSimple cb);
		void remoteToggle(uint8_t targetval);
		void remoteCntrlOn(uint8_t targetval);
		void remoteCntrlOff(void);
		void remoteSlider(uint8_t targetval);
		void remoteConf(void);
		bool remoteCntrlEventsParser();
		uint8_t getSweepParam(){
			return sweeepParam;
		}
		static void staticSweep(DimmeredToggle *pThis) {
			pThis->sweep(pThis->getSweepParam());
		}
};

class FadedSlider : public RemoteBase
{
    private:	
		//************** DEFINIZIONE USCITE **************************/
		enum outs // indice uscite
		{
			OUT1			=0, // slider1
			NOUT			=1
		};
		/*******
		/************** DEFINIZIONE STATI **************************/
		enum btnstates // stato pulsanti 
		{
			STTNOP			=0,
			NSTATES         =1
		};
		/************** DEFINIZIONE SEGNALI **************************/
		enum signals // segnali tra timer callbacks e loop() (flags)
		{
			SGNSLD1 		=0,
			SGNBTNRST1		=1,
			SGNINIT			=2,
			SGNOP			=3,
			NSGN        	=4
		};
		/********* FINE DEFINIZIONE SEGNALI **************************/

		Ticker sweepTimer[NOUT];
		unsigned long maxt[NOUT];
		int outval[NOUT];
		int countr[NOUT];
		long t[NOUT];
		unsigned long tstep[NOUT];
		unsigned long prec_t[NOUT];
		int direct[NOUT];
		uint8_t sharp[NOUT];
		SweepCallbackSimple sweepCallback = nullptr;
		uint8_t sweeepParam;
		
		void remoteCntrlSweepInit();
		// il target_tt si fornisce in percentuale intera di 100 (ad es. 80)
		void startSweep(unsigned nsteps,unsigned delay,unsigned long tmax,unsigned short n);
		// codifica stop/start e direzione della barra per la SPA
		int webdir(uint8_t n);
		/////    GESTORE EVENTI (callback)    /////////////////////////////////////////////////////////////////////////////////
		String getFadedSliderActionFeedback(long targetp, short dir, long t, uint8_t n);
		String getFadedSliderSweepStopFeedback(short dir, long t, uint8_t n);
		String getFadedSliderSweepInitFeedback(long t, short dir, unsigned long maxt, unsigned nstep, uint8_t n);
		
	public:
		FadedSlider(String id, uint8_t startIndex, uint8_t precision = 2, unsigned nlevel = 100, unsigned long maxtime = 10000);
		void sweep(uint8_t n);
		void onSweep(SweepCallbackSimple cb);
		void remoteSlider(uint8_t targetval);
		void remoteConf(void);
		bool remoteCntrlEventsParser();
		uint8_t getSweepParam(){
			return sweeepParam;
		}
		static void staticSweep(FadedSlider *pThis) {
			pThis->sweep(pThis->getSweepParam());
		}
};

class Slider : public RemoteBase
{
    private:	
		//************** DEFINIZIONE USCITE **************************/
		enum outs // indice uscite
		{
			OUT1			=0, // slider1
			NOUT			=1
		};
		/*******
		/************** DEFINIZIONE STATI **************************/
		enum btnstates // stato pulsanti 
		{
			STTNOP			=0,
			NSTATES         =1
		};
		/************** DEFINIZIONE SEGNALI **************************/
		enum signals // segnali tra timer callbacks e loop() (flags)
		{
			SGNSLD1 		=0,
			SGNBTNRST1		=1,
			SGNINIT			=2,
			SGNOP			=3,
			NSGN        	=4
		};
		/********* FINE DEFINIZIONE SEGNALI **************************/
		/////    GESTORE EVENTI (callback)    /////////////////////////////////////////////////////////////////////////////////
		SweepCallbackSimple actionkCallback = nullptr;
		String getSliderFeedback(uint8_t target, uint8_t n);
		
	public:
		Slider(String id, uint8_t startIndex, unsigned nlevels);
		void remoteSlider(uint8_t targetval);
		void remoteConf(void);
		bool remoteCntrlEventsParser();
		void onAction(SweepCallbackSimple cb);
};

class Toggle : public RemoteBase
{
    private:	
		//************** DEFINIZIONE USCITE **************************/
		enum outs // indice uscite
		{
			OUT1			=0, // slider1
			NOUT			=1
		};
		/*******
		/************** DEFINIZIONE STATI **************************/
		enum btnstates // stato pulsanti 
		{
			STTGL1			=0,
			NSTATES         =1
		};
		/************** DEFINIZIONE SEGNALI **************************/
		enum signals // segnali tra timer callbacks e loop() (flags)
		{
			SGNTGL1			=0,
			SGNBTNRST1		=1,
			SGNINIT			=2,
			SGNOP			=3,
			NSGN        	=4
		};
		/********* FINE DEFINIZIONE SEGNALI **************************/
		/////    GESTORE EVENTI (callback)    /////////////////////////////////////////////////////////////////////////////////
		SweepCallbackSimple actionkCallback = nullptr;
		String getToggleFeedback(uint8_t toggleState, uint8_t n);
		
	public:
		Toggle(String id,uint8_t startIndex);
		void remoteConf(void);
		bool remoteCntrlEventsParser();
		void remoteToggle();
		void onAction(SweepCallbackSimple cb);
};
#endif
