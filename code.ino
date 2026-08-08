#include <HardwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// ==========================================================
// --- PENGATURAN WAKTU & DURASI (UBAH NILAI DI SINI) ---
// ==========================================================
// 1. Durasi Pengisian Air Tunggal
const unsigned long WAKTU_ISI_PANAS  = 7683; 
const unsigned long WAKTU_ISI_DINGIN = 7853;
const unsigned long WAKTU_ISI_NORMAL = 7530; 

// 2. Durasi Pengisian Air Hangat (Campuran Bergiliran)
const unsigned long WAKTU_HANGAT_NORMAL = 3820; 
const unsigned long WAKTU_HANGAT_PANAS  = 4000; 

// 3. Batas Waktu Sistem (Timeouts)
const unsigned long BATAS_WAKTU_KONFIRMASI = 10000; 
const unsigned long WAKTU_IDLE_PETUNJUK    = 6000;  

// 4. FITUR PEMANASAN AIR (HEATER COOL-DOWN & INITIAL BOOT)
const unsigned long WAKTU_PEMANASAN_AWAL   = 300000;  // 5 Menit saat pertama nyala
const unsigned long WAKTU_PEMANASAN        = 180000;  // 3 Menit pemanasan
const int BATAS_PENGGUNAAN_PANAS           = 2;       // REVISI: Jeda setelah 2x penggunaan
const unsigned long WAKTU_RESET_COUNTER    = 3600000; // REVISI: 1 Jam (3.600.000 md) untuk reset counter

// 5. Jeda Sistem (Dalam Milidetik)
const unsigned long JEDA_POMPA         = 200;  
const unsigned long JEDA_AUDIO_PENDEK  = 1500; 
const unsigned long JEDA_AUDIO_SEDANG  = 2000; 
const unsigned long JEDA_AUDIO_PANJANG = 3000; 
const unsigned long JEDA_STABILISASI   = 10;   
// ==========================================================

// --- Definisi Pin Input (Tombol & Limit Switch) ---
#define BTN_PANAS   13
#define BTN_NORMAL  14
#define BTN_DINGIN  33
#define BTN_HANGAT  27
#define LS_GELAS    25
#define BTN_INFO    4  

// --- Definisi Pin Output (Relay Valve & Pompa) ---
#define RLY_PANAS   19
#define RLY_NORMAL  21
#define RLY_DINGIN  22
#define RLY_POMPA   23 

// --- Logika Relay ---
#define RELAY_ON    HIGH
#define RELAY_OFF   LOW

// --- Definisi Pin Komunikasi DFPlayer ---
#define RX_PIN 16
#define TX_PIN 17

HardwareSerial mySerial(2);
DFRobotDFPlayerMini myDFPlayer;

// --- Variabel Status Sistem ---
bool lastGelas  = HIGH; 
bool lastPanas  = HIGH;
bool lastNormal = HIGH;
bool lastDingin = HIGH;
bool lastHangat = HIGH;
bool lastInfo   = HIGH; 

int pendingButton = 0;             
unsigned long pendingTime = 0;     

bool isFilled = false;               
unsigned long idleTime = 0;          
bool instructionPlayed = false;      

// --- Variabel Fitur Pemanasan & Unlock ---
int hitungPanas = 0;               
bool isHeating = false;            
unsigned long waktuMulaiPanas = 0; 
unsigned long waktuTerakhirPanas = 0; // REVISI: Menyimpan waktu penggunaan panas terakhir

bool isInitialHeating = true;      
unsigned long waktuBoot = 0;       

bool isHoldingBoth = false;        
unsigned long holdStartTime = 0;   

void setup() {
  Serial.begin(115200);
  mySerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);

  Serial.println("\n--- Sistem Dispenser Tunanetra Pintar ---");

  if (!myDFPlayer.begin(mySerial)) {
    Serial.println("DFPlayer ERROR! Cek SD Card atau Kabel TX/RX.");
    while (true);
  }
  myDFPlayer.volume(30); 

  pinMode(BTN_PANAS, INPUT_PULLUP);
  pinMode(BTN_NORMAL, INPUT_PULLUP);
  pinMode(BTN_DINGIN, INPUT_PULLUP);
  pinMode(BTN_HANGAT, INPUT_PULLUP);
  pinMode(LS_GELAS, INPUT_PULLUP);
  pinMode(BTN_INFO, INPUT_PULLUP); 

  pinMode(RLY_PANAS, OUTPUT);
  pinMode(RLY_NORMAL, OUTPUT);
  pinMode(RLY_DINGIN, OUTPUT);
  pinMode(RLY_POMPA, OUTPUT); 

  digitalWrite(RLY_PANAS, RELAY_OFF);
  digitalWrite(RLY_NORMAL, RELAY_OFF);
  digitalWrite(RLY_DINGIN, RELAY_OFF);
  digitalWrite(RLY_POMPA, RELAY_OFF); 

  waktuBoot = millis(); 

  Serial.println("-> Memutar 12.wav (Sistem Siap)");
  myDFPlayer.play(12);
  delay(JEDA_AUDIO_PANJANG); 
}

