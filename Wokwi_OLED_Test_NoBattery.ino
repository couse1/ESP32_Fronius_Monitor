#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Configuration de l'écran OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

// Configuration des broches
#define OLED_SDA 21
#define OLED_SCL 22

// Configuration LED RGB
#define RGB_RED_PIN 25
#define RGB_GREEN_PIN 26
#define RGB_BLUE_PIN 27

// Déclaration de l'écran OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Variables pour stocker les données simulées
float productionPower = 3500.0;
float consumptionPower = 1200.0;
float gridPower = -2300.0;

// Variables pour la simulation
unsigned long lastUpdateTime = 0;
int simulationStep = 0;
const unsigned long UPDATE_INTERVAL = 3000; // 3 secondes

void setup() {
  Serial.begin(115200);
  Serial.println("Test OLED pour Wokwi - Sans Batterie");
  
  // Configuration des broches pour la LED RGB
  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_GREEN_PIN, OUTPUT);
  pinMode(RGB_BLUE_PIN, OUTPUT);
  
  // Allumer la LED en bleu pendant l'initialisation
  analogWrite(RGB_RED_PIN, 0);
  analogWrite(RGB_GREEN_PIN, 0);
  analogWrite(RGB_BLUE_PIN, 255);
  
  // Initialisation de l'I2C
  Wire.begin(OLED_SDA, OLED_SCL);
  
  // Initialisation de l'écran OLED
  Serial.println("Initialisation de l'écran OLED...");
  
  // Attendre un peu avant d'initialiser l'écran
  delay(100);
  
  // Initialisation de l'écran avec vérification
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("Échec d'initialisation de l'écran SSD1306");
  } else {
    Serial.println("Écran OLED initialisé avec succès");
  }
  
  // Effacer l'écran
  display.clearDisplay();
  
  // Configurer le texte
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Afficher un texte de test
  display.setCursor(0, 0);
  display.println("Test OLED");
  display.println("Sans Batterie");
  display.println("----------------");
  display.println("Si vous voyez ce");
  display.println("texte, l'ecran");
  display.println("fonctionne!");
  
  // Afficher le contenu
  display.display();
  Serial.println("Texte de test affiché sur l'écran");
  
  // Changer la couleur de la LED en vert pour indiquer que l'initialisation est terminée
  analogWrite(RGB_RED_PIN, 0);
  analogWrite(RGB_GREEN_PIN, 255);
  analogWrite(RGB_BLUE_PIN, 0);
  
  delay(2000);
}

void loop() {
  unsigned long currentTime = millis();
  
  // Mettre à jour l'affichage toutes les UPDATE_INTERVAL millisecondes
  if (currentTime - lastUpdateTime >= UPDATE_INTERVAL) {
    lastUpdateTime = currentTime;
    
    // Changer le scénario
    simulationStep = (simulationStep + 1) % 3;
    
    // Mettre à jour les données simulées
    switch (simulationStep) {
      case 0:
        productionPower = 4500.0;
        consumptionPower = 1200.0;
        gridPower = -3300.0;
        // LED verte (production > consommation)
        analogWrite(RGB_RED_PIN, 0);
        analogWrite(RGB_GREEN_PIN, 255);
        analogWrite(RGB_BLUE_PIN, 0);
        break;
      case 1:
        productionPower = 2000.0;
        consumptionPower = 2000.0;
        gridPower = 0.0;
        // LED bleue (production = consommation)
        analogWrite(RGB_RED_PIN, 0);
        analogWrite(RGB_GREEN_PIN, 0);
        analogWrite(RGB_BLUE_PIN, 255);
        break;
      case 2:
        productionPower = 800.0;
        consumptionPower = 3000.0;
        gridPower = 2200.0;
        // LED rouge (consommation > production)
        analogWrite(RGB_RED_PIN, 255);
        analogWrite(RGB_GREEN_PIN, 0);
        analogWrite(RGB_BLUE_PIN, 0);
        break;
    }
    
    // Mettre à jour l'affichage
    updateDisplay();
    
    // Afficher les données dans le moniteur série
    Serial.println("\n--- Données simulées ---");
    Serial.print("Scénario: "); Serial.println(simulationStep);
    Serial.print("Production: "); Serial.print(productionPower); Serial.println(" W");
    Serial.print("Consommation: "); Serial.print(consumptionPower); Serial.println(" W");
    Serial.print("Réseau: "); Serial.print(gridPower); Serial.println(" W");
  }
  
  delay(50);
}

void updateDisplay() {
  // Effacer l'écran
  display.clearDisplay();
  
  // Titre centré avec texte plus grand
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds("FRONIUS", 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 0);
  display.println("FRONIUS");
  
  // Ligne de séparation
  display.drawLine(0, 16, display.width(), 16, SSD1306_WHITE);
  
  // Affichage des valeurs principales avec des icônes et texte plus grand
  display.setTextSize(1);
  
  // Production avec icône soleil
  display.setCursor(0, 19);
  display.print("PROD ");
  // Dessiner un petit soleil
  display.drawCircle(40, 22, 3, SSD1306_WHITE);
  display.drawLine(40, 18, 40, 16, SSD1306_WHITE); // Rayon haut
  display.drawLine(40, 26, 40, 28, SSD1306_WHITE); // Rayon bas
  display.drawLine(36, 22, 34, 22, SSD1306_WHITE); // Rayon gauche
  display.drawLine(44, 22, 46, 22, SSD1306_WHITE); // Rayon droite
  
  // Valeur de production en grand
  display.setTextSize(2);
  display.setCursor(50, 19);
  display.print(productionPower, 0);
  display.setTextSize(1);
  display.setCursor(110, 26);
  display.print("W");
  
  // Consommation avec icône maison
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.print("CONSO ");
  // Dessiner une petite maison
  display.drawLine(37, 35, 43, 30, SSD1306_WHITE); // Toit gauche
  display.drawLine(43, 30, 49, 35, SSD1306_WHITE); // Toit droit
  display.drawRect(39, 35, 8, 6, SSD1306_WHITE);   // Corps maison
  
  // Valeur de consommation en grand
  display.setTextSize(2);
  display.setCursor(50, 35);
  display.print(consumptionPower, 0);
  display.setTextSize(1);
  display.setCursor(110, 42);
  display.print("W");
  
  // Information réseau en bas de l'écran (sans batterie)
  display.setCursor(0, 53);
  display.print("RESEAU: ");
  if (gridPower >= 0) {
    display.print("Import ");
    display.print(gridPower, 0);
  } else {
    display.print("Export ");
    display.print(-gridPower, 0);
  }
  display.print("W");
  
  // Afficher le contenu
  display.display();
  
  Serial.println("Affichage mis à jour (sans batterie)");
}
