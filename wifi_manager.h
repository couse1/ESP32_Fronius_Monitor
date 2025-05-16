#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <DNSServer.h>
#include <ESPmDNS.h>

// Taille de l'EEPROM pour stocker les configurations
#define EEPROM_SIZE 512

// Adresses EEPROM
#define EEPROM_CONFIGURED_FLAG 0
#define EEPROM_SSID_ADDR 1
#define EEPROM_SSID_LENGTH 32
#define EEPROM_PASSWORD_ADDR (EEPROM_SSID_ADDR + EEPROM_SSID_LENGTH)
#define EEPROM_PASSWORD_LENGTH 64
#define EEPROM_FRONIUS_IP_ADDR (EEPROM_PASSWORD_ADDR + EEPROM_PASSWORD_LENGTH)
#define EEPROM_FRONIUS_IP_LENGTH 16

// Serveur DNS pour le portail captif
const byte DNS_PORT = 53;
DNSServer dnsServer;

// Serveur Web sur le port 80
WebServer server(80);

// Variables pour stocker les configurations
String savedSSID = "";
String savedPassword = "";
String savedFroniusIP = "";
bool configChanged = false;

// Fonction pour vérifier si l'ESP a déjà été configuré
bool isConfigured() {
  return EEPROM.read(EEPROM_CONFIGURED_FLAG) == 1;
}

// Fonction pour sauvegarder les configurations dans l'EEPROM
void saveConfiguration(String ssid, String password, String froniusIP) {
  // Marquer comme configuré
  EEPROM.write(EEPROM_CONFIGURED_FLAG, 1);
  
  // Sauvegarder SSID
  for (int i = 0; i < EEPROM_SSID_LENGTH; i++) {
    if (i < ssid.length()) {
      EEPROM.write(EEPROM_SSID_ADDR + i, ssid[i]);
    } else {
      EEPROM.write(EEPROM_SSID_ADDR + i, 0);
    }
  }
  
  // Sauvegarder mot de passe
  for (int i = 0; i < EEPROM_PASSWORD_LENGTH; i++) {
    if (i < password.length()) {
      EEPROM.write(EEPROM_PASSWORD_ADDR + i, password[i]);
    } else {
      EEPROM.write(EEPROM_PASSWORD_ADDR + i, 0);
    }
  }
  
  // Sauvegarder IP Fronius
  for (int i = 0; i < EEPROM_FRONIUS_IP_LENGTH; i++) {
    if (i < froniusIP.length()) {
      EEPROM.write(EEPROM_FRONIUS_IP_ADDR + i, froniusIP[i]);
    } else {
      EEPROM.write(EEPROM_FRONIUS_IP_ADDR + i, 0);
    }
  }
  
  EEPROM.commit();
  
  // Mettre à jour les variables
  savedSSID = ssid;
  savedPassword = password;
  savedFroniusIP = froniusIP;
}

// Fonction pour charger les configurations depuis l'EEPROM
void loadConfiguration() {
  // Charger SSID
  savedSSID = "";
  for (int i = 0; i < EEPROM_SSID_LENGTH; i++) {
    char c = EEPROM.read(EEPROM_SSID_ADDR + i);
    if (c != 0) {
      savedSSID += c;
    } else {
      break;
    }
  }
  
  // Charger mot de passe
  savedPassword = "";
  for (int i = 0; i < EEPROM_PASSWORD_LENGTH; i++) {
    char c = EEPROM.read(EEPROM_PASSWORD_ADDR + i);
    if (c != 0) {
      savedPassword += c;
    } else {
      break;
    }
  }
  
  // Charger IP Fronius
  savedFroniusIP = "";
  for (int i = 0; i < EEPROM_FRONIUS_IP_LENGTH; i++) {
    char c = EEPROM.read(EEPROM_FRONIUS_IP_ADDR + i);
    if (c != 0) {
      savedFroniusIP += c;
    } else {
      break;
    }
  }
  
  Serial.println("Configuration chargée:");
  Serial.print("SSID: ");
  Serial.println(savedSSID);
  Serial.print("Mot de passe: ");
  Serial.println("********");
  Serial.print("IP Fronius: ");
  Serial.println(savedFroniusIP);
}

