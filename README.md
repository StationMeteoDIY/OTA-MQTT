# OTA-MQTT et fonction Millis

Ce script pour ESP8266 NodeMCU est la base de notre station météo, il suffit de lui ajouter chacun des capteurs que l'on souhaite lire. 
Il permet de mettre à jour l'ESP via OTA (Over-the-air), mais également d'envoyer des données via MQTT à Domoticz.
Nous voyont également la fonction "Millis", qui contrairement à la fonction "Delay", permet de faire un timer, sans blocage du script.

C'est ici la suite du Tuto : https://youtu.be/EBX1f232K9w

# Explications
Suite du tuto précédent sur la mise en place un système de mise à jour d'un ESP8266 via OTA (Over-the-air), on ajoute ici la prise en charge du MQTT, ainsi que la fonction qui mermettra d'envoyer les données.
Nous mettons également en place dans ce script la fonction "millis", qui permet de gérer un timer sans blocage du script.

Si le but est pour nous de réaliser une station météo, Les explications données dans ce Tuto sont valables pour tout autre utilisation d'un ESP8266 avec lequel on souhaite utiliser des envois MQTT, ou créer un timer qui contrairement à la fonction "Delay", ne bloquera pas l'exécution du programme.

## Pré-requis
Il faut installer l'IDE Arduino, et y intégrer quelques configurations :

Prise en charge de la carte ESP : http://arduino.esp8266.com/stable/package_esp8266com_index.json
Installation de Python dans une version 2.7.x : https://www.python.org/downloads/

## Bibliothéques nécessaires, et versions utilisées :
  - pubsubclient v2.7.0
  - ArduinoJson v6.15.0
  - ESP8266Wifi v1.0.0
  - ArduinoOTA v1.0.0
  
  
# Tuto vidéo
Vidéo explicative sur YouTube : https://youtu.be/NpkfYsqb5gI
