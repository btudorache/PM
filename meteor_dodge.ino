#include <SPI.h>
#include <TFT.h>

#define CS 10
#define DC 9
#define RST 8 

#define MAX_METEORS 15
#define METEOR_RADIUS 8
#define INITIAL_METEOR_UPDATE_REFRESH 210
#define INITIAL_OCR1A_METEOR_SPAWN 16000

#define SHIP_RADIUS 7

#define SHOT_RADIUS 3
#define SHOT_UPDATE_REFRESH 200

#define SHOT_GENERATE_DEBOUNCE 300
#define MOVEMENT_DEBOUNCE 200

#define MAX_LIVES 3

TFT screen = TFT(CS, DC, RST);

unsigned char movedShip = 0;
unsigned char shipX = 20;
unsigned char shipY = 64;
unsigned char shipLives = MAX_LIVES;

unsigned char shooting = 0;
unsigned char drawShot = 0;
unsigned char shootX = 0;
unsigned char shootY = 0;

long movementDebounceTs = 0;
long meteorDebounceTs = 0;
long shootGenerateDebounceTs = 0;
long shootUpdateDebounceTs = 0;

int meteor_update_refresh = INITIAL_METEOR_UPDATE_REFRESH;
unsigned char numMeteors = 0;
unsigned char meteorX[MAX_METEORS];
unsigned char meteorY[MAX_METEORS];
unsigned char meteorAlive[MAX_METEORS];

unsigned char gameRunning = 0;
unsigned char gamePrep = 0;
unsigned char initialScreen = 1;
int score = 0;

ISR(TIMER1_COMPA_vect) {
    score += 10;
    if (score % 100 == 0) {
        if (meteor_update_refresh > 50) {
            meteor_update_refresh -= 30;
        }

        if (OCR1A > 3000) {
            OCR1A -= 1000;
        }
    }
    
    if (numMeteors < MAX_METEORS) {
        meteorY[numMeteors] = random(5, 120);;
        meteorX[numMeteors] = 150;
        meteorAlive[numMeteors] = 1;
        numMeteors += 1;
    } else {
        for (int i = 0; i < numMeteors; i++){
            if (meteorAlive[i] == 0) {
                meteorY[i] = random(5, 120);
                meteorX[i] = 150;
                meteorAlive[i] = 1;
                break;
            }
        }
    }
}

ISR(PCINT0_vect){
    if (millis() - shootGenerateDebounceTs >= SHOT_GENERATE_DEBOUNCE) {
        shootGenerateDebounceTs = millis();
        if (gameRunning && !shooting) {
            shooting = 1;
            drawShot = 1;
            shootX = shipX + 5;
            shootY = shipY;
            PORTD &= ~(1 << PD4);
            return;
        }

        if (!gameRunning) {
            gameRunning = 1;
            gamePrep = 1;
            initialScreen = 0;
        }   
    }
}

ISR(PCINT1_vect){
    if ((millis() - movementDebounceTs) >= MOVEMENT_DEBOUNCE) {
        if (shipY < 118) {
            movementDebounceTs = millis();
            shipY += 5;
            movedShip = 1;
        }
    }
}

ISR(PCINT2_vect){
    if ((millis() - movementDebounceTs) >= MOVEMENT_DEBOUNCE) {
        if (shipY > 10) {
            movementDebounceTs = millis();
            shipY -= 5;
            movedShip = 1;
        }
    }
}

