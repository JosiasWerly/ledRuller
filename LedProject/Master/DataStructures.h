#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define maxExercicies 2
#define boards 5
#define Leds 40
#define dLeds ((float)180/(float)Leds)
#define maxTime 60
#define dTime 0.05f

template<typename TKey, typename TValue>
class pair
{
public:
	TKey key;
	TValue value;
	pair()
	{}	
	pair(TKey key, TValue value)
		: key(key), value(value)
	{}
	~pair()
	{}
};

typedef pair<int, void*> procedureEvent;

class Display
{
	Adafruit_SSD1306 *display;
public:
	Display()
	{
		display = nullptr;
	}
	~Display()
	{
		if (display)
		{
			delete display;
			display = nullptr;
		}
	}
	void begin(int T, int address, int oledReset = 4)
	{
		if (display)
		{
			delete display;
			display = nullptr;
		}
		display = new Adafruit_SSD1306(oledReset);
		display->begin(T, address);
	}
	void print(String data, int posX = 0, int posY = 0, char fontSize = 1, char color = WHITE)
	{
		display->setTextSize(fontSize);
		display->setTextColor(color);
		display->setCursor(posX, posY);
		display->println(data);
	}
	void xPrint(String data, int posX = 0, int posY = 0, char fontSize = 1, char color = WHITE)
	{
		display->clearDisplay();
		display->setTextSize(fontSize);
		display->setTextColor(color);
		display->setCursor(posX, posY);
		display->println(data);
		display->display();
	}
	void pagePrint(String Header, String l1 = "", String l2 = "")
	{
		clearBuffer();
		print(Header, 0, 11, 2);
		print(l1, 0, 29, 2);
		flushBuffer();
	}
	
	void clearBuffer()
	{
		display->clearDisplay();
	}
	void flushBuffer()
	{
		display->display();
	}
};
class Decoder
{
	bool pState;
	bool mPressed, mState;
	int pA, pB, pC;
	int pos;
public:
	Decoder()
	{
		mState = mPressed = true;
	}
	void begin(int pA, int pB, int pC)
	{
		this->pA = pA;
		this->pB = pB;
		this->pC = pC;
		pinMode(INPUT, pA);
		pinMode(INPUT, pB);
		pinMode(INPUT, pC);
		digitalWrite(pC, HIGH);
		pState = digitalRead(pA);
	}
	bool isPressed()
	{
		if (!digitalRead(pC) && !mState)
			return mState = true;
		else if (digitalRead(pC))
			mState = false;
		return false;
	}
	int getPosition()
	{
		return pos;
	}
	int getDirection()
	{
		bool nState = digitalRead(pA);
		char dir = 0;
		if (pState != nState)
		{
			if (digitalRead(pB) != nState)
			{
				pos++;
				dir = 1;
			}
			else
			{
				pos--;
				dir = -1;
			}
		}
		pState = nState;
		return dir;
	}
};


class BitShifter
{
	int pLch, pDt, pClk;
public:
	char data[boards];
	BitShifter()
	{
	}
	void begin(int pLch, int pDt, int pClk)
	{
		for(int x = 0; x < boards; x++)
			this->data[x] = 0;
		this->pLch = pLch;
		this->pDt = pDt;
		this->pClk = pClk;
		pinMode(pLch, OUTPUT);
		pinMode(pDt, OUTPUT);
		pinMode(pClk, OUTPUT);
	}
	void set(int pos, int signal)
	{
		byte it = (((float)pos/Leds)*boards);
		pos -= it*8;
		unsigned int tPos = 1 << pos;
		if(signal != (tPos & (data[it])))
			data[it] = (signal ? tPos | (data[it]) : tPos ^ (data[it]));
	}
	int get(int pos)
	{
		byte it = pos / Leds;
		return (data[it])&(1 << pos);
	}
	void clear()
	{
		for(int x = 0; x < boards; x++)
			data[x] = 0;
	}
	void updateShifterRegister()
	{
		digitalWrite(pLch, LOW);
		for(int x = (boards-1); x >= 0; x--)
		{shiftOut(pDt, pClk, LSBFIRST, data[x]);}
		digitalWrite(pLch, HIGH);
	}
};

class iProcedure
{
public:	
	iProcedure()
	{}
	virtual ~iProcedure()
	{}
	virtual void setup() = 0;
	virtual procedureEvent loop() = 0;
};
class Director
{
public:
	iProcedure *pc;
	Director()
	{
		pc = nullptr;
	}

