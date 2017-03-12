# domotic-temphumidity
## Description
Projet de "domotique" pour prise de température/humidité dans différentes pièces (et à l'extérieur).

## Matériel
* `WeMos D1 mini v2`
* `DHT22` (module précâblé)
* `DS18B20` (module précâblé)
* Résistances pour _pont diviseur_: 1Kohms (R1) et 220 ohms (R2)
* 2 batteries de 3.7v type `18650 lithium-ion`
* battery holder à 2 emplacements pour batteries 18650
* [ optionel: `WeMos D1 Mini Single Lithium Battery Charging And Battery Boost Shield` ]
* [ optionel (avec shield ci-dessus): `Micro 1.25mm 2-Pin Female Connector Plug` ]

## Architecture
Les mesures sont envoyées vers un _MQTT broker_ (Mosquitto) dans différents _topics_.
Les _topics_ de ce broker sont actuellement consommés par `HomeAssistant`.
Les données sont également sauvegardées dans une _TSDB_ (InfluxDB) par `HomeAssistant`.

## To do:
### Sauvegarde des mesures
La sauvegarde des données dans `InfluxDB` sera revue car `HomeAssistant` ne pousse des données que s'il y a un changement d'état; autrement dit 2 valeurs identiques consécutives ne représentent qu'un point de mesure (le 1ier) dans `InfluxDB`.
Idée à mettre en oeuvre:
* utiliser `Telegraf` pour consommer les _topics MQTT_ et pousser les mesures dans `InfluxDB`.
* désactiver le lien entre `HomeAssistant` et `InfluxDB` (config de `HomeAssistant`).

### Améliorations
#### Home Assistant
##### automate
Si le service `Updater` détecte une mise à jour disponible, une notification sera crée par le service `notify.notify`.
Si la tension de la batterie est inférieur au seuil configuré, une notification sera envoyée par le service `notifiy.pushbullet` à (par défaut) tous les _devices_ définis dans `PushBullet`.

#### WeMos D1 Mini.
##### Consommation d'énergie - deep sleep
Mettre le WeMos en _deepsleep_ entre 2 prises de mesure (attention câblage à ajouter entre `RST` et `D0` (aka `GPIO16`) pour permettre l'éveil).  Ne pas oublier d'ajouter cette ligne dans la fonction `setup`:
```
  // Connect D0 to RST to wake up
  pinMode(D0, WAKEUP_PULLUP);
```
et bien entendu dans la fonction `loop`:
```
  // convert to microseconds
  ESP.deepSleep(sleepSeconds * 1000000);
```

##### tension de la batterie
* Mettre le WeMos en _deepsleep_ plus longtemps si le seuil `X` est atteint.
* Si le seuil `Y` (`X > Y`) est atteint soit mettre le WeMos en _deepsleep_ encore lus longtemps soit stopper (si possible) le WeMos.

##### MQTT
Actuellement, chaque mesure est envoyée dans un topic dédié (ex: température du living dans `home/livingroom/temperature`) ce qui se traduit par 3 _publish_ (température, humidité et tension de la batterie).
Améliorations possibles:
* soit regrouper l'envoi de ces 3 mesures dans 1 seul _topic_ (ex: `home/livingroom`) et ensuite décomposer le message en 3 dans `HomeAssistant`.
* soit revoir la nomenclature des _topics_ en y ajoutant un niveau supplémentaire: l'_étage_ (ex: `home/groundfloor/livingroom/temperature`).  Source: http://www.hivemq.com/blog/mqtt-essentials-part-5-mqtt-topics-best-practices

## Quelques sources:
* Mesure de la tension: http://www.projetsdiy.fr/mesurer-tension-alimentation-batterie-esp8266-arduino/#.WMXEVlXhCHs
* Utilisation d'un DHT22: http://www.projetsdiy.fr/esp8266-dht22-mqtt-projet-objet-connecte/#.WMXDn1XhCHu
* Utilisation d'un DS18B20: http://www.esp8266learning.com/wemos-mini-ds18b20-temperature-sensor-example.php
* mon ami Tixu: https://github.com/tixu