// ==========================================================
// FUNGSI PENGISIAN TUNGGAL
// ==========================================================
void tuangAir(int pinRelay, String namaSuhu, unsigned long durasiIsi) {
  Serial.println("-> Memutar 8.wav (Pengisian Dimulai)");
  myDFPlayer.play(8);
  delay(JEDA_AUDIO_PENDEK); 

  Serial.println("Membuka Valve " + namaSuhu + "...");
  digitalWrite(pinRelay, RELAY_ON); 
  delay(JEDA_POMPA);
  Serial.println("Menyalakan Pompa...");
  digitalWrite(RLY_POMPA, RELAY_ON);

  unsigned long startTime = millis();
  
  while (millis() - startTime < durasiIsi) {
    if (digitalRead(LS_GELAS) == HIGH) {
      Serial.println("AWAS! Gelas terlepas! Pengisian dihentikan darurat!");
      break; 
    }
    delay(JEDA_STABILISASI); 
  }

  Serial.println("Mematikan Pompa...");
  digitalWrite(RLY_POMPA, RELAY_OFF); 
  delay(JEDA_POMPA);
  Serial.println("Menutup Valve " + namaSuhu + "...");
  digitalWrite(pinRelay, RELAY_OFF); 

  // Catat jika air panas digunakan
  if (pinRelay == RLY_PANAS) {
    hitungPanas++;
    waktuTerakhirPanas = millis(); // REVISI: Catat waktu terakhir digunakan
    
    if (hitungPanas >= BATAS_PENGGUNAAN_PANAS) {
      isHeating = true;
      waktuMulaiPanas = millis();
      Serial.println("-> Batas 2x Panas tercapai. Sistem pemanasan 3 menit aktif.");
    }
  }
  
  Serial.println("-> Memutar 9.wav (Pengisian Selesai)");
  myDFPlayer.play(9);
  
  isFilled = true; 
  instructionPlayed = true; 
  delay(JEDA_AUDIO_SEDANG); 
}

// ==========================================================
// FUNGSI PENGISIAN CAMPURAN
// ==========================================================
void tuangAirCampur(unsigned long durasiNormal, unsigned long durasiPanas) {
  Serial.println("-> Memutar 8.wav (Pengisian Dimulai)");
  myDFPlayer.play(8);
  delay(JEDA_AUDIO_PENDEK); 

  bool isCancelled = false; 

  // --- FASE 1: AIR NORMAL ---
  Serial.println("FASE 1: Membuka Valve Normal...");
  digitalWrite(RLY_NORMAL, RELAY_ON); 
  delay(JEDA_POMPA);
  Serial.println("Menyalakan Pompa (Fase 1)...");
  digitalWrite(RLY_POMPA, RELAY_ON);
  
  unsigned long startNormal = millis();
  while (millis() - startNormal < durasiNormal) {
    if (digitalRead(LS_GELAS) == HIGH) {
      Serial.println("AWAS! Gelas terlepas di Fase 1!");
      isCancelled = true;
      break; 
    }
    delay(JEDA_STABILISASI); 
  }
  
  digitalWrite(RLY_POMPA, RELAY_OFF); 
  delay(JEDA_POMPA);
  digitalWrite(RLY_NORMAL, RELAY_OFF); 

  // --- FASE 2: AIR PANAS ---
  if (!isCancelled) {
    Serial.println("FASE 2: Membuka Valve Panas...");
    digitalWrite(RLY_PANAS, RELAY_ON); 
    delay(JEDA_POMPA);
    Serial.println("Menyalakan Pompa (Fase 2)...");
    digitalWrite(RLY_POMPA, RELAY_ON);
    
    unsigned long startPanas = millis();
    while (millis() - startPanas < durasiPanas) {
      if (digitalRead(LS_GELAS) == HIGH) {
        Serial.println("AWAS! Gelas terlepas di Fase 2!");
        break; 
      }
      delay(JEDA_STABILISASI); 
    }
    
    digitalWrite(RLY_POMPA, RELAY_OFF); 
    delay(JEDA_POMPA);
    digitalWrite(RLY_PANAS, RELAY_OFF); 

    hitungPanas++;
    waktuTerakhirPanas = millis(); // REVISI: Catat waktu terakhir digunakan
    
    if (hitungPanas >= BATAS_PENGGUNAAN_PANAS) {
      isHeating = true;
      waktuMulaiPanas = millis();
      Serial.println("-> Batas 2x Hangat tercapai. Sistem pemanasan 3 menit aktif.");
    }
  }

  Serial.println("Pengisian Air Hangat (Campuran) Selesai.");
  Serial.println("-> Memutar 9.wav (Pengisian Selesai)");
  myDFPlayer.play(9);
  
  isFilled = true; 
  instructionPlayed = true; 
  delay(JEDA_AUDIO_SEDANG); 
}

