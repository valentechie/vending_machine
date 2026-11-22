#include <LiquidCrystal.h>
#include <Thread.h>
#include <ThreadController.h>
#include <avr/wdt.h>

#include "config.h"
#include "sensors.h"
#include "joystick.h"
#include "button.h"

LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

ThreadController controller = ThreadController();
Thread distanceThread = Thread();
Thread environmentThread = Thread();

enum Mode {
    STARTUP,
    SERVICE,
    ADMIN
};

Mode mode = STARTUP;

enum ServiceState {
    WAITING,
    SHOW_ENV,
    MENU,
    PREPARING,
    RETIRE
};

ServiceState serviceState = WAITING;

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

int prices[NUM_PRODUCTS] = {100, 110, 125, 150, 200};

const char *ADMIN_OPTIONS[NUM_ADMIN_OPTIONS] = {
    "Ver temperatura",
    "Ver distancia",
    "Ver contador",
    "Modif. precios"
};

unsigned long lastBlink = 0;
int toggles = 0;

unsigned long stateStart = 0;

int selected = 0;
unsigned long prepTime = 0;
bool randomSeeded = false;

int adminOption = 0;
int priceIndex = 0;
int editPrice = 0;
unsigned long lastRefresh = 0;

void setup() {
    wdt_disable();

    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);

    lcd.begin(16, 2);
    lcd.print("CARGANDO ...");

    sensorsSetup();
    joystickSetup();
    buttonSetup();

    distanceThread.enabled = true;
    distanceThread.onRun(updateDistance);
    distanceThread.setInterval(MEASURE_INTERVAL);
    environmentThread.enabled = true;
    environmentThread.onRun(updateEnvironment);
    environmentThread.setInterval(DHT_INTERVAL);

    controller.add(&distanceThread);
    controller.add(&environmentThread);

    wdt_enable(WDTO_2S);
}

void loop() {
    wdt_reset();

    controller.run();

    handleButton();

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

void handleButton() {
    unsigned long d = getPressDuration();

    if (d == 0) {
        return;
    }

    if (d >= ADMIN_TIME) {
        if (mode == ADMIN) {
            exitAdmin();
        } else {
            enterAdmin();
        }
    } else if (d >= CANCEL_MIN && d <= CANCEL_MAX) {
        if (mode == SERVICE && serviceState != WAITING) {
            goToWaiting();
        }
    }
}

void runStartup() {
    if (millis() - lastBlink >= LED1_INTERVAL) {
        lastBlink = millis();

        digitalWrite(LED1, !digitalRead(LED1));
        toggles = toggles + 1;

        if (toggles >= STARTUP_TOGGLES) {
            digitalWrite(LED1, LOW);
            mode = SERVICE;
            goToWaiting();
        }
    }
}

void runService() {
    JoyEvent joy = readJoystick();

    switch (serviceState) {
        case WAITING:
            if (getDistance() < PRESENCE_DISTANCE) {
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
            if (millis() - stateStart >= ENV_DISPLAY_TIME) {
                selected = 0;
                showProduct(selected);
                serviceState = MENU;
            }
            break;

        case MENU:
            if (joy == JOY_UP && selected > 0) {
                selected = selected - 1;
                showProduct(selected);
            } else if (joy == JOY_DOWN && selected < NUM_PRODUCTS - 1) {
                selected = selected + 1;
                showProduct(selected);
            } else if (joy == JOY_PRESS) {
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
            unsigned long elapsed = millis() - stateStart;

            if (elapsed >= prepTime) {
                analogWrite(LED2, 255);
                lcd.clear();
                lcd.print("RETIRE BEBIDA");
                stateStart = millis();
                serviceState = RETIRE;
            } else {
                analogWrite(LED2, map(elapsed, 0, prepTime, 0, 255));
            }
            break;
        }

        case RETIRE:
            if (millis() - stateStart >= RETIRE_TIME) {
                goToWaiting();
            }
            break;
    }
}

void goToWaiting() {
    serviceState = WAITING;
    digitalWrite(LED1, LOW);
    analogWrite(LED2, 0);

    lcd.clear();
    lcd.print("ESPERANDO");
    lcd.setCursor(0, 1);
    lcd.print("CLIENTE");
}

void enterAdmin() {
    mode = ADMIN;

    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);

    adminState = ADMIN_MENU;
    adminOption = 0;
    showAdminMenu();
}

void exitAdmin() {
    mode = SERVICE;
    goToWaiting();
}

void runAdmin() {
    JoyEvent joy = readJoystick();

    switch (adminState) {
        case ADMIN_MENU:
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
            if (joy == JOY_LEFT) {
                adminState = ADMIN_MENU;
                showAdminMenu();
            } else if (millis() - lastRefresh >= ADMIN_REFRESH) {
                lastRefresh = millis();
                showAdminView();
            }
            break;

        case ADMIN_PRICES:
            if (joy == JOY_UP && priceIndex > 0) {
                priceIndex = priceIndex - 1;
                showProduct(priceIndex);
            } else if (joy == JOY_DOWN && priceIndex < NUM_PRODUCTS - 1) {
                priceIndex = priceIndex + 1;
                showProduct(priceIndex);
            } else if (joy == JOY_PRESS) {
                editPrice = prices[priceIndex];
                adminState = ADMIN_EDIT_PRICE;
                showPriceEdit();
            } else if (joy == JOY_LEFT) {
                adminState = ADMIN_MENU;
                showAdminMenu();
            }
            break;

        case ADMIN_EDIT_PRICE:
            if (joy == JOY_UP && editPrice + PRICE_STEP <= MAX_PRICE) {
                editPrice = editPrice + PRICE_STEP;
                showPriceEdit();
            } else if (joy == JOY_DOWN && editPrice - PRICE_STEP >= MIN_PRICE) {
                editPrice = editPrice - PRICE_STEP;
                showPriceEdit();
            } else if (joy == JOY_PRESS) {
                prices[priceIndex] = editPrice;
                adminState = ADMIN_PRICES;
                showProduct(priceIndex);
            } else if (joy == JOY_LEFT) {
                adminState = ADMIN_PRICES;
                showProduct(priceIndex);
            }
            break;
    }
}

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

    lcd.clear();
    showAdminView();
    lastRefresh = millis();
}

