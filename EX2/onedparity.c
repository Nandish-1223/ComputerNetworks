#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STR 100
#define MAX_BITS 1024
#define FRAME_SIZE 7

void ascii_to_bin(char ch, char *b);
void str_to_bits(const char *str, char *bits);
int ask_parity_scheme();
char compute_parity_bit(int ones_in_data, int scheme);
int check_parity_error(int total_ones, int scheme);
char bin_to_ascii(const char *bits);
void extract_received_data(char tx_frames[][16], int num_frames, int total_bits, char *received_bits);
void bits_to_string(const char *bits, char *str);

int main() {
    char input[MAX_STR] = {0};
    char full_msg[MAX_BITS] = {0};
    char tx_frames[100][16] = {0};

    int scheme = 0;
    int total_bits = 0;
    int num_frames = 0;
    int data_ready = 0;

    int choice;

    while (1) {
        printf("\nMENU\n");
        printf(" 1. Sender Side \n");
        printf(" 2. Error Injection \n");
        printf(" 3. Receiver Side\n");
        printf(" 4. Exit\n");
        printf("---------------------------------------------\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("\nInvalid.\n");
            while (getchar() != '\n');
            continue;
        }

        switch (choice) {
            case 1: {
                printf("\nSENDER SIDE OPERATION\n");
                printf("Enter string message: ");
                scanf("%99s", input);

                scheme = ask_parity_scheme();
                str_to_bits(input, full_msg);
                total_bits = strlen(full_msg);
                num_frames = (total_bits + FRAME_SIZE - 1) / FRAME_SIZE;

                printf("\nFull Binary Message (%d bits):\n    %s\n", total_bits, full_msg);
                printf("\n--- Generated Frames ---\n");

                for (int i = 0; i < num_frames; i++) {
                    char current_frame[8] = {0};
                    int ones = 0;

                    for (int j = 0; j < FRAME_SIZE; j++) {
                        int idx = i * FRAME_SIZE + j;
                        if (idx < total_bits)
                            current_frame[j] = full_msg[idx];
                        else
                            current_frame[j] = '0'; // Padding

                        if (current_frame[j] == '1')
                            ones++;
                    }
                    current_frame[FRAME_SIZE] = '\0';

                    char parity_bit = compute_parity_bit(ones, scheme);
                    sprintf(tx_frames[i], "%s%c", current_frame, parity_bit);

                    printf(" Frame %2d | Data: %s | Parity: %c | Total Tx: %s\n",
                           i + 1, current_frame, parity_bit, tx_frames[i]);
                }

                data_ready = 1;
                break;
            }

            case 2: {
                if (!data_ready) {
                    printf("\nPlease run the Sender Side\n");
                    break;
                }

                int frame_no;
                printf("Enter frame number to corrupt (1-%d) or -1 for NO ERROR: ", num_frames);
                scanf("%d", &frame_no);

                if (frame_no >= 1 && frame_no <= num_frames) {
                    int flips;
                    printf("How many bits do you want to flip? :");
                    scanf("%d", &flips);

                    int frame_index = frame_no - 1;

                    for (int i = 0; i < flips; i++) {
                        int bit_pos;
                        printf("Enter bit position %d to flip (0-%d): ", i + 1, FRAME_SIZE);
                        scanf("%d", &bit_pos);

                        if (bit_pos >= 0 && bit_pos <= FRAME_SIZE) {
                            tx_frames[frame_index][bit_pos] = (tx_frames[frame_index][bit_pos] == '0') ? '1' : '0';
                            printf(" Bit %d flipped.\n", bit_pos);
                        } else {
                            printf("Invalid bit position.\n");
                            i--;
                        }
                    }
                    printf("\nModified Frame %d state: %s\n", frame_no, tx_frames[frame_index]);
                } else {
                    printf("\nNo errors or changes introduced to transmission medium.\n");
                }
                break;
            }

            case 3: {
                if (!data_ready) {
                    printf("\nPlease run the Sender Side\n");
                    break;
                }

                printf("\nRECEIVER SIDE\n");
                int global_error = 0;

                for (int i = 0; i < num_frames; i++) {
                    int ones = 0;
                    for (size_t j = 0; j < strlen(tx_frames[i]); j++) {
                        if (tx_frames[i][j] == '1')
                            ones++;
                    }

                    int error = check_parity_error(ones, scheme);
                    printf(" Frame %2d | Data+Parity: %s | Total 1s: %2d | Status: %s\n",
                           i + 1, tx_frames[i], ones, error ? "ERROR " : "OK");

                    if (error)
                        global_error = 1;
                }

                printf("\nFINAL RESULT\n");
                if (global_error) {
                    printf("TRANSMISSION FAILED: Parity violation detected.\n");
                } else {
                    printf("TRANSMISSION SUCCESSFUL: No parity violations detected.\n");

                    char received_bits[MAX_BITS] = {0};
                    char received_string[MAX_STR] = {0};

                    extract_received_data(tx_frames, num_frames, total_bits, received_bits);
                    bits_to_string(received_bits, received_string);

                    printf("\n--- Receiver Data Extraction ---\n");
                    printf(" Extracted Bits   : %s\n", received_bits);
                    printf(" Extracted String : %s\n", received_string);

                }

                    break;
            }

            case 4:
                printf("\nExiting program.\n");
                return 0;

            default:
                printf("\n Invalid option selection.\n");
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

void str_to_bits(const char *str, char *bits) {
    int len = strlen(str);
    char b[9];
    bits[0] = '\0';
    for (int i = 0; i < len; i++) {
        ascii_to_bin(str[i], b);
        strcat(bits, b);
    }
}

int ask_parity_scheme() {
    int choice;
    printf("Select Parity Scheme:\n 1. Even Parity\n 2. Odd Parity\n Choice: ");
    scanf("%d", &choice);
    return (choice == 2) ? 1 : 0;
}

char compute_parity_bit(int ones_in_data, int scheme) {
    if (scheme == 0)
        return (ones_in_data % 2 == 0) ? '0' : '1';
    else
        return (ones_in_data % 2 == 0) ? '1' : '0';
}

int check_parity_error(int total_ones, int scheme) {
    if (scheme == 0)
        return (total_ones % 2 != 0);
    else
        return (total_ones % 2 == 0);
}

char bin_to_ascii(const char *bits) {
    int value = 0;
    for (int i = 0; i < 8; i++) {
        value = value * 2 + (bits[i] - '0');
    }
    return (char)value;
}

void extract_received_data(char tx_frames[][16], int num_frames, int total_bits, char *received_bits) {
    int pos = 0;
    for (int i = 0; i < num_frames; i++) {
        for (int j = 0; j < FRAME_SIZE; j++) {
            if (pos < total_bits) {
                received_bits[pos] = tx_frames[i][j];
                pos++;
            }
        }
    }
    received_bits[pos] = '\0';
}

void bits_to_string(const char *bits, char *str) {
    int bit_len = strlen(bits);
    int char_count = bit_len / 8;
    for (int i = 0; i < char_count; i++) {
        char byte[9];
        for (int j = 0; j < 8; j++) {
            byte[j] = bits[i * 8 + j];
        }
        byte[8] = '\0';
        str[i] = bin_to_ascii(byte);
    }
    str[char_count] = '\0';
}
