# 🔥 Sistem CCTV Surveillance SA-MP v2.0

Sistem CCTV surveillance profesional untuk server SA-MP dengan fitur canggih dan kompatibilitas penuh.

## ✨ Fitur Utama

- **Kamera CCTV profesional** dengan rotasi 360° penuh
- **Sistem auto-rotasi** dengan perubahan arah acak setiap 3 detik
- **Kontrol kamera manual** menggunakan tombol WASD + tombol sentuh mobile
- **Sistem multi-owner** mendukung kepemilikan Admin/Police/Faction/House
- **Integrasi database MySQL** untuk penyimpanan data permanen
- **Interface HUD profesional** dengan tampilan tanggal/waktu real-time
- **Sistem akses tombol Y** untuk aktivasi CCTV yang mudah
- **Kontrol akses berbasis permission** dengan pembatasan yang dapat disesuaikan
- **Viewing kamera real-time** dengan kontrol arah yang halus
- **Mode edit** untuk reposisi CCTV dengan mudah
- **Kompatibilitas gamemode universal** dengan konfigurasi sederhana

## 📋 Persyaratan

- SA-MP Server 0.3.7 atau lebih tinggi
- MySQL plugin (mysql-plugin R41-4 atau lebih tinggi direkomendasikan)
- Streamer plugin v2.9.4 atau lebih tinggi
- YSI library (untuk YCMD dan hooks)
- sscanf plugin v2.8.3 atau lebih tinggi

## 🚀 Cara Instalasi

### 1. **Download dan Include Sistem**
```pawn
#include "cctv_system"
```
**Lokasi:** Letakkan di bagian atas gamemode Anda, setelah include lainnya.

### 2. **Buat Tabel Database**
- Import file `cctv_database.sql` ke database MySQL Anda
- Atau jalankan perintah SQL secara manual

### 3. **Konfigurasi Sistem (WAJIB DIUBAH)**

Buka file `cctv_system.inc` dan ubah bagian konfigurasi berikut sesuai gamemode Anda:

#### **A. Koneksi MySQL**
```pawn
// UBAH 'g_SQL' dengan variabel koneksi MySQL gamemode Anda
#define CCTV_MYSQL_CONNECTION g_SQL
```
**Contoh alternatif:**
- `#define CCTV_MYSQL_CONNECTION MySQL_Handle`
- `#define CCTV_MYSQL_CONNECTION dbHandle`
- `#define CCTV_MYSQL_CONNECTION mysql_connection`

#### **B. Sistem Admin**
```pawn
// UBAH dengan sistem admin gamemode Anda
#define CCTV_ADMIN_CHECK(%0) (AccountData[%0][pAdmin] >= 3)
```
**Contoh alternatif:**
- `#define CCTV_ADMIN_CHECK(%0) (PlayerInfo[%0][pAdmin] >= 3)`
- `#define CCTV_ADMIN_CHECK(%0) (pData[%0][pAdmin] >= 3)`
- `#define CCTV_ADMIN_CHECK(%0) IsPlayerAdmin(%0)` (untuk RCON admin)

#### **C. Sistem Notifikasi**
```pawn
// UBAH dengan fungsi notifikasi gamemode Anda
#define CCTV_NOTIFICATION(%0,%1,%2) ShowTDN(%0, NOTIFICATION_SUCCESS, %2)
#define CCTV_ERROR_MSG(%0,%1) ShowTDN(%0, NOTIFICATION_ERROR, %1)
#define CCTV_USAGE_MSG(%0,%1) SUM(%0, %1)
#define CCTV_PERMISSION_ERROR(%0) PermissionError(%0)
```
**Contoh alternatif:**
- `#define CCTV_NOTIFICATION(%0,%1,%2) SendClientMessage(%0, 0x00FF00FF, %2)`
- `#define CCTV_ERROR_MSG(%0,%1) SendClientMessage(%0, 0xFF0000FF, %1)`
- `#define CCTV_USAGE_MSG(%0,%1) SendClientMessage(%0, 0xFFFF00FF, %1)`

#### **D. Sistem HBE (Opsional)**
```pawn
// JIKA gamemode Anda TIDAK memiliki sistem HBE, HAPUS atau COMMENT kedua baris ini:
#define CCTV_HIDE_HBE(%0) HideHBETD(%0)
#define CCTV_SHOW_HBE(%0) ShowHBETD(%0)

// Atau ganti dengan:
// #define CCTV_HIDE_HBE(%0) 
// #define CCTV_SHOW_HBE(%0)
```

#### **E. Sistem Faction/House (Opsional)**
```pawn
// JIKA gamemode Anda memiliki sistem faction/house, ubah sesuai fungsi Anda:
#define CCTV_GET_PLAYER_FACTION(%0) GetPlayerFactionID(%0)  // Ganti dengan fungsi faction Anda
#define CCTV_GET_PLAYER_HOUSE(%0) GetPlayerHouseID(%0)      // Ganti dengan fungsi house Anda

// JIKA TIDAK ada sistem faction/house, biarkan seperti ini:
#define CCTV_GET_PLAYER_FACTION(%0) (0)
#define CCTV_GET_PLAYER_HOUSE(%0) (0)
```

### 4. **Inisialisasi Sistem**
```pawn
public OnGameModeInit()
{
    // Kode gamemode Anda yang lain...
    
    CCTV_Load(); // TAMBAHKAN baris ini untuk load data CCTV dari database
    
    return 1;
}
```
**Lokasi:** Di dalam callback `OnGameModeInit()` gamemode Anda.

### 5. **Compile dan Test**
- Compile gamemode Anda
- Start server
- Gunakan command `/addcctv` untuk membuat CCTV pertama

