#include "AtsTty.h"
#include <mem/mem.h>
#include <InterruptDescriptors/Drivers/KeyboardDev/KbdDev.h>
#include <TTY/printf/printf.h>

uint64_t cell_x = 3;
uint64_t cell_y = 0;

uint64_t max_cell_x = 0;
uint64_t max_cell_y = 0;

uint32_t color = 0xFFFFFFFF; // Global color

int XOFF = 0;
int YOFF = 0;

int getXoffParam() {
    return XOFF;
}

int getYoffParam() {
    return YOFF;
}

void tty_init() {
    cell_x = cell_y = 0;
    max_cell_x = (GetFb()->width / 8) - 1 - XOFF; // maximum amount of TTY cells on the x-axis is the width of the framebuffer divided by the width of the font-char (8)
    max_cell_y = (GetFb()->height / 16) - 1 - YOFF; // maximum amount of TTY cells on the y-axis is the height of the framebuffer divided by the height of the font-char (16)
}

bool escapeSeqStartFound = false;
bool escapeSeqSquareParenStartFound = false;
bool escapeSequenceNumFound = false;
bool escapeSeqSquareParenEndFound = false;
bool escapeSeqEndFound = false;
int ates_clr = 0;

void tty_putchar(char c) {
    if (c == '\n') { // Handle newline
        cell_x = 0;
        cell_y++;
    } else if (c == '\r') { // Handle carriage return
        cell_x = 0;
    } else if (c == '\b') { // Handle backspace
        if (cell_x > 0) {
            cell_x--;
        } else if (cell_y > 0) { // If we're at the beginning of the line, move up
            cell_y--;
            cell_x = max_cell_x;
        }
        DrawRect(cell_x * 8, cell_y * 16, 8, 16, 0x000000);  // Clear the backspace position
    } else if (c == '\e') {
        escapeSeqStartFound = true;
    } else if (c == '[' && escapeSeqStartFound) {
        escapeSeqSquareParenStartFound = true;
    } else if ('0' <= c && c <= '9' && escapeSeqStartFound && escapeSeqSquareParenStartFound) {
        escapeSequenceNumFound = true;
        ates_clr = c - '0';
    } else if (c == ']' && escapeSeqStartFound && escapeSeqSquareParenStartFound && escapeSequenceNumFound) {
        escapeSeqSquareParenEndFound = true;
    } else if (c == ';' && escapeSeqStartFound && escapeSeqSquareParenStartFound && escapeSequenceNumFound && escapeSeqSquareParenEndFound) {
        escapeSeqEndFound = true;
        color = ParseAtes(ates_clr);
    } else { // Normal character
        if (cell_x >= max_cell_x) { // End of the current line
            cell_x = 0;
            cell_y++;  // Move to the next line
        }
    
        if (cell_y >= max_cell_y) { // End of the screen
            // Clear the entire screen (set all cells to 0)
            DrawRect(0, 0, GetFb()->width, GetFb()->height, 0x000000); // Black background
            cell_x = 0;
            cell_y = 0;
        } else {
            // Draw character at current position
            DrawRect((cell_x + XOFF) * 8, (cell_y + YOFF) * 16, 8, 16, 0x000000);  // Clear previous cell if any
            FontPutChar(c, (cell_x + XOFF) * 8, (cell_y + YOFF) * 16, color); // Draw the character
            cell_x++;
        }

        escapeSeqStartFound = false;
        escapeSeqSquareParenStartFound = false;
        escapeSequenceNumFound = false;
        escapeSeqSquareParenEndFound = false;
        escapeSeqEndFound = false;
        ates_clr = 0;
    }
}

void tty_putchar_simple(char c) {
    if (c == '\n') { // Handle newline
        cell_x = 0;
        cell_y++;
    } else if (c == '\r') { // Handle carriage return
        cell_x = 0;
    } else if (c == '\b') { // Handle backspace
        if (cell_x > 0) {
            cell_x--;
        } else if (cell_y > 0) { // If we're at the beginning of the line, move up
            cell_y--;
            cell_x = max_cell_x;
        }
        DrawRect(cell_x * 8, cell_y * 16, 8, 16, 0x000000);  // Clear the backspace position
    } else { // Normal character
        if (cell_x >= max_cell_x) { // End of the current line
            cell_x = 0;
            cell_y++;  // Move to the next line
        }
    
        if (cell_y >= max_cell_y) { // End of the screen
            // Clear the entire screen (set all cells to 0)
            DrawRect(0, 0, GetFb()->width, GetFb()->height, 0x000000); // Black background
            cell_x = 0;
            cell_y = 0;
        } else {
            // Draw character at current position
            DrawRect((cell_x + XOFF) * 8, (cell_y + YOFF) * 16, 8, 16, 0x000000);  // Clear previous cell if any
            FontPutChar(c, (cell_x + XOFF) * 8, (cell_y + YOFF) * 16, 0xFFFFFF); // Draw the character
            cell_x++;
        }
    }
}

