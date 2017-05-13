

#define LOG_OUT 1 // use the log output function
#define FHT_N 128 // set to 128 point fft

#include <FHT.h> // include the library

#include <elapsedMillis.h>

elapsedMillis timeElapsed; //declare global if you don't want it reset every time loop runs

const int initDuration = 300;

int ampMaxFreq1 = 0;
int ampMaxFreq2 = 0;
int ampMaxFreq3 = 0;

int scale = 0;

int idFrequency1 = 30;
int idFrequency2 = 8;
int idFrequency3 = 70;

int frequencyAmp1;
int frequencyAmp2;
int frequencyAmp3;

int zero1 = 130;
int zero2 = 130;
int zero3 = 130;

int bruit1 = 0;
int bruit2 = 0;
int bruit3 = 0;


//LED Pins
int led1 = 11;
int led2 = 10;
int led3 = 9;

unsigned long oldTime = 0;
unsigned long newTime;

/***********************************************/
/*******          fftComputation         *******/
/***********************************************/
void fftComputation()
{
    cli();  // UDRE interrupt slows this way down on arduino1.0
    for (int i = 0 ; i < FHT_N ; i++) 
    { // save FHT_N samples
      while(!(ADCSRA & 0x10)); // wait for adc to be ready
      ADCSRA = 0xf5; // restart adc
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int
      fht_input[i] = k; // put real data into bins
    }
    fht_window(); // window the data for better frequency response
    fht_reorder(); // reorder the data before doing the fft
    fht_run(); // process the data in the fft
    fht_mag_log(); // take the output of the fft
    sei();
}

void initZeroAndNoise( int& zero1, int& zero2, int& zero3, int& bruit1, int& bruit2, int& bruit3)
{
  // On initialise les zeros
  zero1 = 0;
  zero2 = 0;
  zero3 = 0;

  // On initialise le calcul du bruit
  int maxBruit1 = 0;
  int maxBruit2 = 0;
  int maxBruit3 = 0;

  int minBruit1 = 10000;
  int minBruit2 = 10000;
  int minBruit3 = 10000;

  // Pour normaliser a la fin 
  int nbrValue = 0;

  // On enregistre le temps de debut de l initialisation
  oldTime = timeElapsed;

  // On va faire une moyenne sur une duree egale a initDuration
  while( timeElapsed - oldTime < initDuration) 
  { 
    fftComputation();

    // On releve les frequences
    frequencyAmp1 = fht_log_out[idFrequency1];
    frequencyAmp2 = fht_log_out[idFrequency2];
    frequencyAmp3 = fht_log_out[idFrequency3];

    // Calcul du Zero
    zero1 = zero1 + frequencyAmp1;
    zero2 = zero2 + frequencyAmp2;
    zero3 = zero3 + frequencyAmp3;

    // Calcul du bruit
    maxBruit1 = max(maxBruit1,frequencyAmp1);
    minBruit1 = min(minBruit1,frequencyAmp1);

    maxBruit2 = max(maxBruit2,frequencyAmp2);
    minBruit2 = min(minBruit2,frequencyAmp2);

    maxBruit3 = max(maxBruit3,frequencyAmp3);
    minBruit3 = min(minBruit3,frequencyAmp3);

    // Nombre d'échantillons
    nbrValue ++;

    Serial.println(timeElapsed);
  }

  // Normalisation
  zero1 = zero1/nbrValue;
  zero2 = zero2/nbrValue;
  zero3 = zero3/nbrValue;

  // Calcul du bruit
  bruit1 = max(abs(maxBruit1 - zero1),abs(minBruit1 - zero1));
  bruit2 = max(abs(maxBruit2 - zero2),abs(minBruit2 - zero2));
  bruit3 = max(abs(maxBruit3 - zero3),abs(minBruit3 - zero3));
  
}

void setup() 
{
  // Initialisation de l afichage sur PC 
  Serial.begin(9600); // use the serial port

  // Initialisation FFT
 // TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0

  // On definit les pin des leds en OUTPUT
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);

  // On eteint la led de l arduino. Cela signifie que l initialisation est en cours.
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  // On initialise les zeros des frequences et leur bruit.
  initZeroAndNoise( zero1, zero2, zero3, bruit1, bruit2, bruit3);

  // On allume la LED de l'arduino pour indiquer que l initialisation est terminee
  // On peut alors monter le son
  digitalWrite(13, HIGH);
}

void loop() 
{
  while(1) 
  { // reduces jitter
    fftComputation();

    // On releve les frequences
    frequencyAmp1 = abs(fht_log_out[idFrequency1] - zero1);
    frequencyAmp2 = abs(fht_log_out[idFrequency2] - zero2);
    frequencyAmp3 = abs(fht_log_out[idFrequency3] - zero3);

    // LED 1

    // On regle le max si besoin
    if (frequencyAmp1 > ampMaxFreq1)
    {
      ampMaxFreq1 = frequencyAmp1;
    }

    // On calcule l echelle a appliquer au signal
    scale = 255/ampMaxFreq1; 

    //filtre + Allumage Led
    if (frequencyAmp1 > (bruit1/2) )
    {
      analogWrite(led1, frequencyAmp1*scale);
      
    }else
    {
      analogWrite(led1, 0);      
    }

    // LED 2

    // On regle le max si besoin
    if (frequencyAmp2 > ampMaxFreq2)
    {
      ampMaxFreq2 = frequencyAmp2;
    }

    // On calcule l echelle a appliquer au signal
    scale = 255/ampMaxFreq2; 

    //filtre + Allumage Led
    if (frequencyAmp2 > (bruit2/2) )
    {
      analogWrite(led2, frequencyAmp2*scale);
      
    }else
    {
      analogWrite(led2, 0);      
    }

    // LED 3

    // On regle le max si besoin
    if (frequencyAmp3 > ampMaxFreq3)
    {
      ampMaxFreq3 = frequencyAmp3;
    }

    // On calcule l echelle a appliquer au signal
    scale = 255/ampMaxFreq3; 

    //filtre + Allumage Led
    if (frequencyAmp3 > (bruit3/2) )
    {
      analogWrite(led3, frequencyAmp2*scale);
      
    }else
    {
      analogWrite(led3, 0);      
    }

    // Serial.println(fft_log_out[idFrequency3]);
    newTime = timeElapsed;
    Serial.println(newTime);
  }
}