// ==========================================================

void loop() {
  bool currentGelas  = digitalRead(LS_GELAS);
  bool currentPanas  = digitalRead(BTN_PANAS);
  bool currentDingin = digitalRead(BTN_DINGIN);
  bool currentHangat = digitalRead(BTN_HANGAT);
  bool currentNormal = digitalRead(BTN_NORMAL);
  bool currentInfo   = digitalRead(BTN_INFO); 

  // --- CEK TIMER PEMANASAN AWAL (5 MENIT) ---
  if (isInitialHeating && (millis() - waktuBoot >= WAKTU_PEMANASAN_AWAL)) {
    isInitialHeating = false;
    Serial.println("-> Pemanasan awal 5 menit selesai! | Memutar 16.wav");
    myDFPlayer.play(16); 
    delay(JEDA_AUDIO_SEDANG); 
  }

  // --- CEK TIMER PEMANASAN SETELAH 2X PAKAI (3 MENIT) ---
  if (isHeating && (millis() - waktuMulaiPanas >= WAKTU_PEMANASAN)) {
    isHeating = false;
    hitungPanas = 0; 
    Serial.println("-> Pemanasan 3 menit selesai! | Memutar 16.wav");
    myDFPlayer.play(16); 
    delay(JEDA_AUDIO_SEDANG); 
  }

  // --- REVISI: CEK TIMER RESET COUNTER OTOMATIS (1 JAM TIDAK DIPAKAI) ---
  if (hitungPanas > 0 && !isHeating && (millis() - waktuTerakhirPanas >= WAKTU_RESET_COUNTER)) {
    hitungPanas = 0;
    Serial.println("-> 1 Jam berlalu tanpa penggunaan air panas. Counter direset ke 0.");
  }

  // --- TOMBOL INFO (PLAY 15.WAV) ---
  if (currentInfo == LOW && lastInfo == HIGH) {
    Serial.println("-> Tombol Info Ditekan | Memutar 15.wav");
    myDFPlayer.play(15);
    delay(500); 
  }
  lastInfo = currentInfo;

  // --- TRIGGER GELAS DILETAKKAN / DIAMBIL ---
  if (currentGelas == LOW && lastGelas == HIGH) {
    Serial.println("-> Gelas Diletakkan | Memutar 5.wav");
    myDFPlayer.play(5);
    idleTime = millis(); 
    instructionPlayed = false;
    isFilled = false;
    delay(JEDA_AUDIO_PENDEK); 
  } 
  else if (currentGelas == HIGH && lastGelas == LOW) {
    Serial.println("-> Gelas Diambil | Memutar 6.wav");
    myDFPlayer.play(6);
    pendingButton = 0; 
    isFilled = false;
    instructionPlayed = false;
    delay(JEDA_AUDIO_PENDEK);
  }
  lastGelas = currentGelas;

  // --- DETEKSI TOMBOL AIR DITEKAN ---
  int buttonTriggered = 0;
  if (currentPanas == LOW && lastPanas == HIGH) buttonTriggered = BTN_PANAS;
  else if (currentDingin == LOW && lastDingin == HIGH) buttonTriggered = BTN_DINGIN;
  else if (currentHangat == LOW && lastHangat == HIGH) buttonTriggered = BTN_HANGAT;
  else if (currentNormal == LOW && lastNormal == HIGH) buttonTriggered = BTN_NORMAL;

  // --- UNLOCK RAHASIA (TAHAN PANAS + DINGIN 5 DETIK) ---
  if (currentPanas == LOW && currentDingin == LOW) {
    buttonTriggered = 0; 

    if (!isHoldingBoth) {
      isHoldingBoth = true;
      holdStartTime = millis();
    } else if (millis() - holdStartTime >= 5000) {
      if (isInitialHeating || isHeating) {
        Serial.println("-> UNLOCK BERHASIL: Pemanasan dilewati! | Memutar 15.wav");
        isInitialHeating = false;
        isHeating = false;
        hitungPanas = 0; 
        pendingButton = 0; 
        myDFPlayer.play(15); 
        delay(JEDA_AUDIO_SEDANG);
      }
      holdStartTime = millis(); 
    }
  } else {
    isHoldingBoth = false;
  }

  lastPanas  = currentPanas;
  lastDingin = currentDingin;
  lastHangat = currentHangat;
  lastNormal = currentNormal;

  // --- BLOKIR TOMBOL PANAS & HANGAT SAAT PEMANASAN ---
  if (buttonTriggered == BTN_PANAS || buttonTriggered == BTN_HANGAT) {
    if (isInitialHeating || isHeating) {
      Serial.println("-> ERROR: Air sedang dipanaskan (Terkunci) | Memutar 17.wav");
      myDFPlayer.play(17); 
      buttonTriggered = 0;  
      delay(JEDA_AUDIO_SEDANG);
    }
  }

  // --- LOGIKA UTAMA PENGISIAN AIR ---
  if (buttonTriggered != 0) {
    idleTime = millis(); 
    
    if (currentGelas == LOW) {
      if (isFilled) {
        Serial.println("-> ERROR: Gelas sudah penuh! | Memutar 13.wav");
        myDFPlayer.play(13); 
        delay(JEDA_AUDIO_SEDANG);
      }
      else {
        // KONFIRMASI (TOMBOL KEDUA KALI)
        if (pendingButton == buttonTriggered) {
          pendingButton = 0; 
          
          if (buttonTriggered == BTN_PANAS)       
            tuangAir(RLY_PANAS, "Air Panas", WAKTU_ISI_PANAS);
          else if (buttonTriggered == BTN_DINGIN) 
            tuangAir(RLY_DINGIN, "Air Dingin", WAKTU_ISI_DINGIN);
          else if (buttonTriggered == BTN_NORMAL) 
            tuangAir(RLY_NORMAL, "Air Normal", WAKTU_ISI_NORMAL);
          else if (buttonTriggered == BTN_HANGAT) 
            tuangAirCampur(WAKTU_HANGAT_NORMAL, WAKTU_HANGAT_PANAS); 
        } 
        // TEKANAN PERTAMA
        else {
          pendingButton = buttonTriggered;
          
          if (buttonTriggered == BTN_PANAS) {
            Serial.println("-> Pilihan: Air Panas | Memutar 1.wav");
            myDFPlayer.play(1);
          } else if (buttonTriggered == BTN_DINGIN) {
            Serial.println("-> Pilihan: Air Dingin | Memutar 2.wav");
            myDFPlayer.play(2);
          } else if (buttonTriggered == BTN_HANGAT) {
            Serial.println("-> Pilihan: Air Hangat | Memutar 3.wav");
            myDFPlayer.play(3);
          } else if (buttonTriggered == BTN_NORMAL) {
            Serial.println("-> Pilihan: Air Normal | Memutar 4.wav");
            myDFPlayer.play(4);
          }

          delay(JEDA_AUDIO_PENDEK); 
          Serial.println("-> Instruksi Konfirmasi | Memutar 10.wav");
          myDFPlayer.play(10);
          pendingTime = millis(); 
        }
      }
    } 
    else {
      Serial.println("-> ERROR: Tombol ditekan tapi gelas tidak ada! | Memutar 7.wav");
      myDFPlayer.play(7); 
      delay(JEDA_AUDIO_SEDANG);
    }
  }

  // --- AUTO-INSTRUCTION JIKA IDLE ---
  if (currentGelas == LOW && !isFilled && !instructionPlayed && pendingButton == 0) {
    if (millis() - idleTime > WAKTU_IDLE_PETUNJUK) { 
      Serial.println("-> Memutar Petunjuk Penggunaan (14.wav)");
      myDFPlayer.play(14);
      instructionPlayed = true; 
    }
  }

  // --- CEK BATAS WAKTU KONFIRMASI ---
  if (pendingButton != 0 && (millis() - pendingTime > BATAS_WAKTU_KONFIRMASI)) {
    Serial.println("-> Waktu habis | Memutar 11.wav");
    pendingButton = 0; 
    myDFPlayer.play(11); 
  }
}