void clearRow(int row) {
    lcd.setCursor(0, row);
    lcd.print("                ");
    lcd.setCursor(0, row);
}

void printPrice(int cents) {
    lcd.print(cents / 100);
    lcd.print('.');
    if (cents % 100 < 10) {
        lcd.print('0');
    }
    lcd.print(cents % 100);
    lcd.print(" EUR");
}

void showEnvironment() {
    clearRow(0);
    lcd.print("Temp: ");
    if (isnan(getTemperature())) {
        lcd.print("--");
    } else {
        lcd.print(getTemperature(), 0);
    }
    lcd.print((char)223);
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

void showProduct(int index) {
    lcd.clear();
    lcd.print(">");
    lcd.print(PRODUCT_NAMES[index]);

    lcd.setCursor(0, 1);
    printPrice(prices[index]);
}

void showAdminMenu() {
    lcd.clear();
    lcd.print("MENU ADMIN");
    lcd.setCursor(15, 0);
    lcd.print((char)('a' + adminOption));

    lcd.setCursor(0, 1);
    lcd.print(">");
    lcd.print(ADMIN_OPTIONS[adminOption]);
}

void showAdminView() {
    if (adminState == ADMIN_TEMP) {
        showEnvironment();
    } else if (adminState == ADMIN_DIST) {
        clearRow(0);
        lcd.print("Distancia:");
        clearRow(1);
        if (getDistance() >= NO_ECHO_DISTANCE) {
            lcd.print("Fuera de rango");
        } else {
            lcd.print(getDistance(), 0);
            lcd.print(" cm");
        }
    } else if (adminState == ADMIN_COUNTER) {
        clearRow(0);
        lcd.print("Tiempo encendido");
        clearRow(1);
        lcd.print(millis() / 1000);
        lcd.print(" s");
    }
}

void showPriceEdit() {
    lcd.clear();
    lcd.print(PRODUCT_NAMES[priceIndex]);
    lcd.setCursor(0, 1);
    lcd.print("Nuevo: ");
    printPrice(editPrice);
}
