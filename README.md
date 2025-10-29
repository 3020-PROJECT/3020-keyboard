# ⌨️ 3020-Keyboard

**3020-Keyboard** is a small C program that connects a **keyboard** to both a **Nano microcontroller** and an **ESP-VGA-Displayer**, allowing typed characters to appear live on a VGA display — while also being sent over serial.

It acts as a simple **bridge between hardware input and network display**, enabling a complete interactive terminal setup using only a TL-MR3020 router, an ESP32, and an Arduino Nano.

---

## 🧠 Overview

This program:
- Reads keypresses from `/dev/input/event0`
- Forwards them to:
  - The **Nano** via `/dev/ttyATH0`
  - The **ESP-VGA-Displayer** via a TCP socket
- Displays text in real time on the VGA screen
- Sends an initial identification message (`/identify 3020-ROUTER`) to the ESP32
- Wraps all text sent to the ESP32 with the `/print` command

> **Prerequisite:** This project assumes you have followed **Part 2** of the *WhyNot* TL-MR3020 hardware hacking series (the hardware modifications required to expose `/dev/ttyATH0` and to attach a keyboard). See the WhyNot TL-MR3020 guide for details.

---

## ⚙️ Features
- 🖥️ **Live text mirroring** between keyboard, Nano, and VGA display  
- 🔌 **Dual output**: serial + TCP/IP  
- ⚡ **Direct integration** with [ESP-VGA-Displayer](https://github.com/3020-PROJECT/esp-VGA-Displayer)  
- 🔠 **Simple keymap** — converts raw Linux keycodes into printable ASCII  
- 🧩 **Modular design** — each I/O (serial, keyboard, network) is handled independently  

---

## 🧱 Build & Run

### 🔨 Build
Compile with `gcc`:
```bash
gcc main.c -o 3020-keyboard
```

### ▶️ Run
Usage:
```bash
./3020-keyboard [display-ip] [display-port]
```

Example:
```bash
./3020-keyboard 192.168.1.150 1337
```

This will:
1. Connect to the ESP-VGA-Displayer at `192.168.1.150:1337`
2. Identify as `3020-ROUTER`
3. Open `/dev/ttyATH0` (Nano serial)
4. Read keyboard input from `/dev/input/event0`
5. Send keystrokes to both the Nano and the ESP32 display

---

## 📡 Protocol Example

Characters typed on the keyboard are automatically wrapped for the ESP display:

```
/print hello world
```

If you press Enter, the display automatically starts a new `/print` line.

---

## 🧰 System Requirements
- A **TL-MR3020 router** running **OpenWrt**  
- Access to:
  - `/dev/ttyATH0` — connected to an Arduino Nano  
  - `/dev/input/event0` — connected to a USB or internal keyboard  
- Network access to the **ESP-VGA-Displayer**  
- GCC or compatible C compiler  

**Note:** See the WhyNot TL-MR3020 hardware hacking guide (Part 2) for the required hardware modifications to expose the serial and input devices.

---

## 🧩 Implementation Notes
- The serial port `/dev/ttyATH0` is configured for:
  - **9600 baud**, **8N1**, **raw mode**
- Uses `inet_pton()` + `connect()` for TCP setup  
- Reads input events from Linux’s **evdev** subsystem (`/dev/input/event0`)  
- Sends initial handshake:
  ```
  /print Hello From 3020-Keyboard!
  /identify 3020-ROUTER
  ```
- Currently blocking I/O — `poll()` support planned for non-blocking read/write  

---

## 🚧 TODO
- [X] Use `poll()` to handle I/O without blocking.
- [ ] Implement a clean exit signal (e.g. Ctrl+C handler).
- [ ] Optional visual feedback or command mode.
- [ ] Allow toggling print mode on/off via a key combo.

---

## 🤝 Related Projects
- 🖥️ **[ESP-VGA-Displayer](https://github.com/0rayn/esp-vga-displayer)** — the TCP display server for this keyboard  
- 🖥️ **[3020-NANO](https://github.com/3020-PROJECT/3020-NANO)** — receives serial commands and executes actions 
---

## 🧩 License
MIT License — free to use, modify, and share.

---
