/*
  ==Exploitation de l'anémomètre avec affichage sur écran OLED==

  Connections

  Capteur IR | Arduino
  Vcc        | +3.3V
  GND        | GND
  Data       | A2
  No         | -

  Ecran OLED | Arduino
  GND        | GND
  Vdd        | +3.3V
  SCL        | A5 (Uno, Nano)
  SDA        | A4 (Uno, Nano)
*/

#include <Wire.h> //Interface de communication I2C pour l'écran OLED
#include <Adafruit_GFX.h> //Bibliothèque de gestion de l'écran
#include <Adafruit_SSD1306.h> //idem

#define OLED_RESET 4 //Définie le pin de connection de ???
Adafruit_SSD1306 oled_screen(OLED_RESET); //création d'un objet gérant l'écran

#define pinSensor A2 //pin analogique récupérant la valeur du capteur IR

int valSensor = 0; //variable de lecture du capteur IR
int detectionsNb = 0; //variable gérant le nombre de détections du capteur IR
const int detection_delay = 15000; //délai de la détection en ms
const int radiusNb = 5; //nombre de rayons de la roue codeuse
bool pos; //permet de stocker l'état de la détection
float angularSpeedCoeff; //coefficient de convertion du nombre de détections à la vitesse angulaire
float angularSpeed; //vitesse angulaire
float linearSpeedCoeff; //coefficient de convertion vitesse angulaire/linéaire
float linearSpeed; //vitesse linéaire en km/h
const int kmHCoeff = 3.6; //coefficient de convertion m/s vers km/h
float kmHSpeed; //vitesse linéaire en km/h

void setup() {
  /*
    Calcul des coefficients: le nombres de rayons détectés durant l'intervalle sera divisé par la durée de
    l'intervalle et par le nombres de rayons de la roue pour obtenir une vitesse de rotation en tours/s.
  */
  angularSpeedCoeff = 1.0/((detection_delay/1000)*radiusNb);

  oled_screen.begin(SSD1306_SWITCHCAPVCC, 0x3C); //initialisation de l'écran OLED sur l'adresse I2C 0x3C
  //l'adresse devra être modifiée selon le format d'écran (ici 128*32)
  
  delay(2000); //délai d'initialisation de l'écran
  oled_screen.clearDisplay(); //reset de l'écran
}

void loop() {
  valSensor = analogRead(pinSensor); //première lecture de la valeur du capteur IR

    /*La capteur est utilisé en tout ou rien. On stock dans un booléen de la position détectée,
    si le capteur renvoie une valeur supérieure à 600, c'est un rayon de la roue,
    sinon c'est le vide entre deux rayons.
    */
    if (valSensor >= 600){
      pos = true;
    }
    else if (valSensor < 600){
      pos = false;
    }
  
  for(int i=0;i<detection_delay;i++){ //nous allons maintenant lire la valeur du capteur pendant l'intervalle
    
    valSensor = analogRead(pinSensor);
    
    if (valSensor >= 600 && pos == false){ //si on repère un front montant c'est le début d'un rayon
      pos = true;
      detectionsNb += 1; //on comptabilise ce front
    }
    else if (valSensor < 600 && pos == true){ //les fronts descendants ne sont pas comptabilisés
      pos = false;
    }
    
    delay(1); //on s'assure d'au minimum une ms de délai pour respecter l'intervalle
  }
  
  /*
  Cette vitesse de rotation pourra alors être multiplié par un coefficient permettant d'obtenir la
  vitesse du vent en m/s. Enfin cette dernière pourra être convertie en km/h.
  */

  //paramètres d'affichages
  oled_screen.clearDisplay();
  oled_screen.setTextSize(1);
  oled_screen.setTextColor(WHITE);
  oled_screen.setCursor(15,0);

  //vitesse de rotation
  angularSpeed = angularSpeedCoeff * detectionsNb;
  oled_screen.print("Vitesse du vent: ");
  oled_screen.setCursor(30,8);
  oled_screen.print(angularSpeed);
  oled_screen.print(" tours/s");

  //vitesse linéaire
  //en m/s
  linearSpeed = linearSpeedCoeff * angularSpeed;
  oled_screen.setCursor(30,16);
  oled_screen.print(linearSpeed);
  oled_screen.print(" m/s");

  //en km/h
  kmHSpeed = kmHCoeff * linearSpeed;
  oled_screen.setCursor(30,24);
  oled_screen.print(kmHSpeed);
  oled_screen.print(" km/h");

  //affichage
  oled_screen.display();

  //remise à 0 du nb de détections
  detectionsNb = 0;
  delay(10);
}
