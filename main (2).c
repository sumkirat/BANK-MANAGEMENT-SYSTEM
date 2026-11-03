/*
 * Bank Management System
 * Language : C
 * Features : Create account, PIN login, deposit, withdraw, transfer,
 *            balance check, transaction history, change PIN
 * Storage  : accounts.dat (binary) and transactions.txt (text log)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ACC_FILE   "accounts.dat"
#define TXN_FILE   "transactions.txt"
#define NAME_LEN   50
#define MIN_DEPOSIT 500.0f

typedef struct {
    int   accNo;
    char  name[NAME_LEN];
    int   pin;
    float balance;
} Account;

/* ---------- Input helpers ---------- */

void readLine(const char *prompt, char *buf, int size) {
    printf("%s", prompt);
    if (fgets(buf, size, stdin) != NULL) buf[strcspn(buf, "\n")] = '\0';
    else buf[0] = '\0';
}

int readInt(const char *prompt) {
    char line[32];
    int v;
    while (1) {
        readLine(prompt, line, sizeof(line));
        if (sscanf(line, "%d", &v) == 1) return v;
        printf("Invalid input. Enter a number.\n");
    }
}

float readAmount(const char *prompt) {
    char line[32];
    float v;
    while (1) {
        readLine(prompt, line, sizeof(line));
        if (sscanf(line, "%f", &v) == 1 && v > 0) return v;
        printf("Invalid amount. Enter a value greater than 0.\n");
    }
}

int readPin(const char *prompt) {
    int pin;
    while (1) {
        pin = readInt(prompt);
        if (pin >= 1000 && pin <= 9999) return pin;
        printf("PIN must be exactly 4 digits (1000-9999).\n");
    }
}

/* ---------- File helpers ---------- */

int loadAll(Account **list) {
    FILE *fp = fopen(ACC_FILE, "rb");
    int count = 0, cap = 16;
    Account a;
    *list = malloc(cap * sizeof(Account));
    if (!*list) { if (fp) fclose(fp); return 0; }
    if (!fp) return 0;
    while (fread(&a, sizeof(Account), 1, fp) == 1) {
        if (count == cap) {
            Account *tmp = realloc(*list, (cap *= 2) * sizeof(Account));
            if (!tmp) break;
            *list = tmp;
        }
        (*list)[count++] = a;
    }
    fclose(fp);
    return count;
}

int saveAll(Account *list, int count) {
    FILE *fp = fopen(ACC_FILE, "wb");
    if (!fp) return 0;
    fwrite(list, sizeof(Account), count, fp);
    fclose(fp);
    return 1;
}

int findIndex(Account *list, int n, int accNo) {
    int i;
    for (i = 0; i < n; i++)
        if (list[i].accNo == accNo) return i;
    return -1;
}

int nextAccountNumber(Account *list, int n) {
    int i, max = 1000;
    for (i = 0; i < n; i++)
        if (list[i].accNo > max) max = list[i].accNo;
    return max + 1;
}

void logTransaction(int accNo, const char *type, float amount, float balanceAfter) {
    FILE *fp = fopen(TXN_FILE, "a");
    char timeStr[32];
    time_t now = time(NULL);
    if (!fp) return;
    strftime(timeStr, sizeof(timeStr), "%d-%m-%Y %H:%M", localtime(&now));
    fprintf(fp, "%d|%s|%s|%.2f|%.2f\n", accNo, timeStr, type, amount, balanceAfter);
    fclose(fp);
}

/* ---------- Account operations ---------- */

void createAccount(void) {
    Account *list, a;
    int n = loadAll(&list);
    float deposit;

    readLine("Enter account holder name: ", a.name, NAME_LEN);
    if (strlen(a.name) == 0) {
        printf("Name cannot be empty.\n");
        free(list);
        return;
    }
    a.pin = readPin("Set a 4-digit PIN: ");
    do {
        deposit = readAmount("Initial deposit (minimum 500): ");
        if (deposit < MIN_DEPOSIT) printf("Minimum opening deposit is %.0f.\n", MIN_DEPOSIT);
    } while (deposit < MIN_DEPOSIT);

    a.accNo = nextAccountNumber(list, n);
    a.balance = deposit;

    FILE *fp = fopen(ACC_FILE, "ab");
    if (!fp) {
        printf("Error: could not open file.\n");
    } else {
        fwrite(&a, sizeof(Account), 1, fp);
        fclose(fp);
        logTransaction(a.accNo, "OPENING", deposit, deposit);
        printf("\nAccount created successfully!\n");
        printf("Your account number is: %d (remember it for login)\n", a.accNo);
    }
    free(list);
}

void deposit(Account *list, int idx, int n) {
    float amt = readAmount("Enter amount to deposit: ");
    list[idx].balance += amt;
    if (saveAll(list, n)) {
        logTransaction(list[idx].accNo, "DEPOSIT", amt, list[idx].balance);
        printf("Deposited %.2f. New balance: %.2f\n", amt, list[idx].balance);
    } else {
        list[idx].balance -= amt;
        printf("Error: could not save. Transaction cancelled.\n");
    }
}

