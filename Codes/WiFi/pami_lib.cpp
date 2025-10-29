#include "Arduino.h"
#include "pami_lib.h"
#include "NewPing.h"
#include <WiFi.h>
#include <WebServer.h>
#include <vector>

// Déclaration des variables globales
unsigned long lastCommandTime = 0;
const unsigned long commandTimeout = 1000; // Timeout de 1 seconde
bool isMovingForward = false;

// Réseau Wi-Fi (mode point d'accès)
const char *ssid = "PAMI_WIFI";
const char *password = "12345678";

WebServer server(80);

// Page HTML à afficher sur Safari
template <typename T>
String buttonRow(const T &commands)
{
  String html = "<div style='margin:10px;'>";
  for (auto &cmd : commands)
    html += "<button onclick=\"fetch('/" + String(cmd) + "')\" style='width:80px;height:80px;font-size:24px;margin:5px;'>" + String(cmd[0]) + "</button>";
  return html + "</div>";
}

String controlPage()
{
  String html = "<html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body { font-family: sans-serif; margin: 0; padding: 0; }";
  html += ".container { display: flex; justify-content: space-between; padding: 20px; }";
  html += ".left, .right { display: flex; flex-direction: column; align-items: center; }";
  html += "button { -webkit-user-select: none; user-select: none; width: 80px; height: 80px; font-size: 24px; margin: 5px; }";
  html += "#distance-display, #mode-display { font-size: 20px; margin: 10px; }";
  html += "</style>";

  html += "<script>";
  html += "function sendCommand(cmd) { fetch('/' + cmd); }";
  html += "function updateDistance() {";
  html += "  fetch('/distance')"
          "    .then(res => res.text())"
          "    .then(text => {"
          "      document.getElementById('distance-display').textContent = text;"
          "    });";
  html += "}";
  html += "function updateMode() {";
  html += "  fetch('/mode')"
          "    .then(res => res.text())"
          "    .then(text => {"
          "      document.getElementById('mode-display').textContent = text;"
          "    });";
  html += "}";
  html += "setInterval(() => { updateDistance(); updateMode(); }, 500);";
  html += "</script>";

  html += "</head><body>";
  html += "<h2 style='text-align:center;'>Controle PAMI</h2>";
  html += "<div class='container'>";

  // ✅ Partie gauche : flèches de direction
  html += "<div class='left'>";
  html += "<button onmousedown='sendCommand(\"avancer\")' onmouseup='sendCommand(\"stop\")' ";
  html += "onmouseleave='sendCommand(\"stop\")' ontouchstart='sendCommand(\"avancer\")' ontouchend='sendCommand(\"stop\")'>&#8593;</button>";

  html += "<div>";
  html += "<button onmousedown='sendCommand(\"gauche\")' onmouseup='sendCommand(\"stop\")' ";
  html += "onmouseleave='sendCommand(\"stop\")' ontouchstart='sendCommand(\"gauche\")' ontouchend='sendCommand(\"stop\")'>&#8592;</button>";

  html += "<button onclick='sendCommand(\"stop\")'>&#8597;</button>";

  html += "<button onmousedown='sendCommand(\"droite\")' onmouseup='sendCommand(\"stop\")' ";
  html += "onmouseleave='sendCommand(\"stop\")' ontouchstart='sendCommand(\"droite\")' ontouchend='sendCommand(\"stop\")'>&#8594;</button>";
  html += "</div>";

  html += "<button onmousedown='sendCommand(\"reculer\")' onmouseup='sendCommand(\"stop\")' ";
  html += "onmouseleave='sendCommand(\"stop\")' ontouchstart='sendCommand(\"reculer\")' ontouchend='sendCommand(\"stop\")'>&#8595;</button>";
  html += "</div>";

  // ✅ Partie droite : mode, bouton lumière, distance
  html += "<div class='right'>";
  html += "<div id='mode-display'>Mode : --</div>";
  html += "<button onclick='sendCommand(\"feux_croisement\")' style='background-color: #FFD700;'>&#128161;</button>";
  html += "<div id='distance-display'>Distance : --</div>";
  html += "</div>";

  html += "</div>"; // .container
  html += "</body></html>";

  return html;
}



