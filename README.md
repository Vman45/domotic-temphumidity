# domotic-temphumidity
Projet de "domotique" pour prise de température/humidité dans différentes pièces (et à l'extérieur) et envoi des mesures vers un MQTT broker (Mosquitto).
Les topics de ce broker sont actuellement consommés par HomeAssistant.
Les données sont également sauvegardées dans une TSDB (InfluxDB).  Toutefois cette partie sera revue car HomeAssistant ne pousse des données que s'il y a un changement d'état; autrement dit 2 valeurs identiques consécutives ne représente qu'un point de mesure (le 1ier) dans InfluxDB.
Idée à mettre en oeuvre: utiliser Telegraf pour consommer les topics MQTT et pousser les mesures dans InfluxDB + désactiver le lien entre HomeAssistant et InfluxDB (config de HomeAssistant).


Quelques sources:
* Mesure de la tension: http://www.projetsdiy.fr/mesurer-tension-alimentation-batterie-esp8266-arduino/#.WMXEVlXhCHs
* Utilisation d'un DHT22: http://www.projetsdiy.fr/esp8266-dht22-mqtt-projet-objet-connecte/#.WMXDn1XhCHu
* Utilisation d'un DS18B20: http://www.esp8266learning.com/wemos-mini-ds18b20-temperature-sensor-example.php
* mon ami Tixu: https://github.com/tixu
