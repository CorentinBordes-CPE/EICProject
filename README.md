# Création d'une installation domotique de gestion de température et d'humidité dans une salle de bain

## Matériel
Pour la mise en place de ce système, nous utiliserons :
	- Un capteur de température et d'humidité doté d'un système de communication BLE (capteur 'Mi Temperature and humidity' de Xiaomi, modèle LYWSDCGQ/01ZM ),
	- Microcontrolleur ESP32 Lilygo TTGO T-Display,
	- Carte STM32F746NG de STMicroelectronics,
	- Un servomoteur,
	- Un routeur / switch.

Ces équipements vous permettront, à moindre coût, de réaliser cette installation.

## Communications :

```mermaid
Communication protocol;
Sensor-->|BLE|ESP32;
ESP32-->|UART|PC;
PC-->|WIFI|switch;
switch-->|ETHERNET|STM32;
STM32-->servomotor
```