NewPing observer(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

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

  

  WiFi.softAP(ssid, password);
  Serial.print("Connecté au Wi-Fi AP. IP: ");
  Serial.println(WiFi.softAPIP());

  if (WiFi.softAP(ssid, password))
  {
    Serial.println("✅ Point d'accès Wi-Fi créé avec succès !");
    Serial.print("IP du robot : ");
    Serial.println(WiFi.softAPIP());
  }
  else
  {
    Serial.println("❌ Échec de création du Wi-Fi !");
  }

  server.on("/", []()
            { server.send(200, "text/html", controlPage()); });

  server.on("/avancer", []()
            {
    if (digitalRead(MODE_SWITCH_PIN) == MODE_WIFI) {
      int distance = mesurerDistance();
      if (distance > 15) {
        avancer();
        isMovingForward = true;  // ✅ important
        server.send(204, "");
      } else {
        stop();
        isMovingForward = false;
        server.send(200, "text/plain", "Obstacle détecté !");
      }
    } else {
      server.send(200, "text/plain", "❌ Mode automatique actif");
    } });

  server.on("/reculer", []()
            {
    reculer();
    server.send(204, ""); });

  server.on("/gauche", []()
            {
    gauche();
    server.send(204, ""); });

  server.on("/droite", []()
            {
    droite();
    server.send(204, ""); });

  server.on("/stop", []()
            {
    stop();
    isMovingForward = false;
    server.send(204, ""); });

  server.on("/feux_croisement", []()
            {
    static bool lightState = false;
    lightState = !lightState;
    digitalWrite(FEUX_AVANT_FAIBLE, lightState ? HIGH : LOW);
    digitalWrite(FEUX_ARRIERE_FAIBLE, lightState ? HIGH : LOW);
    server.send(204, ""); });

  server.on("/distance", []()
            {
    int d = mesurerDistance();
    if (d >= MAX_DISTANCE) {
      server.send(200, "text/plain", "MAX");
    } else {
      server.send(200, "text/plain", String(d) + " cm");
    } });

  server.on("/mode", []()
            {
    if (digitalRead(MODE_SWITCH_PIN) == MODE_WIFI) {
      server.send(200, "text/plain", "Mode : Wi-Fi");
    } else {
      server.send(200, "text/plain", "Mode : Automatique");
    } });

  server.begin();
  Serial.println("Serveur HTTP lancé.");
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
    digitalWrite(borneIN3, LOW); // Moteur B marche avant
    digitalWrite(borneIN4, HIGH);
    break;

  case MARCHE_ARRIERE:
    digitalWrite(borneIN3, HIGH); // Moteur B marche arrière
    digitalWrite(borneIN4, LOW);
    break;
  }

  switch (mode_roue_droite)
  {
  case MARCHE_AVANT:
    digitalWrite(borneIN1, LOW); // Moteur A marche avant
    digitalWrite(borneIN2, HIGH);
    break;

  case MARCHE_ARRIERE:
    digitalWrite(borneIN1, HIGH); // Moteur A marche arrière
    digitalWrite(borneIN2, LOW);
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
void avancer()
{
  rotation_moteurs(MARCHE_AVANT, MARCHE_AVANT);
  analogWrite(borneENA, SPEED * RIGHT_RATIO);
  analogWrite(borneENB, SPEED * LEFT_RATIO);
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
  server.handleClient();

  static int lastMode = -1;
  int currentMode = digitalRead(MODE_SWITCH_PIN);

  // 🔁 Changement dynamique de mode
  if (currentMode != lastMode)
  {
    lastMode = currentMode;

    if (currentMode == MODE_WIFI)
    {
      digitalWrite(LED_BLEUE_PIN, HIGH);
      digitalWrite(LED_VERTE_PIN, LOW);
      Serial.println("🔵 Mode Wi-Fi activé.");
      stop(); // arrêter tout si on repasse en Wi-Fi
    }
    else
    {
      digitalWrite(LED_VERTE_PIN, HIGH);
      digitalWrite(LED_BLEUE_PIN, LOW);
      Serial.println("🟢 Mode automatique activé.");
    }
  }

  

  if (digitalRead(MODE_SWITCH_PIN) == MODE_WIFI)
  {
    if (isMovingForward)
    {
      int distance = mesurerDistance();
      if (distance <= 15)
      {
        stop();
        isMovingForward = false;
        Serial.println("🛑 Obstacle soudain ! Stop forcé (Wi-Fi).");
      }
    }
  }
}