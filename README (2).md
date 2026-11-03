# Bank Management System (C)

A console-based banking application written in C. Customers can open an account, log in with a PIN, and manage their money. All data is stored in files, so it is kept between runs.

## Features
- Create an account (auto-generated account number, 4-digit PIN, minimum opening deposit of 500)
- PIN login with 3 attempts
- Check balance, deposit and withdraw
- Transfer money between accounts
- Transaction history with date and time
- Change PIN
- Input validation (no negative amounts, no overdraft, no self-transfer)

## Concepts used
Structures, binary and text file handling, dynamic memory (`malloc`, `realloc`, `free`), the `time.h` library, string parsing with `sscanf`, and modular functions.

## Build and run
```bash
gcc main.c -o bank -Wall
./bank        # Linux / macOS
bank.exe      # Windows
```

## Files created at runtime
- `accounts.dat` stores account records (binary)
- `transactions.txt` stores the transaction log (text)

## Note
This is an educational project. PINs are stored as plain numbers, so it is not meant for real banking use.

## Author
Khayati Gupta, Chandigarh University