inline void drawShip() {
    // MAIN BACKGROUND
    screen.stroke(204, 204, 204);
    screen.point(shipX, shipY);
    screen.point(shipX, shipY - 1);
    screen.point(shipX, shipY + 1);
    screen.point(shipX - 1, shipY - 1);
    screen.point(shipX - 1, shipY + 1);
    screen.point(shipX - 1, shipY);
    screen.point(shipX + 1, shipY);
    screen.point(shipX - 2, shipY);

    // BORDER
    screen.stroke(32, 52, 52);
    screen.point(shipX + 1, shipY - 1);
    screen.point(shipX + 1, shipY + 1);
    screen.point(shipX - 2, shipY - 1);
    screen.point(shipX - 2, shipY + 1);
    screen.point(shipX - 2, shipY - 2);
    screen.point(shipX - 2, shipY + 2);
    screen.point(shipX + 2, shipY);
    screen.point(shipX + 3, shipY);
    screen.point(shipX - 3, shipY);
    screen.point(shipX - 3, shipY - 1);
    screen.point(shipX - 3, shipY + 1);
    screen.point(shipX - 3, shipY - 3);
    screen.point(shipX - 3, shipY + 3);
    screen.point(shipX + 2, shipY - 2);
    screen.point(shipX + 2, shipY + 2);
    screen.point(shipX - 2, shipY - 4);
    screen.point(shipX - 2, shipY + 4);
    screen.point(shipX - 1, shipY - 5);
    screen.point(shipX - 1, shipY + 5);
    screen.point(shipX, shipY - 5);
    screen.point(shipX, shipY + 5);
    screen.point(shipX + 1, shipY - 6);
    screen.point(shipX + 1, shipY + 6);
    screen.point(shipX + 2, shipY - 6);
    screen.point(shipX + 2, shipY + 6);
    screen.point(shipX + 3, shipY - 6);
    screen.point(shipX + 3, shipY + 6);
    screen.point(shipX + 4, shipY - 5);
    screen.point(shipX + 4, shipY + 5);
    screen.point(shipX + 5, shipY - 3);
    screen.point(shipX + 5, shipY + 3);
    screen.point(shipX + 5, shipY - 4);
    screen.point(shipX + 5, shipY + 4);
    screen.point(shipX + 6, shipY - 3);
    screen.point(shipX + 6, shipY + 3);
    screen.point(shipX + 3, shipY - 3);
    screen.point(shipX + 3, shipY + 3);
    screen.point(shipX + 4, shipY - 3);
    screen.point(shipX + 4, shipY + 3);
    

    // SHOT/BACK FLAMES
    screen.stroke(0, 0, 255);
    screen.point(shipX - 5, shipY - 1);
    screen.point(shipX - 5, shipY + 1);
    screen.point(shipX - 6, shipY);

    // SECONDARY BLACK FLAMES
    screen.stroke(37, 151, 199);
    screen.point(shipX - 5, shipY - 3);
    screen.point(shipX - 5, shipY + 3);
    screen.point(shipX - 4, shipY);
    screen.stroke(37, 108, 199);
    screen.point(shipX - 4, shipY - 3);
    screen.point(shipX - 4, shipY + 3);
    screen.point(shipX - 4, shipY - 1);
    screen.point(shipX - 4, shipY + 1);
    screen.point(shipX - 5, shipY);

    // BLUE TINT
    screen.stroke(255, 20, 20);
    screen.point(shipX - 1, shipY - 2);
    screen.point(shipX - 1, shipY + 2);
    screen.point(shipX + 1, shipY - 4);
    screen.point(shipX + 1, shipY + 4);
    screen.point(shipX + 2, shipY - 4);
    screen.point(shipX + 2, shipY + 4);
   
    // SECONDARY BACKGROUND
    screen.stroke(102, 102, 102);
    screen.point(shipX, shipY - 2);
    screen.point(shipX, shipY + 2);
    screen.point(shipX + 1, shipY - 2);
    screen.point(shipX + 1, shipY + 2);
    screen.point(shipX + 1, shipY - 3);
    screen.point(shipX + 1, shipY + 3);
    screen.point(shipX, shipY - 3);
    screen.point(shipX, shipY + 3);
    screen.point(shipX - 1, shipY - 3);
    screen.point(shipX - 1, shipY + 3);
    screen.point(shipX - 2, shipY - 3);
    screen.point(shipX - 2, shipY + 3);
    screen.point(shipX + 2, shipY - 3);
    screen.point(shipX + 2, shipY + 3);
    screen.point(shipX - 1, shipY - 4);
    screen.point(shipX - 1, shipY + 4);
    screen.point(shipX, shipY - 4);
    screen.point(shipX, shipY + 4);
    screen.point(shipX + 1, shipY - 5);
    screen.point(shipX + 1, shipY + 5);
    screen.point(shipX + 2, shipY - 5);
    screen.point(shipX + 2, shipY + 5);
    screen.point(shipX + 3, shipY - 5);
    screen.point(shipX + 3, shipY + 5);
    screen.point(shipX + 3, shipY - 4);
    screen.point(shipX + 3, shipY + 4);
    screen.point(shipX + 4, shipY - 4);
    screen.point(shipX + 4, shipY + 4);
}

