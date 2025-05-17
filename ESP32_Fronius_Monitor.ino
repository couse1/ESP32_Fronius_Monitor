/*
 * ESP32 Fronius Solar API Monitor
 * 
 * Ce programme permet de récupérer les données de production et consommation
 * depuis un onduleur Fronius via son API, puis affiche ces informations
 * sur un écran OLED et fait changer la couleur d'une LED RGB.
 * 
 * Fonctionnalités:
 * - Mode point d'accès (AP) au premier démarrage pour configuration WiFi
 * - Découverte automatique de l'onduleur Fronius sur le réseau
 * - Affichage des données sur un écran OLED
 * - Indication visuelle via LED RGB
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include "config.h"
#include "wifi_manager.h"
#include "wokwi-simulation.h"

// Déclaration de l'écran OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Variables pour stocker les données
float productionPower = 0;    // Production en watts
float consumptionPower = 0;   // Consommation en watts
float gridPower = 0;          // Puissance du réseau (+ = import, - = export)
float batteryPower = 0;       // Puissance de la batterie (si présente)
float batterySOC = 0;         // État de charge de la batterie (si présente)

// Timestamp de la dernière mise à jour
unsigned long lastUpdateTime = 0;

// Variables pour le mode AP et la configuration
unsigned long apStartTime = 0;
bool apMode = false;
String froniusIPAddress = "";

void setup() {
  // Initialisation du port série
  Serial.begin(115200);
  Serial.println("ESP32 Fronius Solar API Monitor");
  
  // Configuration des broches pour la LED RGB
  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_GREEN_PIN, OUTPUT);
  pinMode(RGB_BLUE_PIN, OUTPUT);
  
  // Éteindre la LED au démarrage
  setRGBColor(0, 0, 0);
  
  // Initialisation de l'écran OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("Échec d'initialisation de l'écran SSD1306"));
    for(;;); // Ne pas continuer si l'écran n'est pas initialisé
  }
  
#if WOKWI_SIMULATION
  // Mode simulation Wokwi
  Serial.println("Mode simulation Wokwi activé");
  
  // Effacer l'écran et configurer l'affichage
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("ESP32 Fronius Monitor");
  display.println("Mode SIMULATION");
  display.display();
  delay(2000);
  
  // Simuler la connexion WiFi
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Simulation WiFi...");
  display.display();
  delay(1000);
  
  // Simuler la découverte de l'onduleur
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Simulation recherche");
  display.println("onduleur Fronius...");
  display.display();
  simulateDiscoveryFronius(froniusIPAddress);
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Onduleur simulé:");
  display.println(froniusIPAddress);
  display.println("\nRécupération données...");
  display.display();
  delay(1000);
  
  // Première récupération des données simulées
  getSimulatedFroniusData(productionPower, consumptionPower, gridPower, batteryPower, batterySOC);
  updateDisplay();
  updateRGBLed();
  
#else
  // Mode normal (non-simulation)
  // Initialisation de l'EEPROM
  EEPROM.begin(EEPROM_SIZE);
  
  // Effacer l'écran et configurer l'affichage
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("ESP32 Fronius Monitor");
  display.display();
  
  // Vérifier si l'ESP a déjà été configuré
  if (!isConfigured()) {
    // Premier démarrage, lancer le mode AP pour configuration
    Serial.println("Premier démarrage. Lancement du mode AP...");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Premier démarrage");
    display.println("Mode configuration");
    display.println("\nConnectez-vous au WiFi:");
    display.println(AP_SSID);
    display.println("\nPuis visitez:");
    display.println("http://192.168.4.1");
    display.display();
    
    startAPMode();
    apMode = true;
    apStartTime = millis();
    
    // LED bleue clignotante pour indiquer le mode AP
    setRGBColor(0, 0, 255);
  } else {
    // Charger la configuration existante
    loadConfiguration();
    
    // Connexion au WiFi
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Connexion WiFi...");
    display.println(savedSSID);
    display.display();
    
    if (connectToWiFi()) {
      // Découverte de l'onduleur Fronius
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Recherche onduleur");
      display.println("Fronius...");
      display.display();
      
      if (discoverFroniusInverter(froniusIPAddress)) {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Onduleur trouvé!");
        display.println(froniusIPAddress);
        display.println("\nRécupération données...");
        display.display();
        
        // Première récupération des données
        getFroniusData();
      } else {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Onduleur non trouvé");
        display.println("Vérifiez connexion");
        display.display();
        
        // LED rouge pour indiquer l'erreur
        setRGBColor(255, 0, 0);
      }
    } else {
      // Échec de connexion WiFi, lancer le mode AP
      Serial.println("Échec de connexion WiFi. Lancement du mode AP...");
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Échec connexion WiFi");
      display.println("Mode configuration");
      display.println("\nConnectez-vous au WiFi:");
      display.println(AP_SSID);
      display.println("\nPuis visitez:");
      display.println("http://192.168.4.1");
      display.display();
      
      startAPMode();
      apMode = true;
      apStartTime = millis();
      
      // LED bleue clignotante pour indiquer le mode AP
      setRGBColor(0, 0, 255);
    }
  }
#endif
}

void loop() {
  unsigned long currentTime = millis();
  
#if WOKWI_SIMULATION
  // Mode simulation Wokwi
  // Vérifier si c'est le moment de mettre à jour les données
  if (currentTime - lastUpdateTime >= UPDATE_INTERVAL || lastUpdateTime == 0) {
    // Obtenir les données simulées
    getSimulatedFroniusData(productionPower, consumptionPower, gridPower, batteryPower, batterySOC);
    updateDisplay();
    updateRGBLed();
    lastUpdateTime = currentTime;
    
    // Afficher les données simulées dans la console série
    Serial.println("\n--- Données simulées ---");
    Serial.print("Production: "); Serial.print(productionPower); Serial.println(" W");
    Serial.print("Consommation: "); Serial.print(consumptionPower); Serial.println(" W");
    Serial.print("Réseau: "); Serial.print(gridPower); Serial.println(" W");
    Serial.print("Batterie: "); Serial.print(batteryPower); Serial.println(" W");
    Serial.print("SOC Batterie: "); Serial.print(batterySOC); Serial.println(" %");
  }
#else
  // Mode normal
  // Mode Point d'Accès (AP)
  if (apMode) {
    // Gérer le serveur DNS et Web
    dnsServer.processNextRequest();
    server.handleClient();
    
    // Faire clignoter la LED en bleu pour indiquer le mode AP
    if ((currentTime / 500) % 2 == 0) {
      setRGBColor(0, 0, 255);
    } else {
      setRGBColor(0, 0, 0);
    }
    
    // Vérifier si la configuration a changé
    if (configChanged) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Configuration");
      display.println("enregistrée!");
      display.println("\nRedémarrage...");
      display.display();
      
      delay(3000);
      ESP.restart();
    }
    
    // Vérifier si le délai d'expiration du mode AP est atteint
    if (currentTime - apStartTime >= AP_TIMEOUT) {
      Serial.println("Délai d'expiration du mode AP atteint. Redémarrage...");
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Délai d'attente");
      display.println("dépassé");
      display.println("\nRedémarrage...");
      display.display();
      
      delay(3000);
      ESP.restart();
    }
  }
  // Mode normal - Récupération des données Fronius
  else {
    // Vérifier si c'est le moment de mettre à jour les données
    if (currentTime - lastUpdateTime >= UPDATE_INTERVAL || lastUpdateTime == 0) {
      if (WiFi.status() == WL_CONNECTED) {
        getFroniusData();
        updateDisplay();
        updateRGBLed();
        lastUpdateTime = currentTime;
      } else {
        // Reconnexion WiFi si déconnecté
        Serial.println("WiFi déconnecté. Tentative de reconnexion...");
        WiFi.begin(savedSSID.c_str(), savedPassword.c_str());
        
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("WiFi déconnecté!");
        display.println("Reconnexion...");
        display.display();
        
        // LED en bleu clignotant pour indiquer la reconnexion
        setRGBColor(0, 0, 255);
        delay(500);
        setRGBColor(0, 0, 0);
      }
    }
  }
#endif
  
  // Petit délai pour éviter de surcharger le processeur
  delay(50);
}

// Fonction pour récupérer les données de l'API Fronius
void getFroniusData() {
#if WOKWI_SIMULATION
  // En mode simulation, utiliser les données simulées
  // Cette fonction est appelée par la boucle principale, mais dans le mode simulation,
  // les données sont déjà mises à jour par getSimulatedFroniusData()
  return;
#else
  HTTPClient http;
  
  // URL de l'API Fronius pour les données de puissance
  String url = "http://" + froniusIPAddress + "/solar_api/v1/GetPowerFlowRealtimeData.fcgi";
  
  Serial.print("Connexion à l'API Fronius: ");
  Serial.println(url);
  
  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      
      // Allouer un DynamicJsonDocument suffisamment grand
      // La taille dépend de la réponse de l'API Fronius
      DynamicJsonDocument doc(8192);
      
      DeserializationError error = deserializeJson(doc, payload);
      
      if (error) {
        Serial.print("Erreur de parsing JSON: ");
        Serial.println(error.c_str());
      } else {
        // Extraction des données du JSON
        // La structure peut varier selon la configuration de votre système Fronius
        
        // Vérifier si les données sont présentes
        if (doc["Body"]["Data"]["Site"].containsKey("P_PV")) {
          productionPower = doc["Body"]["Data"]["Site"]["P_PV"];
          Serial.print("Production: ");
          Serial.print(productionPower);
          Serial.println(" W");
        }
        
        if (doc["Body"]["Data"]["Site"].containsKey("P_Load")) {
          consumptionPower = doc["Body"]["Data"]["Site"]["P_Load"];
          Serial.print("Consommation: ");
          Serial.print(consumptionPower);
          Serial.println(" W");
        }
        
        if (doc["Body"]["Data"]["Site"].containsKey("P_Grid")) {
          gridPower = doc["Body"]["Data"]["Site"]["P_Grid"];
          Serial.print("Réseau: ");
          Serial.print(gridPower);
          Serial.println(" W");
        }
        
        // Si vous avez une batterie
        if (doc["Body"]["Data"].containsKey("Storage")) {
          batteryPower = doc["Body"]["Data"]["Storage"][0]["P"];
          batterySOC = doc["Body"]["Data"]["Storage"][0]["SOC"];
          
          Serial.print("Batterie: ");
          Serial.print(batteryPower);
          Serial.print(" W, SOC: ");
          Serial.print(batterySOC);
          Serial.println("%");
        }
      }
    } else {
      Serial.print("Erreur HTTP: ");
      Serial.println(httpCode);
    }
  } else {
    Serial.println("Échec de connexion à l'API Fronius");
  }
  
  http.end();
#endif
}

// Fonction pour mettre à jour l'affichage OLED
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
  
  // Afficher les données de batterie si l'option est activée
  if (showBattery) {
    // Barre de progression pour la batterie
    display.drawRect(0, 53, 128, 11, SSD1306_WHITE);
    int barWidth = (int)(batterySOC / 100.0 * 124);
    display.fillRect(2, 55, barWidth, 7, SSD1306_WHITE);
    
    // Texte pour le réseau et la batterie
    display.setCursor(2, 55);
    if (gridPower >= 0) {
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Texte inversé sur la barre
      display.print("IMP ");
      display.print(gridPower, 0);
      display.print("W");
    } else {
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Texte inversé sur la barre
      display.print("EXP ");
      display.print(-gridPower, 0);
      display.print("W");
    }
    
    // Pourcentage de batterie à droite
    display.setCursor(90, 55);
    display.print("BAT ");
    display.print(batterySOC, 0);
    display.print("%");
    
    // Indiquer l'état de charge/décharge si applicable
    if (batteryPower != 0) {
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(110, 45);
      if (batteryPower > 0) {
        display.print("CHG");
      } else {
        display.print("DCH");
      }
    }
  } else {
    // Si les données de batterie sont désactivées, afficher uniquement l'information réseau
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
  }
  
  // Afficher le contenu
  display.display();
}

// Fonction pour mettre à jour la LED RGB en fonction des données
void updateRGBLed() {
  int r = 0, g = 0, b = 0;
  
  // Logique pour déterminer la couleur en fonction de la production et de la consommation
  if (productionPower > consumptionPower) {
    // Production > Consommation = Vert (plus ou moins intense selon la production)
    if (productionPower < PRODUCTION_LOW) {
      g = 64;  // Vert faible
    } else if (productionPower < PRODUCTION_MEDIUM) {
      g = 150; // Vert moyen
    } else {
      g = 255; // Vert intense
    }
  } else if (consumptionPower > productionPower) {
    // Consommation > Production = Rouge (plus ou moins intense selon la différence)
    float ratio = (consumptionPower - productionPower) / consumptionPower;
    if (consumptionPower > CONSUMPTION_HIGH) {
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
}
