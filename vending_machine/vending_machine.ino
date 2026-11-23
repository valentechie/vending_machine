#include <LiquidCrystal.h>
#include <Thread.h>
#include <ThreadController.h>
#include <avr/wdt.h>

#include "config.h"
#include "inputs.h"

LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// hilos, tareas que se repiten cada cierto tiempo sin bloquear el programa
ThreadController controller = ThreadController();
Thread distanceThread = Thread();
Thread environmentThread = Thread();
Thread refreshThread = Thread();

// los tres modos de la máquina
enum Mode {
    STARTUP,
    SERVICE,
    ADMIN
};

Mode mode = STARTUP;

// las fases del modo servicio
enum ServiceState {
    WAITING,
    SHOW_ENV,
    MENU,
    PREPARING,
    RETIRE
};

ServiceState serviceState = WAITING;

// las pantallas del modo admin
enum AdminState {
    ADMIN_MENU,
    ADMIN_TEMP,
    ADMIN_DIST,
    ADMIN_COUNTER,
    ADMIN_PRICES,
    ADMIN_EDIT_PRICE
};

AdminState adminState = ADMIN_MENU;

const char *PRODUCT_NAMES[NUM_PRODUCTS] = {
    "Cafe Solo",
    "Cafe Cortado",
    "Cafe Doble",
    "Cafe Premium",
    "Chocolate"
};

// precios en céntimos, 100 son 1.00 EUR
int prices[NUM_PRODUCTS] = {100, 110, 125, 150, 200};

// opciones del menú admin
const char *ADMIN_OPTIONS[NUM_ADMIN_OPTIONS] = {
    "a. Temperatura",
    "b. Distancia",
    "c. Contador",
    "d. Precios"
};

// variables del modo arranque
unsigned long lastBlink = 0; // cuándo cambió el LED1 por última vez
int toggles = 0; // cuántas veces ha cambiado

// momento en el que empezó la fase actual
unsigned long stateStart = 0;

// variables del modo servicio
int selected = 0; // producto que se ve en el menú
unsigned long prepTime = 0; // tiempo aleatorio de la preparación
bool randomSeeded = false; // para preparar los números aleatorios una sola vez

// variables del modo admin
int adminOption = 0; // opción que se ve en el menú admin
int priceIndex = 0; // producto cuyo precio se cambia
int editPrice = 0; // precio nuevo mientras lo cambio

void setup() {
    // apago el watchdog mientras arranca todo
    wdt_disable();

    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);

    lcd.begin(16, 2);
    lcd.print("CARGANDO ...");

    inputsSetup();

    // hilo que mide la distancia cada 200 ms
    distanceThread.enabled = true;
    distanceThread.onRun(updateDistance);
    distanceThread.setInterval(MEASURE_INTERVAL);

    // hilo que lee la temperatura y la humedad cada 2 segundos
    environmentThread.enabled = true;
    environmentThread.onRun(updateEnvironment);
    environmentThread.setInterval(DHT_INTERVAL);

    // hilo que actualiza las pantallas de admin, empieza apagado
    refreshThread.enabled = false;
    refreshThread.onRun(showAdminView);
    refreshThread.setInterval(ADMIN_REFRESH);

    controller.add(&distanceThread);
    controller.add(&environmentThread);
    controller.add(&refreshThread);

    // el primer parpadeo empieza ya encendido
    digitalWrite(LED1, HIGH);
    toggles = 1;
    lastBlink = millis();

    // si el programa se queda colgado más de 2 segundos, la placa se reinicia sola
    wdt_enable(WDTO_2S);
}

void loop() {
    // aviso al watchdog de que el programa sigue funcionando
    wdt_reset();

    // ejecuta los hilos a los que les toca
    controller.run();

    // miro si el botón externo pide cancelar o el modo admin
    handleButton();

    // solo se ejecuta el código del modo en el que está la máquina
    switch (mode) {
        case STARTUP:
            runStartup();
            break;

        case SERVICE:
            runService();
            break;

        case ADMIN:
            runAdmin();
            break;
    }
}

// decide qué hacer según cuánto tiempo se ha pulsado el botón externo
void handleButton() {
    ButtonEvent event = readButton();

    if (event == BTN_ADMIN) {
        // 5 segundos o más, entro o salgo del modo admin
        if (mode == ADMIN) {
            exitAdmin();
        } else {
            enterAdmin();
        }
    } else if (event == BTN_CANCEL) {
        // entre 2 y 3 segundos, cancelo y vuelvo al principio del modo servicio
        if (mode == SERVICE) {
            goToWaiting();
        }
    }
}

