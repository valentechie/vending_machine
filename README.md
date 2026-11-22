# Máquina Expendedora con Arduino
Controlador software para una máquina expendedora de café basada en **Arduino UNO**, desarrollado con los sensores y actuadores del kit de laboratorio de la asignatura. El sistema responde a eventos físicos en tiempo real e interactúa con el usuario mediante una pantalla LCD, un joystick analógico, un pulsador y dos LEDs, mientras monitoriza de forma **no bloqueante** la temperatura, la humedad y la distancia a la que se encuentra el cliente.

## Índice
- [Hardware y conexiones](#hardware-y-conexiones)
- [Estructura del código](#estructura-del-código)
- [Arquitectura del software](#arquitectura-del-software)
- [Modo Arranque](#modo-arranque)
- [Modo Servicio](#modo-servicio)
- [Modo Administración](#modo-administración)
- [Problemas y soluciones](#problemas-y-soluciones)

## Hardware y conexiones

### Tabla de pines
| Componente | Pin | | Componente | Pin |
|---|---|---|---|---|
| Botón externo | 2 | | Ultrasonidos (Trig) | 8 |
| LCD (pin E) | 3 | | LED2 | 10 |
| Joystick (botón SW) | 4 | | LED1 | 11 |
| LCD (pin RS) | 5 | | Joystick (eje VRy) | A0 |
| DHT11 (señal) | 6 | | Joystick (eje VRx) | A1 |
| Ultrasonidos (Echo) | 7 | | LCD (pines D4 a D7) | A2 a A5 |

**Otros detalles del montaje:**

El botón externo está en el pin 2 porque es uno de los pines del Arduino que admite interrupciones. El LED2 está en el pin 10 porque ese pin permite cambiar su brillo con PWM. El LCD usa solo 4 cables de datos, el pin RW va a GND y el contraste se regula con un potenciómetro. Todos los componentes se alimentan desde las líneas de 5V y GND de la protoboard, conectadas al Arduino.

### Esquema
![Esquema del circuito](./docs/vending_machine_circuit.png)

## Estructura del código
El programa está dividido en varios ficheros, cada uno con una responsabilidad concreta.
 
```
vending_machine/
├── vending_machine.ino     setup(), loop(), máquina de estados y pantallas del LCD
├── config.h                pines y constantes (tiempos, umbrales, precios y límites)
├── button.h                declaraciones del botón
├── button.cpp              interrupción, antirrebote y duración de cada pulsación
├── joystick.h              declaraciones del joystick
├── joystick.cpp            lectura del joystick, cada movimiento o pulsación una sola vez
├── sensors.h               declaraciones de los sensores
└── sensors.cpp             lectura del sensor de ultrasonidos y del DHT11
```

Con esta estructura, cada componente tiene su propio fichero y es fácil encontrar dónde está cada parte del programa. Además, todos los pines y tiempos están en [`config.h`](./vending_machine/config.h), así que cualquier cambio se hace en un único lugar sin tocar el resto del código.