	template<typename T>
	void changeProcedure()
	{
		if (pc != nullptr)
		{
			delete pc;
			pc = nullptr;
		}
		pc = new T();
		pc->setup();
	}
	void changeProcedure(void *p)
	{
		if (pc != nullptr)
		{
			delete pc;
			pc = nullptr;
		}
		pc = reinterpret_cast<iProcedure*>(p);
		if (!pc)
			Serial.println("invalid Procedure");
		pc->setup();
	}
	virtual void loop()
	{
		procedureEvent pe = pc->loop();
		if (pe.key != 0)
			changeProcedure(pe.value);
	}
};
class iScreen
	: public iProcedure
{
protected:
	Display *dis;
	Decoder *dec;
	BitShifter *bit;
public:
	iScreen()
	{
		
		bit = new BitShifter();
		dis = new Display();
		dec = new Decoder();
		dis->begin(SSD1306_SWITCHCAPVCC, 0x3C);
		dec->begin(2, 3, 4);
		bit->begin(12, 11, 10);
		bit->clear();
		bit->updateShifterRegister();
	}
	~iScreen()
	{
		if (dec)
		{
			delete dec;
			dec = nullptr;
		}
		if (dis)
		{
			delete dis;
			dis = nullptr;
		}
	}
	virtual void setup() = 0;
	virtual procedureEvent loop() = 0;
};


//Just forwardDeclarationo
class scSelectionExer;
class scIExercise;
class scExerciseA;
class scExerciseB;


class scSelectionExer
	: public iScreen
{
	String exercises[maxExercicies];
	int IdExe;
public:
	scSelectionExer()
	{
	}
	void setup();
	procedureEvent loop();
};

class scIExercise
	: public iScreen
{
public:
	int point, range;
	float upTime, downTime;

	scIExercise()
	{}
	~scIExercise()
	{
		Serial.println((char)32);
	}

	void getPointRange(String valA, String valB)
	{
		dis->pagePrint(valA);
		Serial.println(point);
		while (!dec->isPressed())
		{
			if (getData(point, 0, Leds - 1))
			{
				dis->pagePrint(valA);
				Serial.println(point + 1);
				bit->clear();
				bit->set(point, true);
				// for(byte x=0; x<Leds; x++)
				// {bit->set(x, (x>=point ? true:false));}
				bit->updateShifterRegister();
			}
		}
		while (dec->isPressed()) {}

		bit->clear();
		range = 20;
		dis->pagePrint(valB);
		Serial.println(range);
		while (!dec->isPressed())
		{
			if (getData(range, 0, (Leds - 1) - point))
			{				
				dis->pagePrint(valB);
				bit->clear();
				bit->set(range + point, true);
				Serial.println(range + point + 1);
				// for(byte x=point; x<Leds; x++)
				// {bit->set(x, (x<=(point+range)?true:false));}
				bit->updateShifterRegister();
			}
		}
		while (dec->isPressed()) {}
	}
	int getData(int &data, int Min = 0, int Max = -1)
	{
		int d = dec->getDirection();
		if (d != 0)
		{
			data += d;
			data = ((data > Max) ? Max : ((data < Min) ? Min : data));
			return 1;
		}
		return 0;
	}
	int getData(float &data, int Min = 0, int Max = -1, float pass=0.5f)
	{
		int d = dec->getDirection();
		if (d != 0)
		{
			data += (d*pass);
			data = ((data > Max) ? Max : ((data < Min) ? Min : data));
			return 1;
		}
		return 0;
	}
	void getTime()
	{
		dis->pagePrint("UpTime");
		downTime = upTime = 2;
		while (dec->isPressed()) {}
		while (!dec->isPressed())
		{
			if (getData(upTime, 0.5f, maxTime, dTime))
			{
				dis->pagePrint("UpTime");
				Serial.println(String(upTime) + String("s"));
			}
		}
		while (dec->isPressed()) {}

		dis->pagePrint("DownTime");
		Serial.println(downTime);
		while (!dec->isPressed())
		{
			if (getData(downTime, 0.5f, maxTime, dTime))
			{
				dis->pagePrint("DownTime");
				Serial.println(String(downTime) + String("s"));
			}
		}
		while (dec->isPressed()) {}
	}
	virtual void setup() = 0;
	virtual procedureEvent loop()=0;
};
class scExerciseA
	: public scIExercise
{
public:
	scExerciseA()
	{}
	void setup();
	procedureEvent loop()
	{
		do
		{
			bit->clear();
			const unsigned char pos = point + range;
			for (char i = point; i < pos; i++)
			{
				// Serial.print("->");
				// Serial.println(i, HEX);
				bit->clear();
				bit->set(i, 1);
				bit->updateShifterRegister();
				if (dec->isPressed())
					return procedureEvent(1, new scSelectionExer());
				delay(upTime);
			}
			bit->clear();
			for (char i = pos; i >= point; i--)
			{
				// Serial.print("->");
				// Serial.println(i, HEX);
				bit->clear();
				bit->set(i, 1);
				bit->updateShifterRegister();
				if (dec->isPressed())
					return procedureEvent(1, new scSelectionExer());
				delay(downTime);
			}
		} while (!dec->isPressed());
		return procedureEvent(1, new scSelectionExer());
	}
};
class scExerciseB
	: public scIExercise
{
public:
	scExerciseB()
	{}
	void setup();
	procedureEvent loop()
	{
		do
		{
			bit->clear();
			const unsigned char pos = point + range;
			for (char i = point; i < pos; i++)
			{
				// Serial.print("->");
				// Serial.println(i, HEX);
				bit->clear();
				bit->set(i, 1);
				bit->updateShifterRegister();
				if (dec->isPressed())
					return procedureEvent(1, new scSelectionExer());
				delay(upTime);
			}
			bit->clear();
			for (char i = pos; i >= point; i--)
			{
				// Serial.print("->");
				// Serial.println(i, HEX);
				bit->clear();
				bit->set(i, 1);
				bit->updateShifterRegister();
				if (dec->isPressed())
					return procedureEvent(1, new scSelectionExer());
				delay(downTime);
			}
		} while (!dec->isPressed());
		return procedureEvent(1, new scSelectionExer());
	}
};

