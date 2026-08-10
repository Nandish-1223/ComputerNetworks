#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STR 100
#define MAX_BITS 1024

/* --- Function Declarations --- */
void ascii_to_bin(char ch, char *b);
char bin_to_ascii(const char *b);
void str_to_bits(const char *str, char *bits);
void bits_to_str(const char *bits, char *str);
void xor_div(const char *data, const char *gen, char *remainder);
void check_generator_quality(const char *gen);

/* --- Main Function --- */
int main() {
    // Shared State Variables
    char input[MAX_STR] = {0};
    char data[MAX_BITS] = {0};
    char gen[32] = "1001"; // Default generator
    char dividend[MAX_BITS] = {0};
    char remainder[32] = {0};
    char transmitted[MAX_BITS] = {0};
    char received[MAX_BITS] = {0};

    int data_ready = 0;        // Track if sender has generated data
    int error_staged = 0;      // Track if data has been passed to channel
    int choice;

    while (1) {
        printf("\nMENU\n");
        printf(" 1. Sender Side (Encode Data & Generate CRC)\n");
        printf(" 2. Error Injection (Flip Channel Bits)\n");
        printf(" 3. Receiver Side (Decode & Check Errors)\n");
        printf(" 4. Exit\n");
        printf("--------------------------------------------------\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("\nInvalid.\n");
            while (getchar() != '\n'); // Clear buffer
            continue;
        }

        switch (choice) {
            case 1: {
                printf("\n>>> SENDER SIDE OPERATION <<<\n");
                printf("Enter string message: ");
                scanf("%99s", input);

                str_to_bits(input, data);
                printf("Source data (binary): %s\n", data);

                printf("Enter generator bits (or 0 for default 1001): ");
                scanf("%31s", gen);
                if (strcmp(gen, "0") == 0) {
                    strcpy(gen, "1001");
                }
                printf("Generator G(x) used : %s\n", gen);

                /* Validate and explain generator quality */
                check_generator_quality(gen);

                int r = strlen(gen) - 1;
                strcpy(dividend, data);
                for (int i = 0; i < r; i++) {
                    dividend[strlen(data) + i] = '0';
                }
                dividend[strlen(data) + r] = '\0';
                printf("Data padded with %d zeros: %s\n", r, dividend);

                xor_div(dividend, gen, remainder);
                printf("CRC remainder : %s\n", remainder);

                strcpy(transmitted, data);
                strcat(transmitted, remainder);
                printf("Transmitted Codeword (CW = D + R)  : %s\n", transmitted);

                // Stage copy for error injection/receiver
                strcpy(received, transmitted);
                data_ready = 1;
                error_staged = 0; // Reset channel status for new data
                break;
            }

            case 2: {
                if (!data_ready) {
                    printf("\nError: Run Sender Side.\n");
                    break;
                }

                printf("\nERROR INJECTION SIDE\n");
                int len = strlen(received);
                int pos;

                printf("Enter bit position to flip (0 to %d) or -1 for NO error: ", len - 1);
                scanf("%d", &pos);

                if (pos >= 0 && pos < len) {
                    received[pos] = (received[pos] == '0') ? '1' : '0';
                    printf("Bit at position %d flipped.\n", pos);
                } else {
                    printf("No errors introduced into the stream.\n");
                }

                printf("Current bitstream heading to receiver: %s\n", received);
                error_staged = 1;
                break;
            }

            case 3: {
                if (!data_ready) {
                    printf("\nError: Run Sender Side.\n");
                    break;
                }

                printf("\n RECEIVER SIDE VERIFICATION\n");
                printf("Received data bitstream: %s\n", received);

                char check_remainder[32] = {0};
                xor_div(received, gen, check_remainder);
                printf("Remainder after division by G(x): %s\n", check_remainder);

                int all_zero = 1;
                for (int i = 0; i < (int)strlen(check_remainder); i++) {
                    if (check_remainder[i] != '0') {
                        all_zero = 0;
                    }
                }

                printf("\nFINAL RESULT\n");
                if (all_zero) {
                    printf("Result: NO ERROR DETECTED\n");

                    char recovered_bits[MAX_BITS] = {0};
                    char recovered_str[MAX_STR] = {0};

                    int src_data_len = strlen(transmitted) - (strlen(gen) - 1);
                    strncpy(recovered_bits, received, src_data_len);
                    recovered_bits[src_data_len] = '\0';

                    bits_to_str(recovered_bits, recovered_str);
                    printf("Recovered message (string): %s\n", recovered_str);
                } else {
                    printf("Result: ERROR DETECTED\n");
                }
                break;
            }

            case 4:
                printf("\nExiting program.\n");
                return 0;

            default:
                printf("\nInvalid.\n");
        }
    }
    return 0;
}

/* --- Helper Function Definitions --- */

void ascii_to_bin(char ch, char *b) {
    for (int i = 7; i >= 0; i--) {
        b[7 - i] = (ch & (1 << i)) ? '1' : '0';
    }
    b[8] = '\0';
}

char bin_to_ascii(const char *b) {
    char ch = 0;
    for (int i = 0; i < 8; i++) {
        ch = (ch << 1) | (b[i] - '0');
    }
    return ch;
}

void str_to_bits(const char *str, char *bits) {
    int len = strlen(str);
    char b[9];
    for (int i = 0; i < len; i++) {
        ascii_to_bin(str[i], b);
        strcpy(bits + (i * 8), b);
    }
    bits[len * 8] = '\0';
}

void bits_to_str(const char *bits, char *str) {
    int nchars = strlen(bits) / 8;
    for (int i = 0; i < nchars; i++) {
        str[i] = bin_to_ascii(bits + (i * 8));
    }
    str[nchars] = '\0';
}

void xor_div(const char *data, const char *gen, char *remainder) {
    int data_len = strlen(data);
    int gen_len = strlen(gen);
    char *temp = (char *)malloc(data_len + 1);
    strcpy(temp, data);

    for (int i = 0; i <= data_len - gen_len; i++) {
        if (temp[i] == '1') {
            for (int j = 0; j < gen_len; j++) {
                temp[i + j] = (((temp[i + j] - '0') ^ (gen[j] - '0')) + '0');
            }
        }
    }
    strcpy(remainder, temp + (data_len - (gen_len - 1)));
    free(temp);
}

/* Validates the generator and lists exact reasons for failure */
void check_generator_quality(const char *gen) {
    int gen_len = strlen(gen);
    int is_good = 1;

    printf("Generator quality evaluation:\n");

    // Condition 1: Minimum length check
    if (gen_len < 2) {
        printf(" -> [FAIL] Length is less than 2 bits.\n");
        is_good = 0;
    }

    // Condition 2: x^0 (last bit) coefficient check
    if (gen_len >= 1 && gen[gen_len - 1] != '1') {
        printf(" -> [FAIL] The x^0 coefficient (last bit) is not '1'. Cannot detect all single-bit errors.\n");
        is_good = 0;
    }

    // Condition 3: Last two bits check
    if (gen_len >= 2 && (gen[gen_len - 1] != '1' || gen[gen_len - 2] != '1')) {
        printf(" -> [FAIL] The last two bits are not both '1' (Violates structural check components).\n");
        is_good = 0;
    }

    if (is_good) {
        printf(" >>> STATUS: GOOD GENERATOR\n");
    } else {
        printf(" >>> STATUS: NOT A GOOD GENERATOR\n");
    }
}
