// Stałe
const int MOTOR_PIN = 9;       // Pin D9 dla PWM sterującego silnikiem (tranzystor Gate)
const int ENCODER_PIN = 3;     // Pin D3 dla enkodera (hallotron)
const int PWM_MAX = 255;       // Maksymalna wartość PWM

const float WSPOLCZYNNIK_RPM = 4115.0;

// Zmienne dla enkodera
volatile unsigned long czas = 0;
volatile unsigned long pomiar = 0;
volatile int numer = 0;
volatile int pomiary[4] = {1000, 1000, 1000, 1000};

// Zmienne dla PID
float iPredkoscZadana = 0;     // Zadana prędkość obrotowa
float iPredkoscAktualna = 0;   // Aktualna zmierzona prędkość
float Kp = 1.0;                // Wzmocnienie proporcjonalne
float Ki = 0.04;                // Wzmocnienie całkujące
float Kd = 1;               // Wzmocnienie różniczkujące

// Zmienne wewnętrzne PID
float blad = 0;                // Aktualny błąd (error)
float blad_poprzedni = 0;      // Poprzedni błąd
float calka = 0;               // Suma błędów (całka)
float rozniczka = 0;           // Różnica błędów (różniczka)
float wyjscie = 0;             // Wyjście regulatora (sygnał sterujący)

// Zmienne pomocnicze
unsigned long ostatniPomiarCzasu = 0;
unsigned long ostatniaAktualizacja = 0;
const unsigned long CZAS_AKTUALIZACJI = 20; // ms

void setup() {
  // Inicjalizacja komunikacji szeregowej
  Serial.begin(9600);
  
  // Konfiguracja pinów
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(ENCODER_PIN, INPUT_PULLUP);  // Wejście z rezystorem podciągającym
  
  // Konfiguracja przerwań dla enkodera
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), przerwanie, CHANGE);
  
  // Zatrzymanie silnika na początek
  analogWrite(MOTOR_PIN, 0);
  
  delay(1000);
  Serial.println("Regulator silnika PID gotowy");
  Serial.println("Dostępne komendy:");
  Serial.println("s<wartosc> - ustaw prędkość");
  Serial.println("p<wartosc> - ustaw Kp");
  Serial.println("i<wartosc> - ustaw Ki");
  Serial.println("d<wartosc> - ustaw Kd");
  Serial.println("? - odczytaj parametry");
  
  // Wyświetl wartość współczynnika
  Serial.print("Współczynnik RPM = ");
  Serial.println(WSPOLCZYNNIK_RPM);
}

// Funkcja obsługi przerwania dla enkodera
void przerwanie() {
  czas = millis();
  unsigned long delta = czas - pomiar;
  
  // Filtr drgań (debounce)
  if (delta > 5) {
    pomiary[numer] = delta;
    pomiar = czas;
    numer++;
    if (numer > 3) {
      numer = 0;
    }
  }
}

// Funkcja obliczająca aktualną prędkość obrotową
float predkoscMierzona() {
  // Sprawdzenie czy silnik się zatrzymał
  if (millis() - pomiar > 1000) {
    for (int i = 0; i < 4; i++) {
      pomiary[i] = 1000;  // Resetowanie pomiarów gdy silnik stoi
    }
    return 0;
  }
  
  // Obliczanie średniego czasu
  float suma = 0;
  noInterrupts();  // Wyłączamy przerwania na czas odczytu
  for (int i = 0; i < 4; i++) {
    suma += pomiary[i];
  }
  interrupts();  // Włączamy przerwania z powrotem
  
  // Obliczanie prędkości korzystając ze stałej WSPOLCZYNNIK_RPM
  if (suma == 0 || suma > 4000) {
    return 0;
  } else {
    return WSPOLCZYNNIK_RPM / suma;
  }
}

// Aktualizacja regulatora PID
void aktualizacjaPID() {
  // Pomiar aktualnej prędkości
  iPredkoscAktualna = predkoscMierzona();
  
  // Obliczenie błędu
  blad = iPredkoscZadana - iPredkoscAktualna;
  
  // Obliczenie całki z ograniczeniem (anti-windup)
  calka += blad;
  calka = constrain(calka + blad, -250/Ki, 250/Ki);
  
  // Obliczenie różniczki
  rozniczka = blad - blad_poprzedni;
  blad_poprzedni = blad;
  //constrainfcalka + e, -250/ki, 250/ki
  // Obliczenie wyjścia regulatora PID
  wyjscie = (Kp * blad) + (Ki * calka) + (Kd * rozniczka);
  
  // Ograniczenie wyjścia do zakresu PWM
  if (wyjscie > PWM_MAX) wyjscie = PWM_MAX;
  if (wyjscie < 0) wyjscie = 0;
  
  // Sterowanie silnikiem
  analogWrite(MOTOR_PIN, (int)wyjscie);
}

// Obsługa komend z portu szeregowego
void obslugaKomend() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    
    switch (command) {
      case 's':
      case 'S':
        iPredkoscZadana = Serial.parseFloat();
        Serial.print("s = ");
        Serial.println(iPredkoscZadana);
        break;
        
      case 'p':
      case 'P':
        Kp = Serial.parseFloat();
        Serial.print("p = ");
        Serial.println(Kp);
        break;
        
      case 'i':
      case 'I':
        Ki = Serial.parseFloat();
        Serial.print("i = ");
        Serial.println(Ki);
        break;
        
      case 'd':
      case 'D':
        Kd = Serial.parseFloat();
        Serial.print("d = ");
        Serial.println(Kd);
        break;
        
      case '?':
        // Wyświetlenie aktualnych ustawień
        Serial.print("p = ");
        Serial.println(Kp);
        Serial.print("i = ");
        Serial.println(Ki);
        Serial.print("d = ");
        Serial.println(Kd);
        Serial.print("s = ");
        Serial.println(iPredkoscZadana);
        Serial.print("RPM wsp. = ");
        Serial.println(WSPOLCZYNNIK_RPM);
        break;
    }
    
    // Czyszczenie bufora
    while (Serial.available() > 0) {
      Serial.read();
    }
  }
}

// Wysyłanie danych telemetrycznych
void wyslijDane() {
  static unsigned long ostatniaCzasWyslania = 0;
  
  if (millis() - ostatniaCzasWyslania >= 200) {  // Co 200ms
    ostatniaCzasWyslania = millis();
    
    Serial.print("c = ");  // Aktualna prędkość
    Serial.println(iPredkoscAktualna);
    
    Serial.print("e = ");  // Błąd
    Serial.println(blad);
    
    Serial.print("o = ");  // Wyjście PWM
    Serial.println(wyjscie);
  }
}

void loop() {
  // Aktualizacja PID w regularnych odstępach
  if (millis() - ostatniaAktualizacja >= CZAS_AKTUALIZACJI) {
    ostatniaAktualizacja = millis();
    aktualizacjaPID();
  }
  
  // Obsługa komend z portu szeregowego
  obslugaKomend();
  
  // Wysyłanie danych telemetrycznych
  wyslijDane();
}
