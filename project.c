#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FILENAME "accounts.dat"
#define LOGFILE "account_log.txt"

// Structure to hold account details
struct Account {
    int accountNumber;
    char name[100];
    float balance;
    time_t createdTime; // Timestamp for when the account was created
};

// Function prototypes
void logTransaction(int accountNumber, const char *transactionType, float amount);
void createAccount();
void viewAccount();
void deposit();
void withdraw();
void displayTotalAccounts();
void displayAllLogs();
void displayAccountLog();
void transfer(); // Function prototype for transferring money
void clearScreen(); // Prototype for clear screen function

void clearScreen() {
    system("clear");
}

void createAccount() {
    struct Account account;
    FILE *file = fopen(FILENAME, "ab"); // Open in binary append mode

    if (!file) {
        printf("Unable to open accounts file.\n");
        return;
    }

    printf("Enter account number: ");
    scanf("%d", &account.accountNumber);
    printf("Enter account holder name: ");
    getchar(); // To consume newline
    fgets(account.name, sizeof(account.name), stdin);
    account.name[strcspn(account.name, "\n")] = 0; // Remove newline
    account.balance = 0.0;
    account.createdTime = time(NULL); // Set the current time

    fwrite(&account, sizeof(struct Account), 1, file);
    fclose(file);

    // Log account creation
    logTransaction(account.accountNumber, "Account Created", 0);
    printf("Account created successfully!\n");
    getchar(); // Wait for user input
}

void logTransaction(int accountNumber, const char *transactionType, float amount) {
    FILE *logFile = fopen(LOGFILE, "a"); // Open log file in append mode
    if (!logFile) {
        printf("Unable to open log file.\n");
        return;
    }
    time_t now = time(NULL);
    fprintf(logFile, "Account Number: %d, Transaction: %s, Amount: $%.2f, Time: %s",
            accountNumber, transactionType, amount, ctime(&now));
    fclose(logFile);
}