// modo arranque, el LED1 parpadea 3 veces
void runStartup() {
    // si ha pasado un segundo desde el último cambio del LED
    if (millis() - lastBlink >= LED1_INTERVAL) {
        lastBlink = millis();

        // pongo el LED al revés de como estaba y cuento el cambio
        digitalWrite(LED1, !digitalRead(LED1));
        toggles = toggles + 1;

        // después de 3 parpadeos paso al modo servicio
        if (toggles >= STARTUP_TOGGLES) {
            digitalWrite(LED1, LOW);
            mode = SERVICE;
            goToWaiting();
        }
    }
}

// modo servicio, atiende al cliente fase a fase
void runService() {
    // leo el joystick en cada vuelta para no perder ningún movimiento
    JoyEvent joy = readJoystick();

    switch (serviceState) {
        case WAITING:
            // ¿hay alguien a menos de 1 metro?
            if (getDistance() < PRESENCE_DISTANCE) {
                // la primera vez preparo los números aleatorios con un valor que no se repite
                if (!randomSeeded) {
                    randomSeed(micros());
                    randomSeeded = true;
                }

                lcd.clear();
                showEnvironment();
                stateStart = millis();
                serviceState = SHOW_ENV;
            }
            break;

        case SHOW_ENV:
            // después de 5 segundos enseño el menú de productos
            if (millis() - stateStart >= ENV_DISPLAY_TIME) {
                selected = 0;
                showProduct(selected);
                serviceState = MENU;
            }
            break;

        case MENU:
            // arriba y abajo cambian de producto, pulsar el joystick lo elige
            if (joy == JOY_UP && selected > 0) {
                selected = selected - 1;
                showProduct(selected);
            } else if (joy == JOY_DOWN && selected < NUM_PRODUCTS - 1) {
                selected = selected + 1;
                showProduct(selected);
            } else if (joy == JOY_PRESS) {
                // tiempo aleatorio entre 4 y 8 segundos
                prepTime = random(PREP_MIN, PREP_MAX + 1);
                stateStart = millis();

                lcd.clear();
                lcd.print("Preparando");
                lcd.setCursor(0, 1);
                lcd.print("Cafe...");
                serviceState = PREPARING;
            }
            break;

        case PREPARING: {
            // tiempo que lleva preparándose
            unsigned long elapsed = millis() - stateStart;

            if (elapsed >= prepTime) {
                // ya está lista la bebida
                analogWrite(LED2, 255);
                lcd.clear();
                lcd.print("RETIRE BEBIDA");
                stateStart = millis();
                serviceState = RETIRE;
            } else {
                // el brillo del LED2 sube según el progreso (de 0 a 255)
                analogWrite(LED2, map(elapsed, 0, prepTime, 0, 255));
            }
            break;
        }

        case RETIRE:
            // después de 3 segundos vuelvo a esperar al siguiente cliente
            if (millis() - stateStart >= RETIRE_TIME) {
                goToWaiting();
            }
            break;
    }
}

// vuelve al principio del modo servicio y apaga los LEDs
void goToWaiting() {
    serviceState = WAITING;
    digitalWrite(LED1, LOW);
    analogWrite(LED2, 0);

    // el mensaje no cabe en una fila, por eso uso las dos
    lcd.clear();
    lcd.print("ESPERANDO");
    lcd.setCursor(0, 1);
    lcd.print("CLIENTE");
}

// entra en el modo admin y enseña su menú
void enterAdmin() {
    mode = ADMIN;

    // en el modo admin los dos LEDs están encendidos
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);

    adminState = ADMIN_MENU;
    adminOption = 0;
    showAdminMenu();
}

// sale del modo admin y vuelve a esperar cliente
void exitAdmin() {
    // apago el hilo de refresco para que no escriba encima del modo servicio
    refreshThread.enabled = false;
    mode = SERVICE;
    goToWaiting();
}