## 🎮 Commands (Perintah)

### Commands Admin
- `/addcctv [ownertype] [ownerid] [nama]` - Buat kamera CCTV baru
- `/delcctv [cctvid]` - Hapus kamera CCTV
- `/cctvlist` - Lihat daftar semua kamera CCTV
- `/editposcctv [cctvid]` - Edit posisi CCTV
- `/togglecctv [cctvid]` - Aktifkan/nonaktifkan CCTV
- `/gotocctv [cctvid]` - Teleport ke lokasi CCTV

### Commands Player
- `/cctvcek` - Cek kamera CCTV yang dapat diakses di sekitar
- `/stopcctv` - Berhenti melihat CCTV

### Tipe Owner
- `0` = Admin saja
- `1` = Faction Police
- `2` = Faction tertentu (butuh OwnerID)
- `3` = Pemilik house (butuh OwnerID)

## 🎯 Kontrol

### Kontrol Keyboard
- **Y** - Akses kamera CCTV terdekat
- **W** - Lihat ke atas
- **S** - Lihat ke bawah
- **A** - Putar ke kiri
- **D** - Putar ke kanan
- **Enter** - Keluar dari viewing CCTV

### Kontrol Mobile/Touch
- **↑** - Lihat ke atas
- **↓** - Lihat ke bawah
- **←** - Putar ke kiri
- **→** - Putar ke kanan
- **EXIT** - Keluar dari viewing CCTV

## 📊 Struktur Database

Sistem membutuhkan tabel `cctv_data` dengan struktur berikut:

| Field | Type | Keterangan |
|-------|------|------------|
| ID | int(11) | ID unik CCTV |
| PosX | float | Koordinat X |
| PosY | float | Koordinat Y |
| PosZ | float | Koordinat Z |
| Angle | float | Sudut menghadap (0-360°) |
| World | int(11) | ID virtual world |
| Interior | int(11) | ID interior |
| OwnerType | int(11) | Tipe owner (0-3) |
| OwnerID | int(11) | ID Owner (Faction/House) |
| Name | varchar(64) | Nama tampilan CCTV |
| Active | int(11) | Status (0/1) |

## 🔧 Kustomisasi

### Mengatur Pengaturan

Anda dapat memodifikasi konstanta berikut dalam sistem:

```pawn
#define MAX_CCTV 500                        // Maksimal kamera CCTV
#define CCTV_VIEW_DISTANCE 50.0             // Jarak pandang kamera
#define CCTV_ROTATION_SPEED 2.0             // Kecepatan rotasi manual
#define CCTV_AUTO_ROTATION_SPEED 3.0        // Kecepatan rotasi otomatis
#define CCTV_AUTO_ROTATION_INTERVAL 3000    // Interval rotasi otomatis (ms)
```

## 🐛 Troubleshooting

### Masalah Umum

1. **CCTV tidak loading:**
   - Cek koneksi MySQL
   - Pastikan tabel database sudah dibuat
   - Pastikan `CCTV_Load()` dipanggil di `OnGameModeInit`

2. **Commands tidak bekerja:**
   - Pastikan integrasi sistem admin sudah benar
   - Cek define `CCTV_ADMIN_CHECK`
   - Pastikan YSI library sudah di-include

3. **Error permission:**
   - Update define `CCTV_PERMISSION_ERROR`
   - Cek integrasi sistem notifikasi

4. **Kamera tidak muncul:**
   - Pastikan Streamer plugin sudah di-load
   - Cek pengaturan virtual world/interior
   - Pastikan CCTV sudah diaktifkan

## 📝 Contoh Konfigurasi Lengkap

Berikut contoh konfigurasi untuk gamemode populer:

### Untuk Gamemode dengan sistem seperti NGRP/LSRP:
```pawn
#define CCTV_MYSQL_CONNECTION g_SQL
#define CCTV_ADMIN_CHECK(%0) (AccountData[%0][pAdmin] >= 3)
#define CCTV_NOTIFICATION(%0,%1,%2) ShowTDN(%0, NOTIFICATION_SUCCESS, %2)
#define CCTV_ERROR_MSG(%0,%1) ShowTDN(%0, NOTIFICATION_ERROR, %1)
#define CCTV_USAGE_MSG(%0,%1) SUM(%0, %1)
#define CCTV_PERMISSION_ERROR(%0) PermissionError(%0)
#define CCTV_HIDE_HBE(%0) HideHBETD(%0)
#define CCTV_SHOW_HBE(%0) ShowHBETD(%0)
```

### Untuk Gamemode sederhana:
```pawn
#define CCTV_MYSQL_CONNECTION mysql_handle
#define CCTV_ADMIN_CHECK(%0) IsPlayerAdmin(%0)
#define CCTV_NOTIFICATION(%0,%1,%2) SendClientMessage(%0, 0x00FF00FF, %2)
#define CCTV_ERROR_MSG(%0,%1) SendClientMessage(%0, 0xFF0000FF, %1)
#define CCTV_USAGE_MSG(%0,%1) SendClientMessage(%0, 0xFFFF00FF, %1)
#define CCTV_PERMISSION_ERROR(%0) SendClientMessage(%0, 0xFF0000FF, "ERROR: No permission!")
// Hapus atau comment HBE defines jika tidak ada
```

## 📁 File yang Disertakan

- `cctv_system.inc` - File sistem utama
- `cctv_database.sql` - Struktur database

## 👨‍💻 Author

**LeoXD SAMP++**

## 🤝 Contributing

Kontribusi, issues, dan feature requests sangat diterima!

## ⭐ Support

Jika project ini membantu, berikan ⭐ di GitHub!

---

**Dibuat dengan ❤️ untuk komunitas SA-MP Indonesia**
