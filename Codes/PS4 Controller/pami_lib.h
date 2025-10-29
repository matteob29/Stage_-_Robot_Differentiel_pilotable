#ifndef PAMI_LIB_H
#define PAMI_LIB_H

// broches du L298N (driver)
#define borneENA 14 // On associe la borne "ENA" du L298N à la pin ESP32
#define borneIN1 27 // On associe la borne "IN1" du L298N à la pin ESP32
#define borneIN2 26 // On associe la borne "IN2" du L298N à la pin ESP32
#define borneIN3 25 // On associe la borne "IN3" du L298N à la pin ESP32
#define borneIN4 33 // On associe la borne "IN4" du L298N à la pin ESP32
#define borneENB 32 // On associe la borne "ENB" du L298N à la pin ESP32

// broches capteur à ultrason
#define TRIG_PIN 22 // GPIO pour le trigger du capteur ultrason
#define ECHO_PIN 23 // GPIO pour l'echo du capteur ultrason
#define MAX_DISTANCE 200

// Déclaration des broches pour les feux
#define FEUX_AVANT_FAIBLE 19
#define FEUX_AVANT_FORT 18
#define FEUX_ARRIERE_FAIBLE 13
#define FEUX_ARRIERE_FORT 12

// LEDs de statut
#define LED_VERTE_PIN 5
#define LED_BLEUE_PIN 4
#define LED_ORANGE_PIN 0

#define MODE_SWITCH_PIN 15
#define MODE_WIFI HIGH
#define MODE_AUTO LOW

// broches des leds pour les phares
#define BEAM 50
#define HIGH_BEAM 50
#define STOP_LIGHT_PIN 2
#define RIGHT_RATIO 1
#define LEFT_RATIO 1

#define DRIVE_MODE_PIN 15
#define SPEED 250
#define DIAG_RATIO 0.3
#define TURN_RATIO 0.6

void beginPami();
void avancer(float speed, float ratio);
void reculer();
void droite();
void gauche();
void stop();
int mesurerDistance();
void mouvementJoystick(float x, float y);
void controlerFeuxArriere(int vitesse);
void remote_control();
void trim_left();
void trim_right();

#endif
