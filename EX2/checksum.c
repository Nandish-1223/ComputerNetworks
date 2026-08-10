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
unsigned int bits_to_uint(const char *bits, int n);
void uint_to_bits(unsigned int v, int n, char *bits);
unsigned int add_1s_complement(unsigned int a, unsigned int b, int n);

/* --- Main Function --- */
int main() {
    // Shared State Variables
    char input[MAX_STR] = {0};
    char bits[MAX_BITS] = {0};
    char transmitted[MAX_BITS] = {0};
    char received[MAX_BITS] = {0};
    char checksum_bits[33] = {0};

    int n = 8;                 // Block size
    int len = 0;               // Padded length
    int original_len = 0;      // Length before padding
    int data_ready = 0;        // Track if sender side completed
    int error_staged = 0;      // Track if data passed to channel
    int choice;

    while (1) {
        printf("\nMENU\n");
        printf(" 1. Sender Side (Generate Checksum & Frame Data)\n");
        printf(" 2. Error Injection (Flip Channel Bits)\n");
        printf(" 3. Receiver Side (Verify Checksum & Extract)\n");
        printf(" 4. Exit\n");
        printf("--------------------------------------------------\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("\nInvalid input.\n");
            while (getchar() != '\n'); // Clear buffer
            continue;
        }

        switch (choice) {
            case 1: {
                printf("\nSENDER SIDE OPERATION\n");
                printf("Enter string message: ");
                scanf("%99s", input);

                str_to_bits(input, bits);
                original_len = strlen(bits);
                len = original_len;

                printf("Enter block size n: ");
                scanf("%d", &n);
                if (n <= 0 || n > 32) {
                    printf("Invalid block size. Defaulting to 8.\n");
                    n = 8;
                }

                // Apply Padding
                int pad = (n - (len % n)) % n;
                for (int i = 0; i < pad; i++) {
                    bits[len + i] = '0';
                }
                len += pad;
                bits[len] = '\0';

                printf("Source data (binary)    : %s\n", bits);
                printf("Data padded to multiple of %d bits\n", n);

                int nblocks = len / n;
                unsigned int sum = 0;

                printf("\n--- Processing Sender Blocks ---\n");
                for (int i = 0; i < nblocks; i++) {
                    char block[33];
                    strncpy(block, bits + i * n, n);
                    block[n] = '\0';

                    unsigned int val = bits_to_uint(block, n);
                    sum = add_1s_complement(sum, val, n);
                    printf(" Block %2d : %s (Value: %u)\n", i + 1, block, val);
                }

                char sum_bits[33];
                uint_to_bits(sum, n, sum_bits);
                printf("\nSum of all blocks       : %s\n", sum_bits);

                unsigned int mask = (n >= 32) ? 0xFFFFFFFFu : ((1u << n) - 1);
                unsigned int checksum = (~sum) & mask;
                uint_to_bits(checksum, n, checksum_bits);
                printf("Generated Checksum      : %s\n", checksum_bits);

                // Build full transmission string
                strcpy(transmitted, bits);
                strcat(transmitted, checksum_bits);
                printf("Data to be transmitted  : %s\n", transmitted);

                // Initialize receiver buffer with fresh data
                strcpy(received, transmitted);
                data_ready = 1;
                error_staged = 0; // Reset channel status
                break;
            }

            case 2: {
                if (!data_ready) {
                    printf("\nError: Run Sender Side.\n");
                    break;
                }

                printf("\nERROR INJECTION SIDE\n");
                int rlen = strlen(received);
                int pos;

                printf("Enter bit position to flip (0 to %d), or -1 for NO error: ", rlen - 1);
                scanf("%d", &pos);

                if (pos >= 0 && pos < rlen) {
                    received[pos] = (received[pos] == '0') ? '1' : '0';
                    printf("Success: Bit at position %d flipped.\n", pos);
                } else {
                    printf("No errors introduced into the stream.\n");
                }

                printf("Current stream heading to receiver: %s\n", received);
                error_staged = 1;
                break;
            }

            case 3: {
                if (!data_ready) {
                    printf("\nError: Run Sender Side.\n");
                    break;
                }

                printf("\nRECEIVER SIDE VERIFICATION\n");
                printf("Received data bitstream: %s\n", received);

                int rlen = strlen(received);
                int rblocks = rlen / n;
                unsigned int rsum = 0;

                printf("\n--- Processing Receiver Blocks (Data + Checksum) ---\n");
                for (int i = 0; i < rblocks; i++) {
                    char block[33];
                    strncpy(block, received + i * n, n);
                    block[n] = '\0';

                    unsigned int val = bits_to_uint(block, n);
                    rsum = add_1s_complement(rsum, val, n);

                    // Mark the last block explicitly as the checksum block
                    if (i == rblocks - 1) {
                        printf("Block %2d (Checksum): %s\n", i + 1, block);
                    } else {
                        printf("Block %2d (Data)    : %s\n", i + 1, block);
                    }
                }

                char rsum_bits[33];
                uint_to_bits(rsum, n, rsum_bits);
                printf("\nSum of all blocks       : %s\n", rsum_bits);

                unsigned int mask = (n >= 32) ? 0xFFFFFFFFu : ((1u << n) - 1);
                unsigned int rchecksum = (~rsum) & mask;
                char rchecksum_bits[33];
                uint_to_bits(rchecksum, n, rchecksum_bits);
                printf("Complement of sum       : %s\n", rchecksum_bits);

                printf("\nFINAL RESULT\n");
                if (rchecksum == 0) {
                    printf("Complement of sum is all zeros, Checksum valid.\n");
                    printf("Result : NO ERROR DETECTED\n");

                    char recovered_bits[MAX_BITS] = {0};
                    char recovered_str[MAX_STR] = {0};

                    // Extract payload discarding checksum block
                    strncpy(recovered_bits, received, len);
                    recovered_bits[len] = '\0';

                    bits_to_str(recovered_bits, recovered_str);
                    printf("Recovered message (string) : %s\n", recovered_str);
                } else {
                    printf("Complement of sum is NOT all zeros (%s), Mismatch!\n", rchecksum_bits);
                    printf("Result : ERROR DETECTED\n");
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

unsigned int bits_to_uint(const char *bits, int n) {
    unsigned int v = 0;
    for (int i = 0; i < n; i++) {
        v = (v << 1) | (bits[i] - '0');
    }
    return v;
}

void uint_to_bits(unsigned int v, int n, char *bits) {
    for (int i = n - 1; i >= 0; i--) {
        bits[i] = (v & 1) ? '1' : '0';
        v >>= 1;
    }
    bits[n] = '\0';
}

unsigned int add_1s_complement(unsigned int a, unsigned int b, int n) {
    unsigned int mask = (n >= 32) ? 0xFFFFFFFFu : ((1u << n) - 1);
    unsigned int sum = a + b;
    if (sum > mask) {
        sum = (sum & mask) + 1; // End-around carry
    }
    return sum & mask;
}