void scSelectionExer::setup()
{
	IdExe = 0;
	exercises[0] = "Manual";
	exercises[1] = "Automatic";
	Serial.println((char)32);
	dis->pagePrint(exercises[IdExe]);
}
procedureEvent scSelectionExer::loop()
{
	if (dec->isPressed())
	{
		iScreen *iSc = nullptr;
		if(IdExe)
			iSc = new scExerciseB();
		else
			iSc = new scExerciseA();
		return procedureEvent(1, iSc);
	}
	else
	{
		int d = dec->getDirection();		
		if (d != 0)
		{			
			IdExe += d;
			if (IdExe > (maxExercicies-1))
				IdExe = maxExercicies - 1;
			else if (IdExe < 0)
				IdExe = 0;
			
			dis->pagePrint(exercises[IdExe]);
		}
	}
	return procedureEvent(0, nullptr);
}

void scExerciseA::setup()
{
	upTime = downTime = point = range = 0;

	bit->clear();
	bit->set(0, 1);
	bit->updateShifterRegister();
	
	getPointRange("Min", "Range");
	getTime();
	
	dis->pagePrint(String(upTime) + String("s"));
	Serial.println(String(downTime) + String("s"));

	upTime = (upTime * 1000) /range;
	downTime = (downTime * 1000) / range;
}
void scExerciseB::setup()
{
	upTime = downTime = point = range = 0;
	bit->clear();
	bit->set(0, 1);
	bit->updateShifterRegister();

	dis->pagePrint("Lenght");
	Serial.println(String(point) + String("cm"));
	point = 10;
	while (!dec->isPressed())
	{
		if (getData(point, 0, 120))
		{
			dis->pagePrint("Lenght");
			Serial.println(String(point) + String("cm"));
			bit->clear();
			bit->set(point, true);
			// for(byte x=0; x<Leds; x++)
			// {bit->set(x, (x>=point ? true:false));}
			bit->updateShifterRegister();
		}
	}
	while (dec->isPressed()) {}
	delay(50);
	bit->clear();
	range = 120;
	dis->pagePrint("Degress");
	Serial.println(range);
	while (!dec->isPressed())
	{
		if (getData(range, 0, 180))
		{
			dis->pagePrint("Degress");
			Serial.println(range);
			bit->clear();
			bit->set(range, true);
			// for(byte x=point; x<Leds; x++)
			// {bit->set(x, (x<=(point+range)?true:false));}
			bit->updateShifterRegister();
		}
	}
	while (dec->isPressed()) {}
	getTime();


	dis->pagePrint(String(upTime) + String("s"));
	Serial.println(String(downTime) + String("s"));
	range = (float)range/((float)180/(float)Leds);
	point = 0;
	Serial.print("->");
	Serial.println(range);
	upTime = (upTime * 1000) / range;
	downTime = (downTime * 1000) / range;
}






// int latchPin = 12;
// int clockPin = 11;
// int dataPin = 10;


// #define boards 5
// byte data[boards] = {0, 0, 0, 0, 0};


// void setup() 
// {
//   pinMode(latchPin, OUTPUT);
//   pinMode(dataPin, OUTPUT);  
//   pinMode(clockPin, OUTPUT);
//   Serial.begin(9600);  
// }
 
// void loop() 
// {
//   delay(1000);  
//   for(int x = 0; x < 5; x++)
//   {    
//     for (int i = 0; i < 8; i++)
//     {
//       int _bit = 1 << i;
//       data[x] = _bit;
//       Serial.print(x);    Serial.print("-");    Serial.println(data[x], BIN);
//       updateShiftRegister();
//       delay(500);
//     }
//     data[x] = 0;
//     delay(1000);
//   }  
// }

// void updateShiftRegister()
// {
//    digitalWrite(latchPin, LOW);
//    for(int x = (boards-1); x >= 0; x--)
//       shiftOut(dataPin, clockPin, LSBFIRST, data[x]);
//    digitalWrite(latchPin, HIGH);
// }
