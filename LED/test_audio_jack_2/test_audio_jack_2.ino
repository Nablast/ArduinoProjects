/* David Wang
   Code that takes audio input from a 3.5mm cable
   and flashes an LED strip based on the frequency
   of the music.

   HUGE thanks to the arduino community
   If you see your code here, I owe you my gratitude

*/
int sensorPin = A0;

int filterValue = 80;

// LED pins connected to the PWM pins on the Arduino

int ledPinR = 9;
int ledPinG = 10;
int ledPinB = 11;

int sensorValue;

void setup()
{
  Serial.begin(9600);
  // Read from MSGEQ7 OUT
  pinMode(sensorPin, INPUT);
  
  // Write to MSGEQ7 STROBE and RESET

  // Set analogPin's reference voltage
  analogReference(DEFAULT); // 5V

  Serial.println("ok boy");
}

void loop()
{

sensorValue = analogRead(sensorPin);

Serial.print(sensorValue);

Serial.println();

  // analogWrite(ledPinR, 127); // bleu !
  // analogWrite(ledPinG, 127); // rouge !
}

