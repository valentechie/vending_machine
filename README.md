# Máquina Expendedora con Arduino
Controlador software para una máquina expendedora de café basada en **Arduino UNO**, desarrollado con los sensores y actuadores del kit de laboratorio de la asignatura. El sistema responde a eventos físicos en tiempo real e interactúa con el usuario mediante una pantalla LCD, un joystick analógico, un pulsador y dos LEDs, mientras monitoriza de forma **no bloqueante** la temperatura, la humedad y la distancia a la que se encuentra el cliente.

## Índice
 - [Hardware y conexiones](#hardware-y-conexiones)
 - [Arquitectura del software](#arquitectura-del-software)
 - [Modo Arranque](#modo-arranque)
 - [Modo Servicio](#modo-servicio)
 - [Modo Administración](#modo-administración)
 - [Pruebas de hardware](#pruebas-de-hardware)
 - [Problemas y soluciones](#problemas-y-soluciones)

## Hardware y conexiones
![Wiring_diagram](./docs/vending_machine_circuit.png)
