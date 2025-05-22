int licznik = 0;
int liczba = 0;
int minimum, maksimum, jasnosc;

// Funkcja wyświetlająca znak na wyświetlaczu 7-segmentowym
void Znak(int iNumer) {
    // Dla wyświetlacza common anode, LOW włącza segment, HIGH wyłącza
    // Pinowanie: D2=a, D3=b, D4=c, D5=d, D6=e, D7=f, D8=g
    switch (iNumer) {
        case 0: // 0: włączamy a,b,c,d,e,f wyłączamy g
            digitalWrite(2, LOW);  // a
            digitalWrite(3, LOW);  // b
            digitalWrite(4, LOW);  // c
            digitalWrite(5, LOW);  // d
            digitalWrite(6, LOW);  // e
            digitalWrite(7, LOW);  // f
            digitalWrite(8, HIGH); // g
            break;
        case 1: // 1: włączamy b,c wyłączamy a,d,e,f,g
            digitalWrite(2, HIGH); // a
            digitalWrite(3, LOW);  // b
            digitalWrite(4, LOW);  // c
            digitalWrite(5, HIGH); // d
            digitalWrite(6, HIGH); // e
            digitalWrite(7, HIGH); // f
            digitalWrite(8, HIGH); // g
            break;
        case 2: // 2: włączamy a,b,g,e,d wyłączamy c,f
            digitalWrite(2, LOW);  // a
            digitalWrite(3, LOW);  // b
            digitalWrite(4, HIGH); // c
            digitalWrite(5, LOW);  // d
            digitalWrite(6, LOW);  // e
            digitalWrite(7, HIGH); // f
            digitalWrite(8, LOW);  // g
            break;
        case 3: // 3: włączamy a,b,c,d,g wyłączamy e,f
            digitalWrite(2, LOW);  // a
            digitalWrite(3, LOW);  // b
            digitalWrite(4, LOW);  // c
            digitalWrite(5, LOW);  // d
            digitalWrite(6, HIGH); // e
            digitalWrite(7, HIGH); // f
            digitalWrite(8, LOW);  // g
            break;
        case 4: // 4: włączamy b,c,f,g wyłączamy a,d,e
            digitalWrite(2, HIGH); // a
            digitalWrite(3, LOW);  // b
            digitalWrite(4, LOW);  // c
            digitalWrite(5, HIGH); // d
            digitalWrite(6, HIGH); // e
            digitalWrite(7, LOW);  // f
            digitalWrite(8, LOW);  // g
            break;
        case 5: // 5: włączamy a,c,d,f,g wyłączamy b,e
            digitalWrite(2, LOW);  // a
            digitalWrite(3, HIGH); // b
            digitalWrite(4, LOW);  // c
            digitalWrite(5, LOW);  // d
            digitalWrite(6, HIGH); // e
            digitalWrite(7, LOW);  // f
            digitalWrite(8, LOW);  // g
            break;
        case 6: // 6: włączamy a,c,d,e,f,g wyłączamy b
            digitalWrite(2, LOW);  // a
            digitalWrite(3, HIGH); // b
            digitalWrite(4, LOW);  // c
            digitalWrite(5, LOW);  // d
            digitalWrite(6, LOW);  // e
            digitalWrite(7, LOW);  // f
            digitalWrite(8, LOW);  // g
            break;
        case 7: // 7: włączamy a,b,c wyłączamy d,e,f,g
            digitalWrite(2, LOW);  // a
            digitalWrite(3, LOW);  // b
            digitalWrite(4, LOW);  // c
            digitalWrite(5, HIGH); // d
            digitalWrite(6, HIGH); // e
            digitalWrite(7, HIGH); // f
            digitalWrite(8, HIGH); // g
            break;
        case 8: // 8: włączamy wszystkie segmenty
            digitalWrite(2, LOW);  // a
            digitalWrite(3, LOW);  // b
            digitalWrite(4, LOW);  // c
            digitalWrite(5, LOW);  // d
            digitalWrite(6, LOW);  // e
            digitalWrite(7, LOW);  // f
            digitalWrite(8, LOW);  // g
            break;
        case 9: // 9: włączamy a,b,c,d,f,g wyłączamy e
            digitalWrite(2, LOW);  // a
            digitalWrite(3, LOW);  // b
            digitalWrite(4, LOW);  // c
            digitalWrite(5, LOW);  // d
            digitalWrite(6, HIGH); // e
            digitalWrite(7, LOW);  // f
            digitalWrite(8, LOW);  // g
            break;
        default: // Nieznany symbol - wyświetl "0"
            digitalWrite(2, LOW);  // a
            digitalWrite(3, LOW);  // b
            digitalWrite(4, LOW);  // c
            digitalWrite(5, LOW);  // d
            digitalWrite(6, LOW);  // e
            digitalWrite(7, LOW);  // f
            digitalWrite(8, HIGH); // g
            break;
    }
}

void setup() {
    // Konfiguracja pinów do wyświetlaczy jako wyjścia
    for(int i = 2; i <= 8; i++)
        pinMode(i, OUTPUT);
    pinMode(10, OUTPUT);
    pinMode(11, OUTPUT);
    
    // Początkowa kalibracja czujnika światła
    delay(1000);
    minimum = analogRead(0);
    maksimum = minimum;
}

void loop() {
    // Obsługa pierwszej cyfry (jedności)
    digitalWrite(10, LOW);  // Wyłącz drugi wyświetlacz
    digitalWrite(11, HIGH); // Włącz pierwszy wyświetlacz
    Znak(liczba % 10);
    delay(2);
    
    // Obsługa drugiej cyfry (dziesiątki)
    digitalWrite(11, LOW);  // Wyłącz pierwszy wyświetlacz
    digitalWrite(10, HIGH); // Włącz drugi wyświetlacz
    Znak((liczba / 10) % 10);
    delay(2);
    
    // Pomiar jasności i aktualizacja wartości co około 200ms
    if (licznik++ > 100) {
        jasnosc = analogRead(0);
        if (minimum > jasnosc) minimum = jasnosc;
        if (maksimum < jasnosc) maksimum = jasnosc;
        
        // Skalowanie do wartości procentowej (0-99)
        if (minimum != maksimum)
            liczba = 99 * (jasnosc - minimum) / (maksimum - minimum);
        
        licznik = 0;
    }
}