inline void drawMeteors() {
    screen.stroke(0, 0, 255);
    screen.noFill();
    for (int i = 0; i < numMeteors; i++) {
        if (meteorAlive[i]) {
            screen.circle(meteorX[i], meteorY[i], METEOR_RADIUS);
        }
    }
}

inline void refreshShip() {
    if (movedShip) {
        screen.stroke(0, 0, 0);
        screen.fill(0, 0, 0);
        screen.rect(shipX - 6, shipY - 11, 14, 23);
        movedShip = 0;
    }
}

inline void refreshShot() {
    if (shooting) {
        screen.stroke(0, 0, 0);
        screen.fill(0, 0, 0);
        screen.circle(shootX - 5, shootY, SHOT_RADIUS);
    }
}

inline void refreshMeteors() {
    screen.stroke(0, 0, 0);
    for (int i = 0; i < numMeteors; i++) {
        if (meteorAlive[i]) {
            screen.fill(0, 0, 0);
            screen.circle(meteorX[i] + 5, meteorY[i], METEOR_RADIUS);
        }
    }
}

inline void drawShoot() {
    if (drawShot) {
        screen.stroke(255, 0, 0);
        screen.circle(shootX, shootY, SHOT_RADIUS);
        screen.stroke(20, 100, 255);
        screen.point(shootX - 1, shootY - 1);
        screen.point(shootX, shootY - 1);
        screen.point(shootX + 1, shootY - 1);

        screen.point(shootX - 2, shootY);
        screen.point(shootX - 1, shootY);
        screen.point(shootX, shootY);
        screen.point(shootX + 1, shootY);
        screen.point(shootX + 2, shootY);

        screen.point(shootX - 2, shootY + 1);
        screen.point(shootX - 1, shootY + 1);
        screen.point(shootX, shootY + 1);
        screen.point(shootX + 1, shootY + 1);
        screen.point(shootX + 2, shootY + 1);

        screen.point(shootX - 1, shootY + 2);
        screen.point(shootX, shootY + 2);
        screen.point(shootX + 1, shootY + 2);

        screen.stroke(255, 255, 255);
        screen.point(shootX, shootY);
    }
}

inline void updateMeteors() {
    if ((millis() - meteorDebounceTs) >= meteor_update_refresh) {
        meteorDebounceTs = millis();
           
        for (int i = 0; i < numMeteors; i++) {
            meteorX[i] -= 5;
            if (meteorX[i] < 7) {
                destroyMeteor(i);
            }
        }
    }
}

inline void destroyShot() {
    shooting = 0;
    screen.stroke(0, 0, 0);
    screen.fill(0, 0, 0);
    screen.circle(shootX - 5, shootY, SHOT_RADIUS);
    screen.circle(shootX, shootY, SHOT_RADIUS);
    
    shootX = -10;
    shootY = -10;
    PORTD |= (1 << PD4);
}

inline void updateShot() {
    if (shooting && (millis() - shootUpdateDebounceTs) >= SHOT_UPDATE_REFRESH) {
        shootUpdateDebounceTs = millis();
           
        shootX += 5;
        if (shootX > 150) {
            destroyShot();
        }
    }
}

inline void destroyMeteor(int meteorNum) {
    meteorAlive[meteorNum] = 0;
    screen.stroke(0, 0, 0);
    screen.circle(meteorX[meteorNum], meteorY[meteorNum], METEOR_RADIUS);
    screen.circle(meteorX[meteorNum] + 5, meteorY[meteorNum], METEOR_RADIUS);
}

inline void shipMeteorsCollision() {
    double shipDistX = shipX + SHIP_RADIUS;
    double shipDistY = shipY + SHIP_RADIUS;
    for (int i = 0; i < numMeteors; i++) {
        if (meteorAlive[i]) {
            double dx = (meteorX[i] + METEOR_RADIUS) - shipDistX; 
            double dy = (meteorY[i] + METEOR_RADIUS) - shipDistY; 
            double dist = dx * dx + dy * dy;

            if (dist < (METEOR_RADIUS + SHIP_RADIUS) * (METEOR_RADIUS + SHIP_RADIUS)) {
                destroyMeteor(i);
                if (shipLives == 3) {
                    PORTD &= ~(1 << PD6);
                } else if (shipLives == 2) {
                    PORTD &= ~(1 << PD5);
                } else if (shipLives == 1) {
                    PORTD &= ~(1 << PD3);
                }
                shipLives -= 1;
                
                if (shipLives == 0) {
                    gameRunning = 0;
                }
            }
        }
    }
}

