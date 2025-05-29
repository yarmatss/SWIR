import serial
import time
import sys
import os

class KontrolerSilnika:
    def __init__(self, port='/dev/ttyUSB0', predkosc=9600):
        """Inicjalizacja kontrolera silnika"""
        self.port = port
        self.predkosc = predkosc
        self.ser = None
        self.polaczony = False
        
        # Parametry regulatora
        self.zadana_predkosc = 0.0
        self.kp = 1.0
        self.ki = 0.04
        self.kd = 1
        
        # Dane z silnika
        self.aktualna_predkosc = 0.0
        self.blad = 0.0
        self.wyjscie_pwm = 0.0

    def polacz(self):
        """Połączenie z Arduino"""
        try:
            self.ser = serial.Serial(self.port, self.predkosc, timeout=1)
            time.sleep(2)  # Czas na reset Arduino
            self.polaczony = True
            print("Połączono z {0} przy {1} baud".format(self.port, self.predkosc))
            return True
        except Exception as e:
            print("Błąd połączenia: {0}".format(str(e)))
            return False

    def rozlacz(self):
        """Zakończenie połączenia"""
        if self.ser:
            self.ser.close()
            self.polaczony = False
            print("Rozłączono")

    def wyslij_komende(self, komenda):
        """Wysyłanie komendy do Arduino"""
        if not self.polaczony:
            print("Brak połączenia z Arduino")
            return False
        
        try:
            self.ser.write((komenda + "\n").encode())
            time.sleep(0.1)  # Czas na przetworzenie
            return True
        except Exception as e:
            print("Błąd wysyłania komendy: {0}".format(str(e)))
            return False

    def odczytaj_dane(self):
        """Odczyt danych z Arduino"""
        if not self.polaczony:
            return []
        
        dane = []
        try:
            while self.ser.in_waiting > 0:
                linia = self.ser.readline().decode('utf-8').strip()
                dane.append(linia)
                
                # Przetwarzanie odczytanych danych
                if linia.startswith("c = "):
                    self.aktualna_predkosc = float(linia[4:])
                elif linia.startswith("e = "):
                    self.blad = float(linia[4:])
                elif linia.startswith("o = "):
                    self.wyjscie_pwm = float(linia[4:])
            
            return dane
        except Exception as e:
            print("Błąd odczytu: {0}".format(str(e)))
            return []

    def ustaw_predkosc(self, predkosc):
        """Ustawienie zadanej prędkości obrotowej"""
        self.zadana_predkosc = predkosc
        komenda = "s{0}".format(predkosc)
        return self.wyslij_komende(komenda)

    def ustaw_kp(self, kp):
        """Ustawienie współczynnika Kp"""
        self.kp = kp
        komenda = "p{0}".format(kp)
        return self.wyslij_komende(komenda)

    def ustaw_ki(self, ki):
        """Ustawienie współczynnika Ki"""
        self.ki = ki
        komenda = "i{0}".format(ki)
        return self.wyslij_komende(komenda)

    def ustaw_kd(self, kd):
        """Ustawienie współczynnika Kd"""
        self.kd = kd
        komenda = "d{0}".format(kd)
        return self.wyslij_komende(komenda)

    def pobierz_parametry(self):
        """Pobranie aktualnych parametrów regulatora"""
        return self.wyslij_komende("?")

    def wyswietl_status(self):
        """Wyświetlenie aktualnego statusu"""
        print("\nStatus silnika:")
        print("----------------------------------")
        print("Prędkość zadana    : {0:.2f} RPM".format(self.zadana_predkosc))
        print("Prędkość aktualna  : {0:.2f} RPM".format(self.aktualna_predkosc))
        print("Błąd              : {0:.2f}".format(self.blad))
        print("Wyjście PWM       : {0:.2f}/255".format(self.wyjscie_pwm))
        print("----------------------------------")
        print("Parametry regulatora PID:")
        print("Kp = {0:.4f}".format(self.kp))
        print("Ki = {0:.4f}".format(self.ki))
        print("Kd = {0:.4f}".format(self.kd))
        print("----------------------------------")