// Page HTML pour la configuration
const char* configPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 Fronius Monitor - Configuration</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 20px;
      background-color: #f0f0f0;
    }
    .container {
      max-width: 500px;
      margin: 0 auto;
      background-color: white;
      padding: 20px;
      border-radius: 10px;
      box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
    }
    h1 {
      color: #0066cc;
      text-align: center;
    }
    label {
      display: block;
      margin-top: 15px;
      font-weight: bold;
    }
    input[type="text"], input[type="password"] {
      width: 100%;
      padding: 10px;
      margin-top: 5px;
      border: 1px solid #ddd;
      border-radius: 4px;
      box-sizing: border-box;
    }
    button {
      background-color: #0066cc;
      color: white;
      border: none;
      padding: 12px 20px;
      margin-top: 20px;
      border-radius: 4px;
      cursor: pointer;
      width: 100%;
      font-size: 16px;
    }
    button:hover {
      background-color: #0052a3;
    }
    .networks {
      margin-top: 10px;
      max-height: 150px;
      overflow-y: auto;
      border: 1px solid #ddd;
      border-radius: 4px;
    }
    .network-item {
      padding: 8px;
      cursor: pointer;
      border-bottom: 1px solid #eee;
    }
    .network-item:hover {
      background-color: #f0f0f0;
    }
    .scan-btn {
      background-color: #28a745;
      margin-top: 5px;
    }
    .scan-btn:hover {
      background-color: #218838;
    }
    .status {
      margin-top: 20px;
      padding: 10px;
      border-radius: 4px;
      text-align: center;
    }
    .success {
      background-color: #d4edda;
      color: #155724;
    }
    .error {
      background-color: #f8d7da;
      color: #721c24;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>ESP32 Fronius Monitor</h1>
    <form id="config-form">
      <div>
        <label for="ssid">Réseau WiFi:</label>
        <input type="text" id="ssid" name="ssid" placeholder="Nom du réseau WiFi" required>
        <button type="button" class="scan-btn" onclick="scanNetworks()">Scanner les réseaux</button>
        <div id="networks" class="networks" style="display:none;"></div>
      </div>
      
      <div>
        <label for="password">Mot de passe WiFi:</label>
        <input type="password" id="password" name="password" placeholder="Mot de passe du réseau">
      </div>
      
      <div>
        <label for="froniusIP">IP de l'onduleur Fronius (optionnel):</label>
        <input type="text" id="froniusIP" name="froniusIP" placeholder="Laissez vide pour découverte automatique">
      </div>
      
      <button type="submit">Enregistrer et connecter</button>
    </form>
    
    <div id="status" class="status" style="display:none;"></div>
  </div>

  <script>
    // Fonction pour scanner les réseaux WiFi
    function scanNetworks() {
      document.getElementById('networks').style.display = 'block';
      document.getElementById('networks').innerHTML = 'Recherche des réseaux...';
      
      fetch('/scan')
        .then(response => response.json())
        .then(data => {
          const networksDiv = document.getElementById('networks');
          if (data.length === 0) {
            networksDiv.innerHTML = 'Aucun réseau trouvé';
            return;
          }
          
          networksDiv.innerHTML = '';
          data.forEach(network => {
            const div = document.createElement('div');
            div.className = 'network-item';
            div.textContent = `${network.ssid} (${network.rssi} dBm)`;
            div.onclick = function() {
              document.getElementById('ssid').value = network.ssid;
              networksDiv.style.display = 'none';
            };
            networksDiv.appendChild(div);
          });
        })
        .catch(error => {
          document.getElementById('networks').innerHTML = 'Erreur lors du scan: ' + error;
        });
    }
    
    // Gestionnaire de soumission du formulaire
    document.getElementById('config-form').addEventListener('submit', function(e) {
      e.preventDefault();
      
      const ssid = document.getElementById('ssid').value;
      const password = document.getElementById('password').value;
      const froniusIP = document.getElementById('froniusIP').value;
      
      const statusDiv = document.getElementById('status');
      statusDiv.className = 'status';
      statusDiv.innerHTML = 'Enregistrement de la configuration...';
      statusDiv.style.display = 'block';
      
      fetch('/save', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
        },
        body: `ssid=${encodeURIComponent(ssid)}&password=${encodeURIComponent(password)}&froniusIP=${encodeURIComponent(froniusIP)}`
      })
      .then(response => response.json())
      .then(data => {
        if (data.success) {
          statusDiv.className = 'status success';
          statusDiv.innerHTML = 'Configuration enregistrée! Redémarrage en cours...';
          setTimeout(() => {
            window.location.href = '/';
          }, 5000);
        } else {
          statusDiv.className = 'status error';
          statusDiv.innerHTML = 'Erreur: ' + data.message;
        }
      })
      .catch(error => {
        statusDiv.className = 'status error';
        statusDiv.innerHTML = 'Erreur de connexion: ' + error;
      });
    });
  </script>
</body>
</html>
)rawliteral";

// Gestionnaire pour la page d'accueil
void handleRoot() {
  server.send(200, "text/html", configPage);
}