// modo admin, menú y pantallas de administración
void runAdmin() {
    JoyEvent joy = readJoystick();

    switch (adminState) {
        case ADMIN_MENU:
            // arriba y abajo cambian de opción, pulsar la abre
            if (joy == JOY_UP && adminOption > 0) {
                adminOption = adminOption - 1;
                showAdminMenu();
            } else if (joy == JOY_DOWN && adminOption < NUM_ADMIN_OPTIONS - 1) {
                adminOption = adminOption + 1;
                showAdminMenu();
            } else if (joy == JOY_PRESS) {
                openAdminOption();
            }
            break;

        case ADMIN_TEMP:
        case ADMIN_DIST:
        case ADMIN_COUNTER:
            // izquierda, vuelvo al menú y apago el hilo de refresco
            if (joy == JOY_LEFT) {
                refreshThread.enabled = false;
                adminState = ADMIN_MENU;
                showAdminMenu();
            }
            break;

        case ADMIN_PRICES:
            // arriba y abajo cambian de producto
            if (joy == JOY_UP && priceIndex > 0) {
                priceIndex = priceIndex - 1;
                showProduct(priceIndex);
            } else if (joy == JOY_DOWN && priceIndex < NUM_PRODUCTS - 1) {
                priceIndex = priceIndex + 1;
                showProduct(priceIndex);
            } else if (joy == JOY_PRESS) {
                // pulsar, empiezo a cambiar el precio de este producto
                editPrice = prices[priceIndex];
                adminState = ADMIN_EDIT_PRICE;
                showPriceEdit();
            } else if (joy == JOY_LEFT) {
                // izquierda, vuelvo al menú admin
                adminState = ADMIN_MENU;
                showAdminMenu();
            }
            break;

        case ADMIN_EDIT_PRICE:
            // arriba sube 0.05 EUR y abajo baja 0.05 EUR
            if (joy == JOY_UP && editPrice + PRICE_STEP <= MAX_PRICE) {
                editPrice = editPrice + PRICE_STEP;
                showPriceEdit();
            } else if (joy == JOY_DOWN && editPrice - PRICE_STEP >= MIN_PRICE) {
                editPrice = editPrice - PRICE_STEP;
                showPriceEdit();
            } else if (joy == JOY_PRESS) {
                // pulsar, guardo el precio nuevo
                prices[priceIndex] = editPrice;
                adminState = ADMIN_PRICES;
                showProduct(priceIndex);
            } else if (joy == JOY_LEFT) {
                // izquierda, vuelvo atrás sin guardar
                adminState = ADMIN_PRICES;
                showProduct(priceIndex);
            }
            break;
    }
}

// abre la opción elegida en el menú admin
void openAdminOption() {
    if (adminOption == 0) {
        adminState = ADMIN_TEMP;
    } else if (adminOption == 1) {
        adminState = ADMIN_DIST;
    } else if (adminOption == 2) {
        adminState = ADMIN_COUNTER;
    } else {
        adminState = ADMIN_PRICES;
        priceIndex = 0;
        showProduct(priceIndex);
        return;
    }

    // escribo la parte fija de la pantalla una sola vez
    lcd.clear();
    if (adminState == ADMIN_DIST) {
        lcd.print("Distancia:");
    } else if (adminState == ADMIN_COUNTER) {
        lcd.print("Tiempo encendido");
    }

    // enseño los valores y enciendo el hilo que los actualiza cada medio segundo
    showAdminView();
    refreshThread.enabled = true;
}

// borra una fila del LCD y deja el cursor al principio
void clearRow(int row) {
    lcd.setCursor(0, row);
    lcd.print("                ");
    lcd.setCursor(0, row);
}

// escribe un precio en céntimos como 1.25 EUR
void printPrice(int cents) {
    lcd.print(cents / 100);
    lcd.print('.');
    if (cents % 100 < 10) {
        lcd.print('0');
    }
    lcd.print(cents % 100);
    lcd.print(" EUR");
}

// temperatura arriba y humedad abajo
void showEnvironment() {
    clearRow(0);
    lcd.print("Temp: ");
    if (isnan(getTemperature())) {
        lcd.print("--");
    } else {
        lcd.print(getTemperature(), 0);
    }
    lcd.print((char)223); // símbolo de grado
    lcd.print("C");

    clearRow(1);
    lcd.print("Hum: ");
    if (isnan(getHumidity())) {
        lcd.print("--");
    } else {
        lcd.print(getHumidity(), 0);
    }
    lcd.print(" %");
}

// nombre del producto arriba y precio abajo
void showProduct(int index) {
    lcd.clear();
    lcd.print(PRODUCT_NAMES[index]);

    lcd.setCursor(0, 1);
    printPrice(prices[index]);
}

// MENU ADMIN arriba y la opción en la que estoy abajo
void showAdminMenu() {
    lcd.clear();
    lcd.print("MENU ADMIN");

    lcd.setCursor(0, 1);
    lcd.print(ADMIN_OPTIONS[adminOption]);
}

// valores de las pantallas de admin, la llama el hilo de refresco
void showAdminView() {
    if (adminState == ADMIN_TEMP) {
        showEnvironment();
    } else if (adminState == ADMIN_DIST) {
        // solo reescribo la fila de abajo, que es la que cambia
        clearRow(1);
        if (getDistance() >= NO_ECHO_DISTANCE) {
            lcd.print("Fuera de rango");
        } else {
            lcd.print(getDistance(), 0);
            lcd.print(" cm");
        }
    } else if (adminState == ADMIN_COUNTER) {
        // segundos desde que se encendió la placa
        clearRow(1);
        lcd.print(millis() / 1000);
        lcd.print(" s");
    }
}

// pantalla para cambiar el precio
void showPriceEdit() {
    lcd.clear();
    lcd.print(PRODUCT_NAMES[priceIndex]);
    lcd.setCursor(0, 1);
    lcd.print("Nuevo: ");
    printPrice(editPrice);
}