void tty_puts(const char *s) {
    while (*s != 0) {
        if (*s != 0) {
            tty_putchar(*s); // Print the character with the current color
        }
        s++;
    }
}

void tty_set_cursor(uint64_t x, uint64_t y) {
    if (0 <= x && x <= max_cell_x) {
        cell_x = x;
    } else {
        cell_x = cell_x; // Retain current value if out of bounds
    }
    if (0 <= y && y <= max_cell_y) {
        cell_y = y;
    } else {
        cell_y = cell_y; // Retain current value if out of bounds
    }
}

uint64_t tty_get_dimensions() {
    return (max_cell_x << 32) | (max_cell_y & 0xFFFFFFFF);
}

uint64_t getCellx() {
    return cell_x;
}

uint64_t getCelly() {
    return cell_y;
}

#include <Language/language.h>
#include <CMOSTime/CMOSTime.h>

size_t StringLength(const char* str) {
    size_t length = 0;
    
    // Iterate over the string until the null terminator is found
    while (str[length] != '\0') {
        length++;
    }

    return length;
}

#define MAX_STRING_LEN 4096

char static_buffer[MAX_STRING_LEN];

char* StringDuplicate(const char* str) {
    size_t len = StringLength(str);

    if (len >= MAX_STRING_LEN) {
        return NULL;
    }

    memcpy(static_buffer, str, len);
    static_buffer[len] = '\0';

    return static_buffer;
}

char *StringFindCharacter(const char *str, int c) {
    while (*str) {
        if (*str == (char)c) {
            return (char *)str;
        }
        str++;
    }
    
    return NULL;
}

#define MAX_TOKENS 2048

static char *token_ptr = NULL;

char *StringTokenize(char *str) {
    if (str != NULL) {
        token_ptr = str;
    }

    if (token_ptr == NULL) {
        return NULL;
    }

    char *start = token_ptr;

    while (*token_ptr && *token_ptr == ' ') {
        token_ptr++;
    }

    if (*token_ptr == '\0') {
        return NULL;
    }

    while (*token_ptr && *token_ptr != ' ') {
        token_ptr++;
    }

    if (*token_ptr != '\0') {
        *token_ptr = '\0';
        token_ptr++;
    }

    return start;
}

void StringSplitDelimSpace(char *str, char **tokens) {
    int i = 0;
    char *token = StringTokenize(str);

    while (token != NULL && i < MAX_TOKENS-1) {
        tokens[i++] = token;
        token = StringTokenize(NULL);
    }
}

int scmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return (unsigned char)*str1 - (unsigned char)*str2;
}

void handle_command(const char **argv);

void ShellRoutine() {
    tty_init();

    while (1) {
        printf("\e[7];user@%s\e[0];:\e[8];~\e[0];$ ", GetUsername());

        char __cmd_buf[2048] = {0};
        __keyboard_gets(__cmd_buf, 2048);

        char *__argv[2048];
        StringSplitDelimSpace(__cmd_buf, __argv);

        handle_command((const char**)__argv);
    }
}

void do_calc(const char **args);
void do_clear();
void do_date();
void do_echo(const char **args);
void do_exit();
void do_hello();
void do_help();
void do_mac();
void do_ps();
void do_reboot();
void do_sysinfo();
void do_time();
void do_whoami();

void handle_command(const char **argv) {
    const char* cmd = argv[0];

    if (scmp(cmd, "calc") == 0) {
        do_calc(argv);
    } else if (scmp(cmd, "clear") == 0) {
        do_clear();
    } else if (scmp(cmd, "date") == 0) {
        do_date();
    } else if (scmp(cmd, "echo") == 0) {
        do_echo(argv);
    } else if (scmp(cmd, "exit") == 0) {
        do_exit();
    } else if (scmp(cmd, "hello") == 0) {
        do_hello();
    } else if (scmp(cmd, "help") == 0) {
        do_help();
    } else if (scmp(cmd, "mac") == 0) {
        do_mac();
    } else if (scmp(cmd, "ps") == 0) {
        do_ps();
    } else if (scmp(cmd, "reboot") == 0) {
        printf("Rebooting...\n\rLOG: Triple faulting CPU to cause reboot\n\r");
        do_reboot();
    } else if (scmp(cmd, "sysinfo") == 0) {
        do_sysinfo();
    } else if (scmp(cmd, "time") == 0) {
        do_time();
    } else if (scmp(cmd, "whoami") == 0) {
        do_whoami();
    } else {
        if (cmd != 0 && cmd != ' ')
            printf("Unknown command: %s\n\r", cmd);
    }
}

