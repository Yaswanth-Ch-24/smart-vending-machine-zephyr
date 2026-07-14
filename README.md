# Automated Smart Vending Machine using Zephyr RTOS

> **Embedded Systems | RTOS | 2026**  
> A Zephyr RTOS-powered smart vending machine with keypad input, servo actuation, ultrasonic delivery verification, and Wi-Fi inventory management.

---

## 🔍 Project Overview

A **fully automated smart vending machine** built on Zephyr RTOS — one of the most industry-relevant embedded OS platforms used in IoT products worldwide. Features matrix keypad product selection, PWM-driven servo actuation for dispensing, closed-loop ultrasonic verification of delivery, and Wi-Fi connectivity for remote inventory management.

---

## ⚡ Features

- ⌨️ **Matrix keypad** — product selection interface
- 🔧 **PWM servo actuation** — precise product dispensing mechanism
- 📡 **Ultrasonic delivery verification** — confirms item dispensed successfully
- 📶 **Wi-Fi connectivity** — remote inventory tracking
- 🗃️ **Remote inventory management** — real-time stock monitoring
- ⚙️ **Zephyr RTOS** — multi-threaded task scheduling for reliable operation
- 🔔 **Low-stock alerts** — sent over Wi-Fi when inventory is critical

---

## 🛠️ Tech Stack

| Component | Details |
|---|---|
| RTOS | Zephyr RTOS |
| Microcontroller | nRF52 / STM32 (Zephyr-compatible) |
| Input | 4×4 Matrix Keypad |
| Dispense Mechanism | SG90/MG996R Servo (PWM) |
| Verification | HC-SR04 Ultrasonic Sensor |
| Connectivity | ESP8266 Wi-Fi (UART) / nRF Wi-Fi |
| Language | C (Zephyr APIs) |
| IDE | VS Code + Zephyr SDK |

---

## 🏗️ RTOS Task Architecture

```
┌─────────────────────────────────────────────────┐
│              Zephyr RTOS Kernel                 │
│                                                 │
│  [Thread 1] keypad_task    → Read user input    │
│  [Thread 2] dispense_task  → PWM servo control  │
│  [Thread 3] verify_task    → Ultrasonic check   │
│  [Thread 4] wifi_task      → Inventory sync     │
│  [Thread 5] display_task   → UI feedback        │
│                                                 │
│  Message Queues + Semaphores for sync           │
└─────────────────────────────────────────────────┘
```

---

## 📁 Repository Structure

```
smart-vending-machine/
├── src/
│   ├── main.c                  # App entry, RTOS init
│   ├── keypad.c                # Matrix keypad scanning
│   ├── servo_dispense.c        # PWM servo control
│   ├── ultrasonic_verify.c     # Delivery verification
│   ├── wifi_inventory.c        # Wi-Fi + inventory sync
│   └── display.c               # User feedback (LCD/OLED)
├── include/
│   └── *.h
├── boards/                     # Board-specific configs
├── CMakeLists.txt              # Zephyr build system
├── prj.conf                    # Zephyr config options
├── docs/
│   ├── system_flowchart.png
│   └── project_report.pdf
└── README.md
```

---

## 🚀 Build & Flash (Zephyr)

```bash
# Install Zephyr SDK first: https://docs.zephyrproject.org/

git clone https://github.com/Yaswanth-Ch-24/smart-vending-machine.git
cd smart-vending-machine

# Initialize Zephyr workspace
west init -l .
west update

# Build for your board (e.g., nrf52dk_nrf52832)
west build -b nrf52dk_nrf52832

# Flash
west flash
```

---

## 🔄 Operating Flow

```
User presses key → Select product → [Valid?]
                                      │ Yes
                                   Servo rotates → Item dispensed
                                      │
                              Ultrasonic check → [Item detected?]
                                      │ Yes          │ No
                                 Log success    Retry / Alert
                                      │
                               Wi-Fi sync → Update inventory
```

---

## 💡 Why Zephyr RTOS?

Zephyr is used in **real commercial IoT products** by companies like Nordic Semiconductor, Intel, and Google. Using it for this project demonstrates production-grade embedded skills beyond hobby Arduino development.

---

## 👤 Author

**Chlliboina Yaswanth**  
B.Tech Electrical & Electronics Engineering  
Dr. Lankapalli Bullayya College of Engineering, Visakhapatnam  
📧 yaswanth2452005@gmail.com  
🔗 [LinkedIn](https://www.linkedin.com/in/yaswanth-chlliboina/)