def wyswietl_pomoc():
    """Wyświetlenie dostępnych komend"""
    print("\nDostępne komendy:")
    print("  s <wartość>  - ustaw prędkość obrotową (np. s30)")
    print("  p <wartość>  - ustaw Kp (np. p1.25)")
    print("  i <wartość>  - ustaw Ki (np. i0.1)")
    print("  d <wartość>  - ustaw Kd (np. d0.05)")
    print("  ?            - pokaż aktualne parametry")
    print("  status       - pokaż status systemu")
    print("  clear        - wyczyść ekran")
    print("  help         - wyświetl tę pomoc")
    print("  exit         - wyjście z programu")

def wyczysc_ekran():
    """Czyszczenie ekranu terminala"""
    os.system('clear' if os.name != 'nt' else 'cls')

def main():
    """Główna funkcja programu"""
    print("Program sterowania silnikiem z regulatorem PID")
    print("---------------------------------------------")
    
    # Parametry połączenia
    port = '/dev/ttyUSB0'  # Domyślny port
    predkosc = 9600        # Domyślna prędkość
    
    # Inicjalizacja kontrolera
    kontroler = KontrolerSilnika(port, predkosc)
    
    # Próba połączenia
    if not kontroler.polacz():
        print("Nie można nawiązać połączenia. Program zostanie zakończony.")
        return
    
    # Wyświetlenie pomocy
    wyswietl_pomoc()
    
    # Główna pętla programu
    try:
        while True:
            # Odczyt danych z Arduino
            dane = kontroler.odczytaj_dane()
            
            # Odczyt komendy od użytkownika
            try:
                komenda = raw_input("\n> ")  # Python 2.x
            except NameError:
                komenda = input("\n> ")      # Python 3.x
            
            # Przetwarzanie komendy
            if komenda.lower() == 'exit':
                print("Kończenie programu...")
                break
            elif komenda.lower() == 'help':
                wyswietl_pomoc()
            elif komenda.lower() == 'status':
                kontroler.wyswietl_status()
            elif komenda.lower() == 'clear':
                wyczysc_ekran()
            elif komenda.startswith('s') and len(komenda) > 1:
                try:
                    predkosc = float(komenda[1:])
                    kontroler.ustaw_predkosc(predkosc)
                    print("Ustawiono prędkość: {0}".format(predkosc))
                except ValueError:
                    print("Nieprawidłowa wartość prędkości")
            elif komenda.startswith('p') and len(komenda) > 1:
                try:
                    kp = float(komenda[1:])
                    kontroler.ustaw_kp(kp)
                    print("Ustawiono Kp: {0}".format(kp))
                except ValueError:
                    print("Nieprawidłowa wartość Kp")
            elif komenda.startswith('i') and len(komenda) > 1:
                try:
                    ki = float(komenda[1:])
                    kontroler.ustaw_ki(ki)
                    print("Ustawiono Ki: {0}".format(ki))
                except ValueError:
                    print("Nieprawidłowa wartość Ki")
            elif komenda.startswith('d') and len(komenda) > 1:
                try:
                    kd = float(komenda[1:])
                    kontroler.ustaw_kd(kd)
                    print("Ustawiono Kd: {0}".format(kd))
                except ValueError:
                    print("Nieprawidłowa wartość Kd")
            elif komenda == '?':
                kontroler.pobierz_parametry()
                time.sleep(0.5)  # Czekamy na odpowiedź
                dane = kontroler.odczytaj_dane()
                if dane:
                    for linia in dane:
                        print(linia)
            else:
                print("Nieznana komenda. Wpisz 'help' aby uzyskać pomoc.")
            
            # Małe opóźnienie dla oszczędzania zasobów
            time.sleep(0.1)
                
    except KeyboardInterrupt:
        print("\nPrzerwano działanie programu")
    finally:
        # Rozłączenie przed zakończeniem
        kontroler.rozlacz()

if __name__ == "__main__":
    main()