inline void shotMeteorsCollision() {
    if (drawShot) {
        double shotDistX = shootX + SHOT_RADIUS;
        double shotDistY = shootY + SHOT_RADIUS + 2;
        for (int i = 0; i < numMeteors; i++) {
            if (meteorAlive[i]) {
                double dx = (meteorX[i] + METEOR_RADIUS) - shotDistX; 
                double dy = (meteorY[i] + METEOR_RADIUS) - shotDistY; 
                double dist = dx * dx + dy * dy;
    
                if (dist < (METEOR_RADIUS + SHOT_RADIUS) * (METEOR_RADIUS + SHOT_RADIUS)) {
                    destroyMeteor(i);
                    drawShot = 0;
                }
            }
        }
    }
}

void inline prepGame() {
    screen.stroke(255, 255, 255);
    screen.background(0, 0, 0);
    shipLives = MAX_LIVES;
    gamePrep = 0;
    score = 0;
    shooting = 0;
    drawShot = 0;
    numMeteors = 0;
    movedShip = 0;
    meteor_update_refresh = INITIAL_METEOR_UPDATE_REFRESH;
    OCR1A = INITIAL_OCR1A_METEOR_SPAWN;
    
    TIMSK1 |= (1 << OCIE1A);
    PORTD |= (1 << PD6);
    PORTD |= (1 << PD5);
    PORTD |= (1 << PD3);
}

void inline checkEndGame() {
    if (!gameRunning) {
        screen.background(0, 0, 0);
        TIMSK1 &= ~(1 << OCIE1A);
    }
}

void setup() {
    // setup left - right buttons
    // DREAPTA - PCI1
    DDRC &= ~(1 << PC0);
    PORTC |= (1 << PC0);

    // STANGA - PCI2
    DDRD &= ~(1 << PD2);
    PORTD |= (1 << PD2);

    // PUTERE - PCI0
    DDRB &= ~(1 << PB4);
    PORTB |= (1 << PB4);

    // ACTIVARE INTRERUPERI PE BUTOANE
    PCICR |= (1 << PCIE2);
    PCICR |= (1 << PCIE1);
    PCICR |= (1 << PCIE0);
    
    PCMSK2 |= (1 << PCINT18);
    PCMSK1 |= (1 << PCINT8);
    PCMSK0 |= (1 << PCINT4);

    // ACTIVARE TIMER 1 SI INTRERUPERI - SPAWN METEORITI
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    OCR1A = INITIAL_OCR1A_METEOR_SPAWN;
    TCCR1B |= (1 << WGM12);   // CTC mode
    TCCR1B |= (1 << CS12);    // 256 prescaler 
    TCCR1B |= (1 << CS10);
    TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt

    // activare led putere
    DDRD |= (1 << PD4);
    PORTD |= (1 << PD4);

    // activare led rgb
    DDRD |= (1 << PD6);
    DDRD |= (1 << PD5);
    DDRD |= (1 << PD3);

    PORTD |= (1 << PD6);
    PORTD |= (1 << PD5);
    PORTD |= (1 << PD3);
    
    sei();
    
    screen.begin();
    screen.background(0, 0, 0);
}

void loop() {
    if (gameRunning) {
        if (gamePrep) {
            prepGame();
        }
        // first game loop phase
        updateMeteors();
        updateShot();
        
        // second game loop phase
        shipMeteorsCollision();
        shotMeteorsCollision();
        
        // third game loop phase
        refreshMeteors();
        refreshShot();
        refreshShip();
        
        // last game loop phase
        drawMeteors();
        drawShoot(); 
        drawShip();
        // this refresh is needed because it is not known when
        // the ship position will be updated
        refreshShip();

        // end game checking
        checkEndGame();
    } else {
        if (initialScreen) {
            screen.setRotation(-2);
            screen.stroke(255,255,255);
            screen.text("Press Power to start!", 3, 50);
            screen.setRotation(1); 
        } else {
            screen.setRotation(-2);
            screen.stroke(255,255,255);
            screen.text("Score: ", 10, 20);
            screen.text(String(score).c_str(), 50, 20);
            screen.text("Push Power to play!", 10, 50);
            screen.setRotation(1);   
        }
    }
}
