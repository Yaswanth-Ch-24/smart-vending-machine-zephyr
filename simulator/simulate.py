"""
Smart Vending Machine — Zephyr RTOS
PC Terminal Simulator — No hardware needed

Simulates the exact UART output of the STM32 F446RE + Zephyr RTOS system:
  - 5 concurrent threads (keypad, dispense, verify, wifi, display)
  - Product selection via keyboard (type product ID + Enter)
  - Out-of-stock handling
  - Delivery verification
  - Wi-Fi inventory sync

Run: python simulate.py
Type a product ID (A1, A2, A3, B1, B2) and press Enter to "buy" it.
Type 'q' to quit.
"""

import time
import sys
import threading

GREEN  = "\033[92m"
RED    = "\033[91m"
YELLOW = "\033[93m"
CYAN   = "\033[96m"
BLUE   = "\033[94m"
WHITE  = "\033[97m"
DIM    = "\033[2m"
BOLD   = "\033[1m"
MAGENTA= "\033[95m"
RESET  = "\033[0m"

# ── Inventory ─────────────────────────────────────
inventory = {
    "A1": {"name": "Chips",    "price": 20, "stock": 3},
    "A2": {"name": "Water",    "price": 15, "stock": 2},
    "A3": {"name": "Soda",     "price": 25, "stock": 1},
    "B1": {"name": "Biscuits", "price": 10, "stock": 2},
    "B2": {"name": "Juice",    "price": 30, "stock": 0},
}

lock = threading.Lock()

def banner():
    print(f"\n{CYAN}{'='*47}{RESET}")
    print(f"{BOLD}{WHITE}  Smart Vending Machine — Zephyr RTOS{RESET}")
    print(f"{BOLD}{WHITE}  STM32 F446RE | Yaswanth Chlliboina{RESET}")
    print(f"{CYAN}{'='*47}{RESET}")

def log_thread(name, color, msg):
    print(f"{DIM}[{name:>8}]{RESET} {color}{msg}{RESET}")

def print_inventory():
    print(f"\n{CYAN}  Inventory:{RESET}")
    for pid, p in inventory.items():
        stock_color = GREEN if p["stock"] > 1 else (YELLOW if p["stock"] == 1 else RED)
        print(f"  {BOLD}{pid}{RESET}  {p['name']:<12} Rs.{p['price']:>2}  "
              f"Stock: {stock_color}{p['stock']}{RESET}")
    print()

def lcd_display(line1, line2=""):
    print(f"\n  {BLUE}┌──────────────────┐{RESET}")
    print(f"  {BLUE}│{RESET} {line1:<16} {BLUE}│{RESET}")
    print(f"  {BLUE}│{RESET} {line2:<16} {BLUE}│{RESET}")
    print(f"  {BLUE}└──────────────────┘{RESET}")

def thread_keypad(pid):
    log_thread("KEYPAD", WHITE, f"Key pressed: {pid[0]} → {pid[1]} → # (confirm)")

def thread_dispense(pid, product):
    log_thread("DISPENSE", YELLOW, f"Servo rotating to 90deg (dispense)...")
    time.sleep(0.6)
    log_thread("DISPENSE", YELLOW, f"Servo reset to 0deg")

def thread_verify():
    time.sleep(0.4)
    dist = 12  # cm — item detected
    log_thread("VERIFY", GREEN, f"Ultrasonic: object at {dist}cm >> DELIVERED ✅")
    return True

def thread_wifi(pid, stock):
    time.sleep(0.3)
    if stock == 0:
        log_thread("WIFI", RED, f"LOW STOCK ALERT: {pid}=0 >> Sent to server")
    else:
        log_thread("WIFI", CYAN, f"Inventory update sent: {pid}={stock} remaining")
    log_thread("WIFI", DIM, "Server ACK received")

def process_purchase(pid):
    pid = pid.strip().upper()

    print(f"\n{DIM}{'─'*47}{RESET}")
    thread_keypad(pid)

    if pid not in inventory:
        log_thread("DISPENSE", RED, f"Unknown product: {pid}")
        lcd_display(f"Unknown: {pid}", "Try again       ")
        return

    with lock:
        product = inventory[pid]

    if product["stock"] == 0:
        log_thread("DISPENSE", RED, f"{pid} OUT OF STOCK")
        lcd_display(f"{pid}: OUT STOCK", "                ")
        thread_wifi(pid, 0)
        return

    lcd_display(f"{pid}: {product['name']}", f"Rs.{product['price']} Dispensing")
    log_thread("DISPENSE", YELLOW, f"Dispensing {pid} ({product['name']}) Rs.{product['price']}")

    thread_dispense(pid, product)

    delivered = thread_verify()

    with lock:
        if delivered:
            inventory[pid]["stock"] -= 1
            remaining = inventory[pid]["stock"]
            log_thread("VERIFY", GREEN, f"Stock updated: {pid}={remaining}")
            lcd_display("Enjoy! Take item", f"{pid} left: {remaining}")
            thread_wifi(pid, remaining)
        else:
            log_thread("VERIFY", RED, "DELIVERY FAILED — Please retry")
            lcd_display("Error! No item", "Please retry    ")

    print()

def help_text():
    print(f"\n{DIM}  Commands:{RESET}")
    print(f"  {YELLOW}A1{RESET} A2 A3 B1 B2 — buy a product")
    print(f"  {YELLOW}inv{RESET}            — show inventory")
    print(f"  {YELLOW}q{RESET}              — quit\n")

def main():
    banner()
    print(f"\n{BOLD}[RTOS]  5 threads spawned — System running{RESET}")
    print_inventory()
    lcd_display("Select Product  ", "A1 A2 A3 B1 B2  ")
    help_text()

    while True:
        try:
            user_input = input(f"\n{CYAN}keypad>{RESET} ").strip()
        except (EOFError, KeyboardInterrupt):
            break

        if not user_input:
            continue
        if user_input.lower() == 'q':
            break
        if user_input.lower() == 'inv':
            print_inventory()
            continue

        if len(user_input) == 2 and user_input[0].isalpha() and user_input[1].isdigit():
            process_purchase(user_input)
        else:
            print(f"{RED}  Invalid input. Try A1, A2, A3, B1, B2{RESET}")

    print(f"\n{DIM}Simulation ended. On real hardware, this runs 24/7 on STM32 F446RE.{RESET}\n")

if __name__ == "__main__":
    main()