/* COMMANDS HELPER FUNCTIONS */

int stoi(const char* str) {
    int result = 0;
    int sign = 1;
    int i = 0;

    if (str[0] == '-') {
        sign = -1;
        i = 1;
    }

    for (; str[i] != '\0'; i++) {
        if (str[i] < '0' || str[i] > '9') return 0;
        result = result * 10 + (str[i] - '0');
    }

    return sign * result;
}

void itos(int num, char* str) {
    int i = 0;
    int is_negative = 0;

    // Handle 0 as a special case
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    // Handle negative numbers
    if (num < 0) {
        is_negative = 1;
        num = -num; // Make the number positive for easier processing
    }

    // Process the number and convert it to a string
    while (num != 0) {
        str[i++] = (num % 10) + '0'; // Store the last digit
        num /= 10; // Remove the last digit
    }

    // Add negative sign if necessary
    if (is_negative) {
        str[i++] = '-';
    }

    str[i] = '\0'; // Null-terminate the string

    // Reverse the string (since we processed digits in reverse order)
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

/* COMMANDS */

// Function to print "Hello, World!"
void do_hello() {
    printf("Hello, World!\n\r");
}

// Function to show the current username
void do_whoami() {
    printf("%s\n\r", GetUsername());
}

// Function to show help
void do_help() {
    printf("Help: List of available commands:\n"
              "hello - Print a greeting\n"
              "whoami - Display the current user\n"
              "csess - Create a new session\n"
              "help - Show this help message\n"
              "echo - Echo a message\n"
              "calc - Perform a basic calculation\n"
              "date - Show the current date\n"
              "clear - Clear the terminal\n"
              "exit - Exit the shell\n"
              "time - Show the current time\n"
              "mac - Show the MAC address\n"
              "ps - Show process status\n"
              "reboot - Reboot the system\n"
              "sysinfo - Show system information\n\r");
}

// Function to echo back a message
void do_echo(const char **args) {
    if (args[1] == NULL) {
        return;
    }

    for (int i = 1; args[i] != NULL; i++) {
        printf("%s ", args[i]);
    }

    printf("\n\r");
}

// Function to perform basic calculation (add two numbers)
void do_calc(const char **args) {
    // Check if all necessary arguments are provided
    if (args[1] == NULL || args[2] == NULL || args[3] == NULL) {
        printf("Invalid input format. Usage: calc <num1> <operator> <num2>\n\r");
        return;
    }

    int num1 = stoi(args[1]);
    int num2 = stoi(args[3]);
    char op = args[2][0];

    // Perform operation based on the operator
    switch (op) {
        case '+':
            printf("Result: %d\n\r", num1 + num2);
            break;
        case '-':
            printf("Result: %d\n\r", num1 - num2);
            break;
        case '*':
            printf("Result: %d\n\r", num1 * num2);
            break;
        case '/':
            if (num2 != 0) {
                printf("Result: %d\n\r", num1 / num2);
            } else {
                printf("Error: Division by zero.\n\r");
            }
            break;
        case '^': // Exponentiation
            {
                int result = 1;
                for (int i = 0; i < num2; i++) {
                    result *= num1;
                }
                printf("Result: %d\n\r", result);
            }
            break;
        case '&': // Bitwise AND
            printf("Result: %d\n\r", num1 & num2);
            break;
        case '|': // Bitwise OR
            printf("Result: %d\n\r", num1 | num2);
            break;
        case '~': // Bitwise NOT (on num1)
            printf("Result: %d\n\r", ~num1);
            break;
        default:
            printf("Invalid operator! Supported operators are: +, -, *, /, ^, &, |, ~\n\r");
            break;
    }
}

// Function to show the current date
void do_date() {
    do_time();
}

// Function to clear the terminal screen
void do_clear() {
    ClearScreenColor(0x000000);
    tty_set_cursor(0, 0);
}

// Function to exit the shell
void do_exit() {
    printf("Note: To reopen shell you must reset the machine\n\rExiting the shell...\n\r");
    while (1) {};
}

void do_time() {
    printf("%s\n\r", CmosGetTime());
}

void do_mac() {
    printf("WIP\n\r");
}

void do_ps() {
    printf("AtlasShell: PID: 0000\n\r");
}

void do_reboot() {
    idtr_t idtr;
    idtr_t* idt_r_empty = &idtr;
    idt_r_empty->base = 0;
    idt_r_empty->limit = 0;
    __asm__ volatile ("lidt %0" : : "m"(idt_r_empty));
    __asm__ volatile ("int $0x00");
    while (1) { }
}

void do_sysinfo() {
    printf("System Info: AtlasOS - Version 0.0.4\n\r");
}