void viewAccount() {
    struct Account account;
    int accountNumber;
    int found = 0;
    FILE *file = fopen(FILENAME, "rb"); // Open in binary read mode

    if (!file) {
        printf("Unable to open accounts file.\n");
        return;
    }

    printf("Enter account number to view: ");
    scanf("%d", &accountNumber);

    while (fread(&account, sizeof(struct Account), 1, file)) {
        if (account.accountNumber == accountNumber) {
            printf("\nAccount Number: %d\n", account.accountNumber);
            printf("Account Holder Name: %s\n", account.name);
            printf("Balance: $%.2f\n", account.balance);
            printf("Account Created On: %s", ctime(&account.createdTime)); // Display creation time
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("Account not found!\n");
    }

    fclose(file);
    getchar(); // Wait for user input
}

void deposit() {
    struct Account account;
    int accountNumber;
    float amount;
    int found = 0;
    FILE *file = fopen(FILENAME, "r+b"); // Open in binary read/write mode

    if (!file) {
        printf("Unable to open accounts file.\n");
        return;
    }

    printf("Enter account number to deposit: ");
    scanf("%d", &accountNumber);
    printf("Enter amount to deposit: ");
    scanf("%f", &amount);

    while (fread(&account, sizeof(struct Account), 1, file)) {
        if (account.accountNumber == accountNumber) {
            account.balance += amount;
            fseek(file, -sizeof(struct Account), SEEK_CUR); // Move back to update
            fwrite(&account, sizeof(struct Account), 1, file);
            found = 1;
            logTransaction(accountNumber, "Deposit", amount); // Log the transaction
            printf("Deposit successful! New Balance: $%.2f\n", account.balance);
            break;
        }
    }

    if (!found) {
        printf("Account not found!\n");
    }

    fclose(file);
    getchar(); // Wait for user input
}

void withdraw() {
    struct Account account;
    int accountNumber;
    float amount;
    int found = 0;
    FILE *file = fopen(FILENAME, "r+b"); // Open in binary read/write mode

    if (!file) {
        printf("Unable to open accounts file.\n");
        return;
    }

    printf("Enter account number to withdraw from: ");
    scanf("%d", &accountNumber);
    printf("Enter amount to withdraw: ");
    scanf("%f", &amount);

    while (fread(&account, sizeof(struct Account), 1, file)) {
        if (account.accountNumber == accountNumber) {
            if (account.balance >= amount) {
                account.balance -= amount;
                fseek(file, -sizeof(struct Account), SEEK_CUR); // Move back to update
                fwrite(&account, sizeof(struct Account), 1, file);
                found = 1;
                logTransaction(accountNumber, "Withdrawal", amount); // Log the transaction
                printf("Withdrawal successful! New Balance: $%.2f\n", account.balance);
            } else {
                printf("Insufficient balance!\n");
            }
            break;
        }
    }

    if (!found) {
        printf("Account not found!\n");
    }

    fclose(file);
    getchar(); // Wait for user input
}

void transfer() {
    struct Account fromAccount, toAccount;
    int fromAccountNumber, toAccountNumber;
    float amount;
    int foundFrom = 0, foundTo = 0;
    FILE *file = fopen(FILENAME, "r+b"); // Open in binary read/write mode

    if (!file) {
        printf("Unable to open accounts file.\n");
        return;
    }

    printf("Enter account number to transfer from: ");
    scanf("%d", &fromAccountNumber);
    printf("Enter account number to transfer to: ");
    scanf("%d", &toAccountNumber);
    printf("Enter amount to transfer: ");
    scanf("%f", &amount);

    // Find the from account
    while (fread(&fromAccount, sizeof(struct Account), 1, file)) {
        if (fromAccount.accountNumber == fromAccountNumber) {
            foundFrom = 1;
            if (fromAccount.balance >= amount) {
                // Find the to account
                fseek(file, 0, SEEK_SET); // Reset file position to the start
                while (fread(&toAccount, sizeof(struct Account), 1, file)) {
                    if (toAccount.accountNumber == toAccountNumber) {
                        foundTo = 1;
                        // Perform the transfer
                        fromAccount.balance -= amount;
                        toAccount.balance += amount;

                        // Move back to update both accounts
                        fseek(file, -2 * sizeof(struct Account), SEEK_CUR);
                        fwrite(&fromAccount, sizeof(struct Account), 1, file);
                        fwrite(&toAccount, sizeof(struct Account), 1, file);

                        logTransaction(fromAccountNumber, "Transfer Out", amount); // Log the transfer out
                        logTransaction(toAccountNumber, "Transfer In", amount); // Log the transfer in
                        printf("Transfer successful! New Balance from Account %d: $%.2f\n", fromAccountNumber, fromAccount.balance);
                        printf("New Balance to Account %d: $%.2f\n", toAccountNumber, toAccount.balance);
                        break;
                    }
                }
            } else {
                printf("Insufficient balance in from account!\n");
            }
            break;
        }
    }

    if (!foundFrom) {
        printf("From account not found!\n");
    } else if (!foundTo) {
        printf("To account not found!\n");
    }

    fclose(file);
    getchar(); // Wait for user input
}

void displayTotalAccounts() {
    struct Account account;
    int count = 0;
    FILE *file = fopen(FILENAME, "rb"); // Open in binary read mode

    if (!file) {
        printf("Unable to open accounts file.\n");
        return;
    }

    while (fread(&account, sizeof(struct Account), 1, file)) {
        count++;
    }

    printf("Total Accounts: %d\n", count);
    fclose(file);
    getchar(); // Wait for user input
}

// Function to display all logs
void displayAllLogs() {
    FILE *logFile = fopen(LOGFILE, "r"); // Open log file in read mode
    if (!logFile) {
        printf("Unable to open log file.\n");
        return;
    }

    char line[256];
    printf("\nLog History:\n");
    while (fgets(line, sizeof(line), logFile)) {
        printf("%s", line);
    }

    fclose(logFile);
    getchar(); // Wait for user input
}

// Function to display log for a specific account
void displayAccountLog() {
    int accountNumber;
    printf("Enter account number to view its log: ");
    scanf("%d", &accountNumber);

    FILE *logFile = fopen(LOGFILE, "r"); // Open log file in read mode
    if (!logFile) {
        printf("Unable to open log file.\n");
        return;
    }

    char line[256];
    int found = 0;
    printf("\nLog History for Account Number %d:\n", accountNumber);
    while (fgets(line, sizeof(line), logFile)) {
        if (strstr(line, "Account Number:") && strstr(line, (char*)&accountNumber)) {
            printf("%s", line);
            found = 1;
        }
    }

    if (!found) {
        printf("No logs found for account number %d.\n", accountNumber);
    }

    fclose(logFile);
    getchar(); // Wait for user input
}

// Main function with menu selection
int main() {
    int choice;

    do {
        clearScreen(); // Clear screen at the start of each loop
        printf("\n===== Bank Management System =====\n");
        printf("1. Create Account\n");
        printf("2. View Account\n");
        printf("3. Deposit Money\n");
        printf("4. Withdraw Money\n");
        printf("5. Transfer Money\n");
        printf("6. Display Total Accounts\n");
        printf("7. Display All Logs\n");
        printf("8. Display Log for Specific Account\n");
        printf("9. Exit\n");
        printf("==================================\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                createAccount();
                break;
            case 2:
                viewAccount();
                break;
            case 3:
                deposit();
                break;
            case 4:
                withdraw();
                break;
            case 5:
                transfer();
                break;
            case 6:
                displayTotalAccounts();
                break;
            case 7:
                displayAllLogs();
                break;
            case 8:
                displayAccountLog();
                break;
            case 9:
                printf("Exiting...\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
                printf("Press Enter to continue...");
                getchar(); // Wait for user input
                getchar(); // To capture the newline after scanf
        }

        // After each operation, ask the user if they want to continue or return to the menu
        if (choice >= 1 && choice <= 8) {
            printf("Press Enter to return to the menu...");
           // getchar(); // Consume leftover newline
            getchar(); // Wait for user input
        }

    } while (choice != 9);

    return 0;
}
