<!-- TOC depthFrom:1 depthTo:6 withLinks:1 updateOnSave:1 orderedList:0 -->

- [domotic-temphumidity](#domotic-temphumidity)
	- [Description](#description)
	- [Matériel:](#matriel)
	- [Architecture](#architecture)
	- [TODO:](#todo)
		- [Sauvegarde des mesures](#sauvegarde-des-mesures)
		- [Améliorations:](#amliorations)
			- [Home Assistant:](#home-assistant)
				- [automate](#automate)
			- [WeMos D1 Mini:](#wemos-d1-mini)
				- [Consommation d'énergie - deep sleep](#consommation-dnergie-deep-sleep)
				- [Tension de la batterie](#tension-de-la-batterie)
				- [Ajouter Heat Index et Dew Point](#ajouter-heat-index-et-dew-point)
				- [MQTT](#mqtt)
	- [Quelques sources:](#quelques-sources)
	- [Installation stack TICK sur Raspberry Pi 3](#installation-stack-tick-sur-raspberry-pi-3)
		- [Telegraf](#telegraf)
		- [InfluxDB](#influxdb)
		- [Chonograph](#chonograph)
		- [kapacitor](#kapacitor)
	- [Installation de Grafana](#installation-de-grafana)

<!-- /TOC -->

# domotic-temphumidity
## Description
Projet de "domotique" pour prise de température/humidité dans différentes pièces (et à l'extérieur).

## Matériel:
* `WeMos D1 mini v2`
* `DHT22` (module précâblé)
* `DS18B20` (module précâblé)
* Résistances pour _pont diviseur_: `1 KOhms` (R1) et `220 Ohms` (R2)
* 2 batteries de 3.7v type `18650 lithium-ion`
* `battery holder` à 2 emplacements pour batteries 18650
* [ optionel: `WeMos D1 Mini Single Lithium Battery Charging And Battery Boost Shield` ]
* [ optionel (avec shield ci-dessus): `Micro 1.25mm 2-Pin Female Connector Plug` ]

## Architecture
Les mesures sont envoyées vers un _MQTT broker_ (`Mosquitto`) dans différents _topics_.
Les _topics_ de ce broker sont actuellement consommés par `HomeAssistant`.
Les données sont également sauvegardées dans une _TSDB_ (InfluxDB) par `HomeAssistant`.

## TODO:
### Sauvegarde des mesures
La sauvegarde des données dans `InfluxDB` sera revue car `HomeAssistant` ne pousse des données que s'il y a un changement d'état; autrement dit plusieurs valeurs identiques consécutives ne représentent qu'un point de mesure (le 1ier) dans `InfluxDB`.
Idée à mettre en oeuvre:
* utiliser `Telegraf` pour consommer les _topics MQTT_ et pousser les mesures dans `InfluxDB`.
* désactiver le lien entre `HomeAssistant` et `InfluxDB` (config de `HomeAssistant`).

### Améliorations:
#### Home Assistant:
##### automate
Si le service `Updater` détecte une mise à jour disponible, une notification sera crée par le service `notify.notify`.
Si la tension de la batterie est inférieur au seuil configuré, une notification sera envoyée par le service `notifiy.pushbullet` à (par défaut) tous les _devices_ définis dans `PushBullet`.

#### WeMos D1 Mini:
##### Consommation d'énergie - deep sleep
Mettre le WeMos en _deep sleep_ entre 2 prises de mesure.  !! Attention câblage à ajouter entre `RST` et `D0` (aka `GPIO16`) pour permettre l'éveil) !!
Ne pas oublier d'ajouter cette ligne dans la fonction `setup`:
```
  // Connect D0 to RST to wake up
  pinMode(D0, WAKEUP_PULLUP);
```
et bien entendu dans la fonction `loop`:
```
  // convert to microseconds
  ESP.deepSleep(sleepSeconds * 1000000);
```

##### Tension de la batterie
* Mettre le WeMos en _deep sleep_ plus longtemps si le seuil `X` est atteint.
* Si le seuil `Y` (`X > Y`) est atteint soit mettre le WeMos en _deep sleep_ encore lus longtemps soit stopper (si possible) le WeMos.

##### Ajouter Heat Index et Dew Point
* ajouter la lecture ou le calcul de l'index de température (température ressentie)
* ajouter la lecture ou le calcul du point de rosée ()
Voir les articles:
 - http://www.best-microcontroller-projects.com/dht22.html
 - https://github.com/adafruit/DHT-sensor-library/blob/master/examples/DHTtester/DHTtester.ino
 - https://github.com/adafruit/DHT-sensor-library/blob/master/DHT.cpp
 - https://forums.adafruit.com/viewtopic.php?f=25&t=30241#p167348

##### MQTT
Actuellement, chaque mesure est envoyée dans un topic dédié (ex: température du living dans `home/livingroom/temperature`) ce qui se traduit par 3 _publish_ (température, humidité et tension de la batterie).
Améliorations possibles:
* soit regrouper l'envoi de ces 3 mesures dans 1 seul _topic_ (ex: `home/livingroom`) et ensuite décomposer le message en 3 dans `HomeAssistant`.
* soit revoir la nomenclature des _topics_ en y ajoutant un niveau supplémentaire: l'_étage_ (ex: `home/groundfloor/livingroom/temperature`).  Source: http://www.hivemq.com/blog/mqtt-essentials-part-5-mqtt-topics-best-practices

## Quelques sources:
* mon ami Tixu: https://github.com/tixu
* Mesure de la tension: http://www.projetsdiy.fr/mesurer-tension-alimentation-batterie-esp8266-arduino/#.WMXEVlXhCHs
* Utilisation d'un DHT22: http://www.projetsdiy.fr/esp8266-dht22-mqtt-projet-objet-connecte/#.WMXDn1XhCHu
* Utilisation d'un DS18B20: http://www.esp8266learning.com/wemos-mini-ds18b20-temperature-sensor-example.php
* InfluxDB et Telegraf sur Raspberry Pi: http://padcom13.blogspot.be/2015/12/influxdb-telegraf-and-grafana-on.html
* Packages officiels TICK pour ARM:
 - https://dl.influxdata.com/telegraf/releases/telegraf_1.2.1_armhf.deb
 - https://dl.influxdata.com/influxdb/releases/influxdb_1.2.1_armhf.deb
 - https://dl.influxdata.com/chronograf/releases/chronograf_1.2.0~beta4_armhf.deb
 - https://dl.influxdata.com/kapacitor/releases/kapacitor_1.2.0_armhf.deb
* Grafana on Raspberry Pi: https://github.com/fg2it/grafana-on-raspberry

## Installation stack TICK sur Raspberry Pi 3
### Telegraf
```
wget https://dl.influxdata.com/telegraf/releases/telegraf_1.2.1_armhf.deb
sudo dpkg -i telegraf_1.2.1_armhf.deb
```
### InfluxDB
```
wget https://dl.influxdata.com/influxdb/releases/influxdb_1.2.1_armhf.deb
sudo dpkg -i influxdb_1.2.1_armhf.deb
```
### Chonograph
```
wget https://dl.influxdata.com/chronograf/releases/chronograf_1.2.0~beta4_armhf.deb
sudo dpkg -i chronograph_1.2.0~beta4_armhf.deb
```
### kapacitor
```
wget https://dl.influxdata.com/kapacitor/releases/kapacitor_1.2.0_armhf.deb
sudo dpkg -i kapacitor_1.2.0_armhf.deb
```

## Installation de Grafana
Basé sur https://github.com/fg2it/grafana-on-raspberry (releases: https://github.com/fg2it/grafana-on-raspberry/releases)
```
wget https://github.com/fg2it/grafana-on-raspberry/releases/download/v4.1.2/grafana_4.1.2-1487023783_armhf.deb
sudo dpkg -i grafana_4.1.2-1487023783_armhf.deb
sudo apt-get install -f
```
