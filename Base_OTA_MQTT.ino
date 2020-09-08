
#include "connect.h"
// const char* ssid = "_MON_SSID_";                   // SSID du réseau Wifi
// const char* password = "_MOT_DE_PASSE_WIFI_";      // Mot de passe du réseau Wifi.
// const char* mqtt_server = "_IP_DU_BROKER_";         // Adresse IP ou DNS du Broker.
// const int mqtt_port = 1883;                         // Port du Brocker MQTT
// const char* mqtt_login = "_LOGIN_";                 // Login de connexion à MQTT.
// const char* mqtt_password = "_PASSWORD_";           // Mot de passe de connexion à MQTT.
// ------------------------------------------------------------
// Variables de configuration :
const char* topicIn     = "domoticz/out";             // Nom du topic envoyé par Domoticz
const char* topicOut    = "domoticz/in";              // Nom du topic écouté par Domoticz
// ------------------------------------------------------------
// Variables et constantes utilisateur :
String nomModule  = "Station Météo";                  // Nom usuel de ce module. Sera visible uniquement dans les Log Domoticz.
unsigned long t_lastActionCapteur = 0;                // enregistre le Time de la dernière intérogation.
const long t_interoCapteur = 60000;                   // Valeur de l'intervale entre 2 relevés.


// On intégre les bibliothéques necessaire à la mise à jour via OTA.
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
// On intégre les librairies necessaires à la connexion Wifi, au MQTT, et à JSON, necessaire pour le MQTT.
#include <PubSubClient.h>
#include <ArduinoJson.h>


void setup() {
  Serial.begin(115200);                           // On initialise la vitesse de transmission de la console.
  setup_wifi();                                   // Connexion au Wifi
  initOTA();                                      // Initialisation de l'OTA
  client.setServer(mqtt_server, mqtt_port);       // On défini la connexion MQTT  
  
}

void loop() {
  ArduinoOTA.handle();                            // On verifie si une mise a jour OTA nous est envoyée. Si OUI, la lib ArduinoOTA se charge de faire la MAJ.

  // On s'assure que MQTT est bien connecté.
  if (!client.connected()) {
    // MQTT déconnecté, on reconnecte.
    Serial.println("MQTT déconnecté, on reconnecte !");
    reconnect();
  } else {
    // On vérifie que l'intervale de relevé des capteurs est atteint.
    if (currentMillis - t_lastActionCapteur >= t_interoCapteur) {
      // MQTT connecté, on exécute les traitements

      // Traitement effectué, on met à jour la valeur du dernier traitement.
      t_lastActionCapteur = currentMillis;
    }
  }
  
}



// CONNEXION WIFI
// **************
void setup_wifi() {
  // Connexion au réseau Wifi
  delay(10);
  Serial.println();
  Serial.print("Connection au réseau : ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);            // Passage de la puce en mode client
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    // Tant que l'on est pas connecté, on boucle.
    delay(500);
    Serial.print(".");
  }
  // Initialise la séquence Random
  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connecté");
  Serial.print("Addresse IP : ");
  Serial.println(WiFi.localIP());
}


// INITIALISATION Arduino OTA
// **************************
void initOTA() {
  /* Port par defaut = 8266 */
  // ArduinoOTA.setPort(8266);

  /* Hostname, par defaut = esp8266-[ChipID] */
  ArduinoOTA.setHostname("maStationMeteo");

  /* Pas d'authentication par defaut */
  // ArduinoOTA.setPassword("admin");

  /* Le mot de passe peut également être défini avec sa valeur md5 */
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");
  
  // code à exécuter au démarrage de la mise à jour
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: si vous mettez à jour SPIFFS, démonter SPIFFS à l'aide de SPIFFS.end ()
    Serial.println("Début de update " + type);
  });
  
  // code à exécuter à la fin de la mise à jour
  ArduinoOTA.onEnd([]() {
    Serial.println("\nFin");
  });
  
  // code à exécuter pendant la mise à jour
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progression : %u%%\r", (progress / (total / 100)));
  });
  
  // code à exécuter en cas d'erreur de la mise à jour
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Erreur[%u] : ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Authentification Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Exécution Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connexion Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Réception Failed");
    else if (error == OTA_END_ERROR) Serial.println("Fin Failed");
  });
  
  ArduinoOTA.begin();
  Serial.println("Prêt");
  Serial.print("Adresse IP : ");
  Serial.println(WiFi.localIP());
}


// CONNEXION MQTT
// **************
void reconnect() {
  
  // Boucle jusqu'à la connexion MQTT
  while (!client.connected()) {
    Serial.print("Tentative de connexion MQTT...");
    // Création d'un ID client aléatoire
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    
    // Tentative de connexion
    if (client.connect(clientId.c_str(), mqtt_login, mqtt_password)) {
      Serial.println("connecté");
      
      // Connexion effectuée, publication d'un message...
      String message = "Connexion MQTT de "+ nomModule + " réussi sous référence technique : " + clientId + ".";
      
      // Création de la chaine JSON6
      DynamicJsonDocument doc(256);
      // On renseigne les variables.
      doc["command"] = "addlogmessage";
      doc["message"] = message;
      // On sérialise la variable JSON
      String messageOut;
      serializeJson(doc, messageOut);
      
      // Convertion du message en Char pour envoi dans les Log Domoticz.
      char messageChar[messageOut.length()+1];
      messageOut.toCharArray(messageChar,messageOut.length()+1);
      client.publish(topicOut, messageChar);
        
      // On souscrit (écoute)
      client.subscribe("#");
    } else {
      Serial.print("Erreur, rc=");
      Serial.print(client.state());
      Serial.println(" prochaine tentative dans 2s");
      // Pause de 2 secondes
      delay(2000);
    }
  }
}


// ENVOI DES DATAS.
// ***************
void SendData (String command, int idxDevice, int nvalue, String svalue) {
  // Création de la chaine JSON6
  DynamicJsonDocument doc(256);
  // On renseigne les variables.
  doc["command"]  = command;
  doc["idx"]      = idxDevice;
  doc["nvalue"]   = nvalue;
  doc["svalue"]   = svalue;
  
  // On sérialise la variable JSON
  String messageOut;
  serializeJson(doc, messageOut);
      
  // Convertion du message en Char pour envoi dans les Log Domoticz.
  char messageChar[messageOut.length()+1];
  messageOut.toCharArray(messageChar,messageOut.length()+1);
  client.publish(topicOut, messageChar);
  // Pause de 1 secondes
  delay(1000);
  Serial.println("\t -> Message envoyé à Domoticz");
}
 