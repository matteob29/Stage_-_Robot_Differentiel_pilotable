#include <Arduino.h>
#include "pami_lib.h"



void setup()
{

  pinMode(MODE_SWITCH_PIN, INPUT_PULLUP);
  Serial.begin(115200);
  beginPami();

}

void loop()
{
  remote_control();
  // 🤖 Mode automatique : comportement intelligent
  if (digitalRead(MODE_SWITCH_PIN) == MODE_AUTO)
  {
    static enum { AVANCER,
                  STOP,
                  RECULER,
                  TOURNER } etat = AVANCER;
    static unsigned long actionStartTime = 0;
    int distance = mesurerDistance();

    switch (etat)
    {
    case AVANCER:
      avancer();
      if (distance <= 15)
      {
        stop();
        etat = STOP;
        actionStartTime = millis();
        Serial.println("Obstacle détecté — arrêt !");
      }
      break;

    case STOP:
      if (millis() - actionStartTime > 500)
      { // petite pause
        reculer();
        etat = RECULER;
        actionStartTime = millis();
        Serial.println("Recul...");
      }
      break;

    case RECULER:
      if (millis() - actionStartTime > 400)
      {           // recule 400ms
        droite(); // petite rotation droite
        etat = TOURNER;
        actionStartTime = millis();
        Serial.println("Rotation à droite...");
      }
      break;

    case TOURNER:
      if (millis() - actionStartTime > 400)
      { // tourne ≈ 45°
        etat = AVANCER;
        Serial.println("Reprise de l'avance");
      }
      break;
    }
  }
}