// Gestionnaire pour le scan des réseaux WiFi
void handleScan() {
  String json = "[";
  int n = WiFi.scanNetworks();
  
  for (int i = 0; i < n; ++i) {
    if (i > 0) json += ",";
    json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
  }
  
  json += "]";
  server.send(200, "application/json", json);
}

// Gestionnaire pour sauvegarder la configuration
void handleSave() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");
  String froniusIP = server.arg("froniusIP");
  
  if (ssid.length() == 0) {
    server.send(200, "application/json", "{\"success\":false,\"message\":\"SSID est requis\"}");
    return;
  }
  
  saveConfiguration(ssid, password, froniusIP);
  configChanged = true;
  
  server.send(200, "application/json", "{\"success\":true}");
}

// Fonction pour démarrer le mode point d'accès
void startAPMode() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  
  // Configuration du serveur DNS pour le portail captif
  dnsServer.start(DNS_PORT, "*", myIP);
  
  // Configuration des routes du serveur web
  server.on("/", handleRoot);
  server.on("/scan", handleScan);
  server.on("/save", HTTP_POST, handleSave);
  
  // Gestionnaire pour toutes les autres routes (portail captif)
  server.onNotFound([]() {
    server.sendHeader("Location", "http://" + WiFi.softAPIP().toString(), true);
    server.send(302, "text/plain", "");
  });
  
  server.begin();
  Serial.println("Serveur Web démarré");
}

// Fonction pour se connecter au WiFi avec les paramètres sauvegardés
bool connectToWiFi() {
  if (savedSSID.length() == 0) {
    Serial.println("SSID non configuré");
    return false;
  }
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(savedSSID.c_str(), savedPassword.c_str());
  
  Serial.print("Connexion à ");
  Serial.println(savedSSID);
  
  // Attendre la connexion
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 20000) {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nÉchec de connexion au WiFi");
    return false;
  }
  
  Serial.println("\nConnecté au WiFi");
  Serial.print("Adresse IP: ");
  Serial.println(WiFi.localIP());
  
  return true;
}

// Fonction pour découvrir automatiquement l'onduleur Fronius sur le réseau
bool discoverFroniusInverter(String &froniusIP) {
  if (savedFroniusIP.length() > 0) {
    froniusIP = savedFroniusIP;
    Serial.print("Utilisation de l'IP Fronius configurée: ");
    Serial.println(froniusIP);
    return true;
  }
  
  Serial.println("Recherche de l'onduleur Fronius sur le réseau...");
  
  // Obtenir l'adresse IP et le masque de sous-réseau
  IPAddress localIP = WiFi.localIP();
  IPAddress subnetMask = WiFi.subnetMask();
  
  // Calculer l'adresse du réseau
  IPAddress networkAddress;
  for (int i = 0; i < 4; i++) {
    networkAddress[i] = localIP[i] & subnetMask[i];
  }
  
  // Calculer le nombre d'hôtes possibles
  int maxHosts = 1;
  for (int i = 0; i < 4; i++) {
    maxHosts *= (255 - subnetMask[i] + 1);
  }
  maxHosts = min(maxHosts, 255); // Limiter à 255 hôtes pour éviter un scan trop long
  
  // Scanner le réseau
  WiFiClient client;
  HTTPClient http;
  
  unsigned long startTime = millis();
  
  for (int i = 1; i < maxHosts && (millis() - startTime < FRONIUS_DISCOVERY_TIMEOUT); i++) {
    IPAddress targetIP = networkAddress;
    
    // Calculer l'adresse IP cible
    int temp = i;
    for (int j = 3; j >= 0; j--) {
      int val = temp % 256;
      temp /= 256;
      targetIP[j] = (targetIP[j] & subnetMask[j]) | (val & ~subnetMask[j]);
    }
    
    // Ignorer notre propre IP
    if (targetIP == localIP) continue;
    
    String url = "http://" + targetIP.toString() + ":" + String(FRONIUS_PORT) + "/solar_api/GetAPIVersion.cgi";
    
    Serial.print("Test de ");
    Serial.print(targetIP.toString());
    Serial.print("...");
    
    http.begin(client, url);
    http.setTimeout(500);
    
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      
      // Vérifier si c'est bien un onduleur Fronius
      if (payload.indexOf("Fronius") >= 0 || payload.indexOf("APIVersion") >= 0) {
        Serial.println(" Trouvé!");
        froniusIP = targetIP.toString();
        http.end();
        return true;
      }
    }
    
    Serial.println(" Non");
    http.end();
  }
  
  Serial.println("Aucun onduleur Fronius trouvé sur le réseau");
  return false;
}

#endif // WIFI_MANAGER_H
