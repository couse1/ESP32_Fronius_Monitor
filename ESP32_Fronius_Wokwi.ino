/*
 * ESP32 Fronius Solar API Monitor - Version Wokwi
 * 
 * Version simplifiée pour la simulation Wokwi
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Configuration des broches
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RESET -1
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

#define RGB_RED_PIN 25
#define RGB_GREEN_PIN 26
#define RGB_BLUE_PIN 27
#define COMMON_ANODE false  // Wokwi utilise une LED RGB à cathode commune

// Déclaration de l'écran OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Variables pour stocker les données
float productionPower = 0;    // Production en watts
float consumptionPower = 0;   // Consommation en watts
float gridPower = 0;          // Puissance du réseau (+ = import, - = export)
float batteryPower = 0;       // Puissance de la batterie (si présente)
float batterySOC = 0;         // État de charge de la batterie (si présente)

// Variables pour la simulation
unsigned long lastUpdateTime = 0;
unsigned long lastSimulationUpdate = 0;
int simulationStep = 0;
const unsigned long UPDATE_INTERVAL = 5000; // 5 secondes pour la simulation

void setup() {
  // Initialisation du port série
  Serial.begin(115200);
  Serial.println("ESP32 Fronius Solar API Monitor - Simulation Wokwi");
  
  // Configuration des broches pour la LED RGB
  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_GREEN_PIN, OUTPUT);
  pinMode(RGB_BLUE_PIN, OUTPUT);
  
  // Éteindre la LED au démarrage
  setRGBColor(0, 0, 0);
  
  // Initialisation de l'écran OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  
  // Attendre un peu pour que l'écran s'initialise correctement dans Wokwi
  delay(100);
  
  // Initialiser l'écran avec une vérification de réussite
  bool oledInitSuccess = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  
  if(!oledInitSuccess) {
    Serial.println(F("Échec d'initialisation de l'écran SSD1306"));
    // Dans Wokwi, continuons quand même pour le débogage
    delay(1000);
  } else {
    Serial.println(F("OLED initialisé avec succès"));
  }
  
  // Effacer l'écran et configurer l'affichage
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("ESP32 Fronius Monitor");
  display.println("Mode SIMULATION");
  display.display();
  
  Serial.println("Affichage du texte initial sur l'OLED");
  delay(2000);
  
  // Première récupération des données simulées
  getSimulatedData();
  updateDisplay();
  updateRGBLed();
  
  // Afficher les données initiales dans la console
  Serial.println("\n--- Données initiales ---");
  Serial.print("Production: "); Serial.print(productionPower); Serial.println(" W");
  Serial.print("Consommation: "); Serial.print(consumptionPower); Serial.println(" W");
  Serial.print("Réseau: "); Serial.print(gridPower); Serial.println(" W");
  Serial.print("Batterie: "); Serial.print(batteryPower); Serial.println(" W");
  Serial.print("SOC Batterie: "); Serial.print(batterySOC); Serial.println(" %");
}

void loop() {
  unsigned long currentTime = millis();
  
  // Vérifier si c'est le moment de mettre à jour les données
  if (currentTime - lastUpdateTime >= UPDATE_INTERVAL) {
    // Forcer le changement de scénario pour la démonstration
    simulationStep = (simulationStep + 1) % 5;
    
    getSimulatedData();
    updateDisplay();
    updateRGBLed();
    lastUpdateTime = currentTime;
    
    // Afficher les données simulées dans la console série
    Serial.println("\n--- Données simulées ---");
    Serial.print("Scénario: "); Serial.println(simulationStep);
    Serial.print("Production: "); Serial.print(productionPower); Serial.println(" W");
    Serial.print("Consommation: "); Serial.print(consumptionPower); Serial.println(" W");
    Serial.print("Réseau: "); Serial.print(gridPower); Serial.println(" W");
    Serial.print("Batterie: "); Serial.print(batteryPower); Serial.println(" W");
    Serial.print("SOC Batterie: "); Serial.print(batterySOC); Serial.println(" %");
  }
  
  // Petit délai pour éviter de surcharger le processeur
  delay(50);
}

// Fonction pour obtenir des données simulées qui changent avec le temps
void getSimulatedData() {
  // Dans cette version, nous ne changeons pas automatiquement le scénario
  // car nous le forçons dans la boucle principale pour la démonstration
  lastSimulationUpdate = millis();
  
  // Simuler différents scénarios
  switch (simulationStep) {
    case 0: // Production élevée, consommation faible
      productionPower = 4500.0;
      consumptionPower = 1200.0;
      gridPower = -3300.0; // Export vers le réseau
      batteryPower = 0.0;
      batterySOC = 100.0;
      break;
    case 1: // Production moyenne, consommation moyenne
      productionPower = 2800.0;
      consumptionPower = 2500.0;
      gridPower = -300.0; // Export léger
      batteryPower = 0.0;
      batterySOC = 95.0;
      break;
    case 2: // Production faible, consommation élevée
      productionPower = 800.0;
      consumptionPower = 3200.0;
      gridPower = 2400.0; // Import du réseau
      batteryPower = 0.0;
      batterySOC = 90.0;
      break;
    case 3: // Nuit, consommation depuis batterie
      productionPower = 0.0;
      consumptionPower = 500.0;
      gridPower = 0.0;
      batteryPower = -500.0; // Décharge
      batterySOC = 85.0;
      break;
    case 4: // Aube, recharge batterie
      productionPower = 1200.0;
      consumptionPower = 300.0;
      gridPower = -400.0;
      batteryPower = 500.0; // Charge
      batterySOC = 80.0;
      break;
  }
}

// Fonction pour mettre à jour l'affichage OLED
void updateDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);
  
  // Titre
  display.setTextSize(1);
  display.println("Fronius Monitor");
  display.drawLine(0, 9, display.width(), 9, SSD1306_WHITE);
  
  // Données
  display.setCursor(0, 12);
  display.print("Production: ");
  display.print(productionPower, 0);
  display.println(" W");
  
  display.print("Consommation: ");
  display.print(consumptionPower, 0);
  display.println(" W");
  
  display.print("Reseau: ");
  if (gridPower >= 0) {
    display.print("Import ");
    display.print(gridPower, 0);
  } else {
    display.print("Export ");
    display.print(-gridPower, 0);
  }
  display.println(" W");
  
  // Afficher les données de la batterie si présente
  if (batterySOC > 0) {
    display.print("Batterie: ");
    display.print(batterySOC, 0);
    display.print("% ");
    
    if (batteryPower > 0) {
      display.println("Charge");
    } else if (batteryPower < 0) {
      display.println("Decharge");
    } else {
      display.println("Idle");
    }
  }
  
  // Afficher l'heure de la dernière mise à jour
  display.setCursor(0, 56);
  display.print("MAJ: ");
  
  // Convertir le temps en heures:minutes:secondes
  unsigned long uptime = millis() / 1000;
  int hours = uptime / 3600;
  int mins = (uptime % 3600) / 60;
  int secs = uptime % 60;
  
  if (hours < 10) display.print("0");
  display.print(hours);
  display.print(":");
  if (mins < 10) display.print("0");
  display.print(mins);
  display.print(":");
  if (secs < 10) display.print("0");
  display.print(secs);
  
  display.display();
}

// Fonction pour mettre à jour la LED RGB en fonction des données
void updateRGBLed() {
  int r = 0, g = 0, b = 0;
  
  // Logique pour déterminer la couleur en fonction de la production et de la consommation
  if (productionPower > consumptionPower) {
    // Production > Consommation = Vert (plus ou moins intense selon la production)
    if (productionPower < 1000) {
      g = 64;  // Vert faible
    } else if (productionPower < 3000) {
      g = 150; // Vert moyen
    } else {
      g = 255; // Vert intense
    }
  } else if (consumptionPower > productionPower) {
    // Consommation > Production = Rouge (plus ou moins intense selon la différence)
    float ratio = (consumptionPower - productionPower) / consumptionPower;
    if (consumptionPower > 3000) {
      r = 255; // Rouge intense
    } else {
      r = 100 + (int)(155 * ratio); // Rouge variable
    }
  } else {
    // Production = Consommation = Bleu
    b = 255;
  }
  
  // Appliquer la couleur à la LED
  setRGBColor(r, g, b);
}

// Fonction pour définir la couleur de la LED RGB
void setRGBColor(int red, int green, int blue) {
  // Inverser les valeurs si LED à anode commune
  if (COMMON_ANODE) {
    red = 255 - red;
    green = 255 - green;
    blue = 255 - blue;
  }
  
  // Écrire les valeurs PWM sur les broches
  analogWrite(RGB_RED_PIN, red);
  analogWrite(RGB_GREEN_PIN, green);
  analogWrite(RGB_BLUE_PIN, blue);
  
  // Afficher les valeurs RGB dans la console pour débogage
  Serial.print("LED RGB: R=");
  Serial.print(red);
  Serial.print(", G=");
  Serial.print(green);
  Serial.print(", B=");
  Serial.println(blue);
}
