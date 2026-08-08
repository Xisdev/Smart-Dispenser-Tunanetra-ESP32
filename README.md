# Smart Dispenser untuk Penyandang Disabilitas Tunanetra Berbasis ESP32

Proyek ini adalah sistem kendali dispenser pengisian air otomatis yang dirancang khusus untuk membantu penyandang disabilitas tunanetra. Menggunakan mikrokontroler ESP32, sistem ini memberikan panduan audio (suara) interaktif untuk setiap aksi, fitur pengaman untuk mencegah air tumpah atau luka bakar, dan logika pencampuran air presisi tinggi.

##  Fitur Utama

- **Panduan Audio Interaktif:** Menggunakan modul DFPlayer Mini untuk membacakan instruksi, konfirmasi pilihan, dan status dispenser.
- **Sistem Konfirmasi Ganda (Double-Check):** Pengguna harus menekan tombol yang sama dua kali dalam 10 detik untuk mengonfirmasi pilihan suhu air, mencegah insiden salah tekan.
- **Pencampuran Suhu Cerdas (Azas Black):** Air hangat dibuat dengan mencampurkan air normal dan air panas secara bergiliran untuk mencapai suhu optimal tanpa mengorbankan stabilitas kelistrikan.
- **Keamanan *Anti-Double Fill*:** Mencegah sistem mengisi ulang gelas yang sudah penuh jika gelas belum diambil dari dudukannya.
- **Proteksi Cangkir Terlepas:** Jika gelas tersenggol atau ditarik sebelum pengisian selesai, sistem langsung menutup katup air (valve) dan mematikan pompa dalam hitungan milidetik.
- **Sistem *Cool-down* Pemanas (Heater):** Setelah 2 kali penggunaan air panas/hangat, sistem akan otomatis terkunci selama 3 menit untuk memulihkan suhu pemanas. 
- **Pemanasan Awal (Initial Boot):** Sistem akan mengunci tombol air panas selama 5 menit saat alat pertama kali dicolokkan ke listrik.
- **Kombinasi *Unlock* Rahasia:** Tahan tombol Panas + Dingin selama 5 detik untuk melewati (bypass) waktu tunggu pemanasan (khusus untuk teknisi/pengujian).
- **Auto-Instruction (Idle):** Jika pengguna meletakkan gelas tetapi diam selama 6 detik, sistem otomatis membacakan petunjuk penggunaan alat.
- **Proteksi *Water Hammer*:** Jeda 200ms antara pembukaan valve dan penyalaan pompa untuk menjaga keawetan selang dan mesin pompa.

##  Kebutuhan Perangkat Keras (Hardware)

1. Mikrokontroler ESP32
2. Modul DFPlayer Mini + MicroSD Card (Format FAT32) + Speaker 3W
3. Modul Relay 4 Channel (Active-HIGH)
4. 3x Solenoid Valve 12V (Panas, Normal, Dingin)
5. 1x Pompa Air DC 12V
6. 1x Limit Switch (Pendeteksi Gelas)
7. 5x Push Button (Panas, Normal, Dingin, Hangat, Info)
8. Power Supply (PSU) dengan output 12V dan 5V independen

##  Pemetaan Pin (Pinout) ESP32

| Komponen | Pin ESP32 | Keterangan |
| :--- | :--- | :--- |
| **Tombol Panas** | GPIO 13 | Input (Pull-up ke GND) |
| **Tombol Normal** | GPIO 14 | Input (Pull-up ke GND) |
| **Tombol Dingin** | GPIO 33 | Input (Pull-up ke GND) |
| **Tombol Hangat** | GPIO 27 | Input (Pull-up ke GND) |
| **Tombol Info** | GPIO 4 | Input (Pull-up ke GND) |
| **Limit Switch** | GPIO 25 | Input (Pull-up ke GND) |
| **Relay Valve Panas** | GPIO 19 | Output |
| **Relay Valve Normal**| GPIO 21 | Output |
| **Relay Valve Dingin**| GPIO 22 | Output |
| **Relay Pompa Air** | GPIO 23 | Output |
| **DFPlayer TX** | GPIO 16 (RX2) | UART Serial2 |
| **DFPlayer RX** | GPIO 17 (TX2) | UART Serial2 |

##  Direktori Audio SD Card

- `001.wav` - Pilihan Air Panas
- `002.wav` - Pilihan Air Dingin
- `003.wav` - Pilihan Air Hangat
- `004.wav` - Pilihan Air Normal
- `005.wav` - Gelas diletakkan
- `006.wav` - Gelas diambil
- `007.wav` - Error: Gelas tidak terdeteksi
- `008.wav` - Pengisian dimulai
- `009.wav` - Pengisian selesai
- `010.wav` - Instruksi tekan sekali lagi untuk konfirmasi
- `011.wav` - Waktu konfirmasi habis, pilihan dibatalkan
- `012.wav` - Sistem dispenser pintar siap digunakan (Boot audio)
- `013.wav` - Error: Gelas sudah penuh, silakan diambil
- `014.wav` - Petunjuk penggunaan idle (Bentuk tombol segitiga, bulat, dst)
- `015.wav` - Audio info ekstra / Suara Unlock Berhasil
- `016.wav` - Pemanasan selesai, air panas siap digunakan
- `017.wav` - Air sedang dipanaskan, mohon tunggu 3 menit
4. Sesuaikan nilai di bagian `PENGATURAN WAKTU & DURASI` pada kode jika Anda ingin mempercepat/memperlambat waktu pengisian (azaz black pencampuran).
5. Unggah (Upload) kode ke board ESP32 Anda.
