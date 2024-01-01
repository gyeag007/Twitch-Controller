/*
PROJECT: TV Volume Guard - stolen from http://www.instructables.com/id/TV-Volume-Loudness-Guard-using-Arduino/
AUTHOR: Hazim Bitar (techbitar)
DATE: FEB 9, 2013
CONTACT: techbitar at gmail dot com
LICENSE: My code is in the public domain.
IRremote library: copyright by Ken Shirriff http://arcfn.com
Modified to raise volume when commercial is over (volume is too low).
Also modified to read peak-to-peak sound values - from Adafruit "MeasuringSoundLevels" sketch.
A0 is from microphone analog output.
*/

#include "PinDefinitionsAndMore.h"
#include <IRremote.h>
 
#define NOISE_LEVEL_MAX    340       // Max level of noise to detect from 0 to 1023
#define NOISE_LEVEL_MIN    170        // Min level of noise to detect from 0 to 1023
#define MUTE_LEVEL_MIN     25
#define TWITCH_FROZEN_LEVEL_MIN 15        // Min level of noise to catch mute
#define REPEAT_TX          3          // how many times to transmit the IR remote code
#define IR_SEND_PIN         3 
#define LED                13         // pin for LED used to blink when volume too high
#define SWITCH_READ_PIN 2
int switchOn = 0;
int AmbientSoundLevel = 260;            // Microphone sensor initial value start high!!!
const int sampleWindow = 50;          // Sample window width in mS (50 mS = 20Hz)
long twitch_time_sent = 0;
long twitch_send_interval = 20000; //milliseconds from last time sent
float bias = 0.7;

void setup()
{
  pinMode(LED, OUTPUT);
  pinMode(SWITCH_READ_PIN, INPUT);

  Serial.begin(9600);
  Serial.println("Twitch controller");
  Serial.print("NOISE_LEVEL_MIN is ");
  Serial.println(NOISE_LEVEL_MIN);
  Serial.print("NOISE_LEVEL_Max is ");
  Serial.println(NOISE_LEVEL_MAX);
  Serial.print("Ambient sound level: ");
  Serial.println(AmbientSoundLevel);
  Serial.println("-------------------");
}
 
void loop()
{
    switchOn = digitalRead(SWITCH_READ_PIN);
    Serial.print("switchOn: ");
    Serial.println(switchOn);
    AmbientSoundLevel = getAmbientSoundLevel();
    Serial.print("Ambient sound level: ");
    Serial.println(AmbientSoundLevel);

    if ((AmbientSoundLevel < TWITCH_FROZEN_LEVEL_MIN) && switchOn && (millis() - twitch_time_sent >= twitch_send_interval))
    {
      Serial.println("Twitch stopped");
    //  while(1){
      //        Serial.println("Twitch stopped");
//
  //            IrSender.sendSony(0x1, 0x75, 2, 12); //down arrow
    //  delay(2000);
      //}
      digitalWrite(LED, HIGH);  // LED on
      IrSender.sendSony(0x1, 0x75, 2, 12); //down arrow
      delay(500);
      IrSender.sendSony(0x1, 0x65, 2, 12); //select key
      delay(6000);
      IrSender.sendSony(0x1, 0x74, 2, 12); //up arrow
      delay(500);
      IrSender.sendSony(0x1, 0x65, 2, 12); //select key
      twitch_time_sent = millis();
      digitalWrite(LED, LOW); // LED off
    }
    //digitalWrite(LED, LOW); // LED off
}
 
int getAmbientSoundLevel()
{
  unsigned long startMillis; // Start of sample window
  unsigned int peakToPeak = 0;   // peak-to-peak level
  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;
  unsigned int sample;
  unsigned int samples[100];             // Array to store samples
  int sampleAvg = 0;
  long sampleSum = 0;

 
  for (int i = 0; i <= 100; i++) {
    startMillis = millis(); // Start of sample window

    while (millis() - startMillis < sampleWindow){  // collect data for 50 mS
      sample = analogRead(0);

      if (sample < 1024 && sample > 0)  // toss out spurious readings
      {
        if (sample > signalMax)
        {
          signalMax = sample;  // save just the max levels
        }
        else if (sample < signalMin)
        {
          signalMin = sample;  // save just the min levels
        }
      }
    }

    peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
    samples[i] = peakToPeak;
  }
  for (int i = 0; i <= 100; i++) { 
    sampleSum = sampleSum + samples[i];
  }

  sampleAvg = sampleSum/100;
  Serial.print("sampleavg: ");
  Serial.println(sampleAvg);
  sampleAvg = (sampleAvg * bias) + (AmbientSoundLevel * (1- bias));

  return sampleAvg;
}
