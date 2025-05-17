#ifndef WOKWI_SIMULATION_H
#define WOKWI_SIMULATION_H

// Activer ce flag pour utiliser la simulation Wokwi
#define WOKWI_SIMULATION 1

#if WOKWI_SIMULATION

// Exemple de réponse JSON de l'API Fronius (format simplifié)
const char* SIMULATED_FRONIUS_RESPONSE = R"(
{
  "Body": {
    "Data": {
      "Site": {
        "P_PV": 3500.5,
        "P_Load": 2100.8,
        "P_Grid": -1399.7,
        "P_Akku": 0,
        "E_Day": 15.6,
        "E_Year": 4320.5,
        "E_Total": 12500.8,
        "rel_SelfConsumption": 60.0,
        "rel_Autonomy": 100.0,
        "Mode": "normal"
      },
      "Inverters": {
        "1": {
          "P": 3500.5,
          "E_Day": 15.6,
          "E_Year": 4320.5,
          "E_Total": 12500.8
        }
      },
      "Storage": [
        {
          "Controller": {
            "StateOfCharge_Relative": 85.5
          },
          "P": -200.5,
          "SOC": 85.5
        }
      ]
    }
  },
  "Head": {
    "Status": {
      "Code": 0,
      "Reason": "",
      "UserMessage": ""
    },
    "Timestamp": "2025-05-17T09:30:00+02:00"
  }
})";

// Fonction pour simuler la découverte de l'onduleur Fronius
bool simulateDiscoveryFronius(String &froniusIP) {
  delay(2000); // Simuler un délai de recherche
  froniusIP = "192.168.1.100"; // IP simulée
  return true;
}

// Fonction pour simuler la connexion WiFi
bool simulateWiFiConnection() {
  delay(2000); // Simuler un délai de connexion
  return true;
}

// Variables pour simuler des changements de données au fil du temps
unsigned long lastSimulationUpdate = 0;
int simulationStep = 0;

// Fonction pour obtenir des données simulées qui changent avec le temps
void getSimulatedFroniusData(float &productionPower, float &consumptionPower, 
                            float &gridPower, float &batteryPower, float &batterySOC) {
  // Mettre à jour les valeurs simulées toutes les 10 secondes
  if (millis() - lastSimulationUpdate > 10000) {
    lastSimulationUpdate = millis();
    simulationStep = (simulationStep + 1) % 5;
  }
  
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

#endif // WOKWI_SIMULATION
#endif // WOKWI_SIMULATION_H
