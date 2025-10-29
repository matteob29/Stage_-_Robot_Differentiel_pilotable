#include "Arduino.h"
#include "pami_lib.h"
#include "NewPing.h"

#define TRIMINC 0.05

NewPing observer(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);
float ltrim = 1;
float rtrim = 1;

void trim_left(){
  if(rtrim < 1-TRIMINC){
    rtrim +=TRIMINC;
  }
  else if(ltrim > 0.5 + TRIMINC){
    ltrim -= TRIMINC;
  }
}

void trim_right(){
  if(ltrim < 1 - TRIMINC){
    ltrim += TRIMINC;
  }
  else if(rtrim > 0.5 + TRIMINC){
    rtrim -= TRIMINC;
  }
}

/**
 * @brief Fonction d'initialisation du contrôle des moteurs du PAMI
 *
 * Cette fonction doit être appelée dans le Setup().
 */
void beginPami()
{
  pinMode(borneENA, OUTPUT);
  pinMode(borneIN1, OUTPUT);
  pinMode(borneIN2, OUTPUT);
  pinMode(borneIN3, OUTPUT);
  pinMode(borneIN4, OUTPUT);
  pinMode(borneENB, OUTPUT);

  stop();

  pinMode(BEAM, OUTPUT);
  pinMode(HIGH_BEAM, OUTPUT);
  pinMode(STOP_LIGHT_PIN, OUTPUT);

  

  // Initialisation des LED de statut
  pinMode(LED_VERTE_PIN, OUTPUT);
  pinMode(LED_BLEUE_PIN, OUTPUT);
  pinMode(LED_ORANGE_PIN, OUTPUT); // Allumée en permanence
  digitalWrite(LED_ORANGE_PIN, HIGH);

  // Configuration des broches pour les LED
  pinMode(FEUX_AVANT_FAIBLE, OUTPUT);
  pinMode(FEUX_ARRIERE_FAIBLE, OUTPUT);

  // Assurez-vous que les LED sont éteintes au début
  digitalWrite(FEUX_AVANT_FAIBLE, LOW);
  digitalWrite(FEUX_ARRIERE_FAIBLE, LOW);
}

// contrôle des moteurs //
enum mode_moteur
{
  MARCHE_AVANT,
  MARCHE_ARRIERE,
};

/**
 * @brief Cette fonction permet de choisir le sens de rotation des moteurs de droite et de gauche
 *
 * Les options sont : MARCHE_AVANT ou MARCHE_ARRIERE.
 *
 * @param mode_roue_gauche Sens de rotation désiré pour la roue de gauche
 *
 * @param mode_roue_droite Sens de rotation désiré pour la roue de droite
 */
void rotation_moteurs(enum mode_moteur mode_roue_gauche, enum mode_moteur mode_roue_droite)
{

  switch (mode_roue_gauche)
  {
  case MARCHE_AVANT:
  
    digitalWrite(borneIN3, LOW); 
    digitalWrite(borneIN4, HIGH);
    break;

  case MARCHE_ARRIERE:
    digitalWrite(borneIN3, HIGH); 
    digitalWrite(borneIN4, LOW);
    break;
  }

  switch (mode_roue_droite)
  {
  case MARCHE_AVANT:
    digitalWrite(borneIN1, HIGH); 
    digitalWrite(borneIN2, LOW);
    break;

  case MARCHE_ARRIERE:
    digitalWrite(borneIN1, LOW); 
    digitalWrite(borneIN2, HIGH);
    break;
  }
}

// fonction de mesure avec le capteur à ultrasons //
/**
 * @brief Cette fonction renvoie la distance en cm captée par le détecteur à ultrasons à l'avant du robot
 */
int mesurerDistance()
{
  int distance = observer.ping_cm();
  if (!distance)
    distance = MAX_DISTANCE;
  return distance;
}

/**
 * @brief Fait avancer le PAMI tout droit
 *
 * Définis le sens de rotation des deux moteurs comme MARCHE_AVANT, et leur commande la même vitesse.
 */
void avancer(float speed, float ratio)
{
  Serial.print(speed);
  Serial.println(ratio);
  if(speed == 0) stop();
  else if(speed > 0) rotation_moteurs(MARCHE_AVANT, MARCHE_AVANT);
  else{
    rotation_moteurs(MARCHE_ARRIERE, MARCHE_ARRIERE);
    speed = -speed;
  }
  
  float lratio = speed;
  float rratio = speed;

  if(ratio < 0) lratio *= 1+ratio;
  else if(ratio>0) rratio *= 1-ratio;

  analogWrite(borneENA, SPEED * rtrim * rratio);
  analogWrite(borneENB, SPEED * ltrim * lratio);
}

/**
 * @brief Fait avancer le PAMI tout droit en marche arrière
 *
 * Définis le sens de rotation des deux moteurs comme MARCHE_ARRIERE, et leur commande la même vitesse.
 */
