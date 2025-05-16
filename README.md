# ESP32 Fronius Solar API Monitor

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-green.svg)](https://www.espressif.com/)
[![API: Fronius Solar](https://img.shields.io/badge/API-Fronius%20Solar-blue.svg)](https://www.fronius.com/)

Ce projet permet de connecter un ESP32 à l'API Fronius Solar pour récupérer les informations de production et de consommation d'énergie solaire. Ces données sont ensuite affichées sur un écran OLED et une LED RGB change de couleur en fonction des valeurs.

<p align="center">
  <img src="docs/images/placeholder_image.jpg" alt="ESP32 Fronius Monitor" width="400"/>
</p>

> **Note:** Vous pouvez remplacer l'image placeholder par une photo de votre montage final.

## Fonctionnalités

- **Mode point d'accès (AP)** au premier démarrage pour configuration WiFi via interface web
- **Découverte automatique** de l'onduleur Fronius sur le réseau
- Connexion WiFi avec l'ESP32 (paramètres sauvegardés en mémoire)
- Communication avec l'API Fronius Solar
- Affichage des données sur un écran OLED
- Indication visuelle via LED RGB
- Mise à jour périodique des données

## Matériel nécessaire

- ESP32
- Écran OLED (SSD1306 ou similaire)
- LED RGB (commune anode ou cathode)
- Câbles de connexion
- Alimentation pour l'ESP32

## Configuration

### Première utilisation

1. Téléversez le code sur votre ESP32
2. L'ESP32 démarrera en mode point d'accès avec le SSID "ESP32_Fronius_Setup"
3. Connectez-vous à ce réseau WiFi avec votre smartphone ou ordinateur
4. Accédez à l'adresse http://192.168.4.1 dans votre navigateur
5. Configurez vos paramètres WiFi via l'interface web
6. Optionnellement, spécifiez l'adresse IP de votre onduleur Fronius (si non spécifiée, elle sera découverte automatiquement)
7. Après enregistrement, l'ESP32 redémarrera et se connectera à votre réseau WiFi

### Configuration avancée

Si nécessaire, vous pouvez modifier le fichier `config.h` pour ajuster :
- Les paramètres du point d'accès (SSID, mot de passe, délai d'expiration)
- Les broches utilisées pour l'écran OLED et la LED RGB
- Les seuils pour les couleurs de la LED RGB

## Bibliothèques requises

- WiFi.h (incluse dans l'ESP32)
- HTTPClient.h (incluse dans l'ESP32)
- WebServer.h (incluse dans l'ESP32)
- DNSServer.h (incluse dans l'ESP32)
- EEPROM.h (incluse dans l'ESP32)
- ESPmDNS.h (incluse dans l'ESP32)
- ArduinoJson
- Adafruit_GFX
- Adafruit_SSD1306

## Schéma de câblage

### Écran OLED (I2C)
- VCC → 3.3V
- GND → GND
- SCL → GPIO 22
- SDA → GPIO 21

### LED RGB
- R → GPIO 25
- G → GPIO 26
- B → GPIO 27
- VCC/GND selon le type de LED (anode ou cathode commune)

## Comportement de l'ESP32

### Indications de la LED RGB
- **Bleu clignotant** : Mode point d'accès (configuration) ou reconnexion WiFi
- **Rouge** : Erreur (onduleur non trouvé ou erreur de connexion)
- **Vert** (intensité variable) : Production > Consommation
- **Rouge** (intensité variable) : Consommation > Production
- **Bleu fixe** : Production = Consommation

### Écran OLED
Affiche les informations suivantes :
- Production en watts
- Consommation en watts
- État du réseau (import/export)
- État de la batterie (si présente)
- Heure de la dernière mise à jour

## Dépannage

- Si l'ESP32 ne parvient pas à se connecter au WiFi, il redémarrera en mode point d'accès après plusieurs tentatives
- Le mode point d'accès s'éteint automatiquement après 5 minutes d'inactivité
- Pour réinitialiser la configuration, effacez l'EEPROM en téléversant un sketch vide avec `EEPROM.begin(512); EEPROM.write(0, 0); EEPROM.commit();`

## Installation

### Prérequis

- [Arduino IDE](https://www.arduino.cc/en/software) (version 1.8.x ou supérieure)
- [Support ESP32 pour Arduino](https://github.com/espressif/arduino-esp32) ([Guide d'installation](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html))

### Bibliothèques à installer

Dans l'IDE Arduino, allez dans `Outils > Gérer les bibliothèques...` et installez les bibliothèques suivantes :

- ArduinoJson (version 6.x)
- Adafruit GFX Library
- Adafruit SSD1306

### Téléversement du code

1. Clonez ce dépôt ou téléchargez-le en tant que ZIP
2. Ouvrez le fichier `ESP32_Fronius_Monitor.ino` dans l'IDE Arduino
3. Sélectionnez votre carte ESP32 dans `Outils > Type de carte`
4. Sélectionnez le bon port COM dans `Outils > Port`
5. Cliquez sur le bouton "Téléverser"

## API Fronius Solar

Ce projet utilise l'API Fronius Solar pour récupérer les données de l'onduleur. L'API est accessible via HTTP et renvoie des données au format JSON.

Point d'entrée principal utilisé :
- `/solar_api/v1/GetPowerFlowRealtimeData.fcgi` - Fournit les données en temps réel sur les flux d'énergie

Pour plus d'informations sur l'API Fronius Solar, consultez la [documentation officielle](https://www.fronius.com/en/solar-energy/installers-partners/technical-data/all-products/system-monitoring/open-interfaces/fronius-solar-api-json-).

## Contribution

Les contributions à ce projet sont les bienvenues ! N'hésitez pas à :

1. Fork le projet
2. Créer une branche pour votre fonctionnalité (`git checkout -b feature/amazing-feature`)
3. Commit vos changements (`git commit -m 'Add some amazing feature'`)
4. Push vers la branche (`git push origin feature/amazing-feature`)
5. Ouvrir une Pull Request

## Licence

Ce projet est sous licence MIT - voir le fichier [LICENSE](LICENSE) pour plus de détails.

## Remerciements

- [Fronius](https://www.fronius.com/) pour leur API Solar
- [Espressif](https://www.espressif.com/) pour l'ESP32
- [Adafruit](https://www.adafruit.com/) pour leurs bibliothèques OLED