void withdraw(Account *list, int idx, int n) {
    float amt = readAmount("Enter amount to withdraw: ");
    if (amt > list[idx].balance) {
        printf("Insufficient balance. Available: %.2f\n", list[idx].balance);
        return;
    }
    list[idx].balance -= amt;
    if (saveAll(list, n)) {
        logTransaction(list[idx].accNo, "WITHDRAW", amt, list[idx].balance);
        printf("Withdrawn %.2f. New balance: %.2f\n", amt, list[idx].balance);
    } else {
        list[idx].balance += amt;
        printf("Error: could not save. Transaction cancelled.\n");
    }
}

void transfer(Account *list, int idx, int n) {
    int toNo = readInt("Enter receiver account number: ");
    int toIdx = findIndex(list, n, toNo);
    float amt;

    if (toIdx == -1) { printf("Receiver account not found.\n"); return; }
    if (toIdx == idx) { printf("You cannot transfer to your own account.\n"); return; }

    amt = readAmount("Enter amount to transfer: ");
    if (amt > list[idx].balance) {
        printf("Insufficient balance. Available: %.2f\n", list[idx].balance);
        return;
    }
    list[idx].balance -= amt;
    list[toIdx].balance += amt;
    if (saveAll(list, n)) {
        logTransaction(list[idx].accNo, "TRANSFER-OUT", amt, list[idx].balance);
        logTransaction(list[toIdx].accNo, "TRANSFER-IN", amt, list[toIdx].balance);
        printf("Transferred %.2f to %s (A/C %d). New balance: %.2f\n",
               amt, list[toIdx].name, toNo, list[idx].balance);
    } else {
        list[idx].balance += amt;
        list[toIdx].balance -= amt;
        printf("Error: could not save. Transaction cancelled.\n");
    }
}

void showHistory(int accNo) {
    FILE *fp = fopen(TXN_FILE, "r");
    char line[128];
    int found = 0;
    if (!fp) { printf("No transactions yet.\n"); return; }

    printf("\n%-18s %-14s %-12s %-12s\n", "Date & Time", "Type", "Amount", "Balance");
    printf("------------------------------------------------------------\n");
    while (fgets(line, sizeof(line), fp)) {
        int no;
        char date[32], type[24];
        float amt, bal;
        if (sscanf(line, "%d|%31[^|]|%23[^|]|%f|%f", &no, date, type, &amt, &bal) == 5
            && no == accNo) {
            printf("%-18s %-14s %-12.2f %-12.2f\n", date, type, amt, bal);
            found = 1;
        }
    }
    if (!found) printf("No transactions found.\n");
    fclose(fp);
}

void changePin(Account *list, int idx, int n) {
    int oldPin = readInt("Enter current PIN: ");
    int newPin;
    if (oldPin != list[idx].pin) { printf("Incorrect PIN.\n"); return; }
    newPin = readPin("Enter new 4-digit PIN: ");
    list[idx].pin = newPin;
    if (saveAll(list, n)) printf("PIN changed successfully.\n");
    else printf("Error: could not save new PIN.\n");
}

/* ---------- Login and menus ---------- */

void accountMenu(int accNo) {
    Account *list;
    int n, idx, choice;

    do {
        n = loadAll(&list);               /* reload so data is always current */
        idx = findIndex(list, n, accNo);
        if (idx == -1) { printf("Account no longer exists.\n"); free(list); return; }

        printf("\n------ Welcome, %s (A/C %d) ------\n", list[idx].name, accNo);
        printf("1. Check balance\n2. Deposit\n3. Withdraw\n4. Transfer money\n");
        printf("5. Transaction history\n6. Change PIN\n0. Logout\n");
        choice = readInt("Enter your choice: ");

        switch (choice) {
            case 1: printf("Current balance: %.2f\n", list[idx].balance); break;
            case 2: deposit(list, idx, n);  break;
            case 3: withdraw(list, idx, n); break;
            case 4: transfer(list, idx, n); break;
            case 5: showHistory(accNo);     break;
            case 6: changePin(list, idx, n); break;
            case 0: printf("Logged out.\n"); break;
            default: printf("Invalid choice.\n");
        }
        free(list);
    } while (choice != 0);
}

void login(void) {
    Account *list;
    int n = loadAll(&list), idx, accNo, pin, attempts;

    accNo = readInt("Enter account number: ");
    idx = findIndex(list, n, accNo);
    if (idx == -1) {
        printf("Account not found.\n");
        free(list);
        return;
    }
    for (attempts = 3; attempts > 0; attempts--) {
        pin = readInt("Enter PIN: ");
        if (pin == list[idx].pin) {
            free(list);
            accountMenu(accNo);
            return;
        }
        printf("Wrong PIN. Attempts left: %d\n", attempts - 1);
    }
    printf("Too many wrong attempts. Returning to main menu.\n");
    free(list);
}

int main(void) {
    int choice;
    do {
        printf("\n============= BANK MANAGEMENT SYSTEM =============\n");
        printf("1. Create new account\n2. Login\n0. Exit\n");
        choice = readInt("Enter your choice: ");
        switch (choice) {
            case 1: createAccount(); break;
            case 2: login();         break;
            case 0: printf("Thank you for banking with us!\n"); break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 0);
    return 0;
}