void reculer()
{
  rotation_moteurs(MARCHE_ARRIERE, MARCHE_ARRIERE);
  analogWrite(borneENA, SPEED * RIGHT_RATIO);
  analogWrite(borneENB, SPEED * LEFT_RATIO);
}

/**
 * @brief Fait tourner le PAMI sur lui même dans le sens horaire
 *
 * Définis le sens de rotation du moteur droit comme MARCHE_ARRIERE, celui du moteur gauche comme MARCHE_AVANT, et leur commande la même vitesse.
 */
void droite()
{
  rotation_moteurs(MARCHE_AVANT, MARCHE_ARRIERE);
  analogWrite(borneENA, SPEED * RIGHT_RATIO * TURN_RATIO);
  analogWrite(borneENB, SPEED * LEFT_RATIO * TURN_RATIO);
}

/**
 * @brief Fait tourner le PAMI sur lui même dans le sens anti-horaire
 *
 * Définis le sens de rotation du moteur droit comme MARCHE_AVANT, celui du moteur gauche comme MARCHE_ARRIERE, et leur commande la même vitesse.
 */
void gauche()
{
  rotation_moteurs(MARCHE_ARRIERE, MARCHE_AVANT);
  analogWrite(borneENA, SPEED * RIGHT_RATIO * TURN_RATIO);
  analogWrite(borneENB, SPEED * LEFT_RATIO * TURN_RATIO);
}
void diagonale_avant_gauche()
{
  rotation_moteurs(MARCHE_AVANT, MARCHE_AVANT);
  analogWrite(borneENA, SPEED * RIGHT_RATIO); // Active l'alimentation du moteur A
  analogWrite(borneENB, SPEED * LEFT_RATIO * DIAG_RATIO);
}

void diagonale_avant_droite()
{
  rotation_moteurs(MARCHE_AVANT, MARCHE_AVANT);
  analogWrite(borneENA, SPEED * RIGHT_RATIO * DIAG_RATIO); // Active l'alimentation du moteur A
  analogWrite(borneENB, SPEED * LEFT_RATIO);
}

void diagonale_arriere_droite()
{
  rotation_moteurs(MARCHE_ARRIERE, MARCHE_ARRIERE);
  analogWrite(borneENA, SPEED * RIGHT_RATIO * DIAG_RATIO); // Active l'alimentation du moteur A
  analogWrite(borneENB, SPEED * LEFT_RATIO);
}
void diagonale_arriere_gauche()
{
  rotation_moteurs(MARCHE_ARRIERE, MARCHE_ARRIERE);
  analogWrite(borneENA, SPEED * RIGHT_RATIO); // Active l'alimentation du moteur A
  analogWrite(borneENB, SPEED * LEFT_RATIO * DIAG_RATIO);
}

void mouvementJoystick(float x, float y)
{
  // Calculer la vitesse et la direction pour chaque moteur
  int vitesseGauche = SPEED * LEFT_RATIO * (y - x);
  int vitesseDroite = SPEED * RIGHT_RATIO * (y + x);

  // Normaliser les vitesses pour qu'elles soient dans la plage de 0 à 255
  vitesseGauche = constrain(vitesseGauche, -255, 255);
  vitesseDroite = constrain(vitesseDroite, -255, 255);

  // Déterminer la direction des moteurs
  if (y > 0)
  {
    rotation_moteurs(MARCHE_AVANT, MARCHE_AVANT);
  }
  else
  {
    rotation_moteurs(MARCHE_ARRIERE, MARCHE_ARRIERE);
  }

  // Appliquer les vitesses aux moteurs
  analogWrite(borneENA, abs(vitesseDroite));
  analogWrite(borneENB, abs(vitesseGauche));
}

void beams(bool activation)
{
  digitalWrite(BEAM, activation ? HIGH : LOW);
}

void high_beams(bool activation)
{
  digitalWrite(HIGH_BEAM, activation);
}

void stop_lights(bool activation)
{
  digitalWrite(STOP_LIGHT_PIN, activation ? HIGH : LOW);
}

void controlerFeuxArriere(int vitesse)
{
  // Définir un seuil de vitesse pour allumer les feux arrière
  const int seuilVitesse = 100; // Seuil de vitesse pour allumer les feux arrière

  if (vitesse < seuilVitesse)
  {
    digitalWrite(FEUX_ARRIERE_FAIBLE, HIGH); // Allumer les feux arrière
  }
  else
  {
    digitalWrite(FEUX_ARRIERE_FAIBLE, LOW); // Éteindre les feux arrière
  }
}

/**
 * @brief Fait arrêter le PAMI
 *
 * Commande une vitesse nulle aux deux moteurs.
 */
void stop()
{
  analogWrite(borneENA, 0);
  analogWrite(borneENB, 0);
}

void remote_control(){

 
}