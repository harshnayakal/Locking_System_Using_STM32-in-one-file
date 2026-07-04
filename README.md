# 🔐 Two-Factor Authentication (2FA) Locker System using STM32F103C8T6

## 📌 Overview

The **Two-Factor Authentication (2FA) Locker System** is an embedded security application built using the **STM32F103C8T6 (Blue Pill)** microcontroller. The system provides enhanced access control by combining a **4-digit password** with a **randomly generated 4-digit One-Time Password (OTP)**.

Users enter the password through a **4×4 matrix keypad**. Upon successful verification, the system generates a unique OTP and displays it on a **16×2 LCD**. Access is granted only after the correct OTP is entered. To improve security, the system locks for **100 seconds** after three consecutive incorrect password attempts. The project is implemented entirely in **Embedded C** using **register-level programming** in **Keil uVision**, without using HAL or external libraries.

---

# ✨ Features

* 🔐 Password + OTP based Two-Factor Authentication
* 🔢 4-digit password verification
* 🎲 Pseudo-random 4-digit OTP generation
* ⌨️ 4×4 Matrix Keypad input
* 📺 16×2 LCD user interface
* 🔒 Three-attempt security lockout
* ⏳ 100-second lockout countdown
* 🔔 Buzzer feedback for keypresses and alerts
* 💡 LED lock/unlock indication
* ⚡ Register-level STM32 programming
* 🧩 Single-file modular firmware implementation

---

# 🛠 Hardware Components

* STM32F103C8T6 (Blue Pill)
* 16×2 LCD (8-bit Mode)
* 4×4 Matrix Keypad
* Active Buzzer
* 4 LEDs
* Push Button (Start Switch)
* Breadboard
* Jumper Wires
* 5V Power Supply

---

# 📌 Pin Configuration

| Component        | STM32 Pin |
| ---------------- | --------- |
| LCD RS           | PB0       |
| LCD EN           | PB1       |
| LCD Data (D0–D7) | PB8–PB15  |
| Keypad Rows      | PA0–PA3   |
| Keypad Columns   | PA4–PA7   |
| Buzzer           | PB2       |
| LED1             | PA8       |
| LED2             | PA9       |
| LED3             | PA10      |
| LED4             | PA11      |
| Start Switch     | PB7       |

---

# 💻 Software & Tools

* Keil uVision 5
* STM32 CMSIS Device Headers
* Embedded C
* Register-Level Programming

---

# 📂 Project Structure

```text
Project/
│── main.c
│── startup_stm32f10x_md.s
│── system_stm32f10x.c
│── system_stm32f10x.h
│── stm32f10x.h
```

---

# ⚙️ Working Principle

1. The system displays a welcome message and waits for the **Start Switch**.
2. The user enters a **4-digit password** using the keypad.
3. Incorrect passwords increase the failed-attempt counter.
4. After three incorrect attempts, the system enters a **100-second lockout**.
5. When the correct password is entered, a **random 4-digit OTP** is generated.
6. The generated OTP is displayed on the LCD.
7. The user enters the OTP using the keypad.
8. If the OTP is correct:

   * Access is granted.
   * LEDs perform an unlock sequence.
   * The lock remains open for a few seconds.
9. After the timeout, LEDs return to the locked state and the system waits for the next user.

---

# 🔒 Security Features

* Two-Factor Authentication (Password + OTP)
* Three incorrect password attempt limit
* Automatic 100-second lockout
* Password masking using '*'
* Random OTP generation
* Keypress buzzer feedback
* LED status indication

---

# 📸 Demonstration

* Password Entry
* OTP Generation
* OTP Verification
* Lock Opening Sequence
* Lock Closing Sequence
* Security Lockout Countdown

*(Add project photos, Proteus simulation, or demonstration video here.)*

---

# 🚀 Future Enhancements

* GSM-based OTP delivery
* ESP8266/ESP32 IoT integration
* Fingerprint authentication
* RFID access control
* Servo motor controlled locker
* EEPROM password storage
* RTC-based access logging
* Mobile application integration

---

# 📚 Learning Outcomes

* STM32 GPIO configuration
* Register-level Embedded C programming
* Matrix keypad interfacing
* LCD interfacing in 8-bit mode
* OTP generation techniques
* Embedded security implementation
* State-machine based firmware design
* User interface development for embedded systems

---

# 👨‍💻 Author

**Harsh Nayakal**

Final-Year Mechatronics Engineering Student

**Areas of Interest**

* Embedded Systems
* Internet of Things (IoT)
* STM32 Development
* PCB Design
* Robotics
* Industrial Automation

---

# 📄 License

This project is licensed under the **MIT License**.

⭐ If you found this project helpful, consider giving the repository a **Star** on GitHub.
