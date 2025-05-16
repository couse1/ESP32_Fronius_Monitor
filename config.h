// Configuration pour le projet ESP32 Fronius Monitor

// Configuration WiFi
// Ces valeurs seront remplacées par celles stockées en EEPROM après configuration
#define WIFI_SSID ""
#define WIFI_PASSWORD ""

// Configuration du point d'accès (AP) pour la configuration initiale
#define AP_SSID "ESP32_Fronius_Setup"
#define AP_PASSWORD "froniussetup"  // Laissez vide pour un AP ouvert ou définissez un mot de passe
#define AP_TIMEOUT 300000            // Délai d'expiration du mode AP en millisecondes (5 minutes)

// Configuration Fronius
#define FRONIUS_PORT 80              // Port de l'API Fronius (généralement 80)
#define FRONIUS_DISCOVERY_TIMEOUT 10000 // Délai d'attente pour la découverte de l'onduleur en ms
#define UPDATE_INTERVAL 30000        // Intervalle de mise à jour en millisecondes (30 secondes)

// Configuration des broches
// OLED Display (I2C)
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RESET -1  // Reset partagé avec l'Arduino (utiliser -1 si partage avec RESET Arduino)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C  // Adresse I2C de l'écran OLED (généralement 0x3C ou 0x3D)

// LED RGB
#define RGB_RED_PIN 25
#define RGB_GREEN_PIN 26
#define RGB_BLUE_PIN 27
#define COMMON_ANODE true  // Mettre à true pour LED RGB à anode commune, false pour cathode commune

// Seuils pour les couleurs de la LED RGB (en watts)
#define PRODUCTION_LOW 500
#define PRODUCTION_MEDIUM 2000
#define CONSUMPTION_HIGH 3000
