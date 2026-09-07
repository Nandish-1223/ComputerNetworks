#include <stdio.h>
#include <stdlib.h>

#define MAX 50

void stopAndWait(int frames, int lostFrame) {
    int Sn = 0;
    int step = 1, total = 0;

    printf("\n================ STOP-AND-WAIT SIMULATION ================\n");

    while (Sn < frames) {
        total++;
        printf("[Step %d] Current Window: [%d]\n", step++, Sn);

        if (Sn == lostFrame) {
            printf("  -> Transmission: Sending Frame %d\n", Sn);
            printf("  -> Status      : LOST\n");
            printf("  -> Event       : Timer expired! *** TIMEOUT ***\n");
            printf("  -> Action      : Resending the same frame.\n\n");
            lostFrame = -1;
        } else {
            printf("  -> Transmission: Sending Frame %d\n", Sn);
            printf("  -> Status      : SUCCESS (ACK %d Received)\n", Sn + 1);
            printf("  -> Action      : Frame %d delivered. Sliding window forward.\n\n", Sn);
            Sn++;
        }
    }

    printf("==========================================================\n");
    printf("Simulation Completed!\n");
    printf("Total Frames Delivered : %d\n", frames);
    printf("Total Transmissions    : %d\n", total);
    printf("==========================================================\n\n");
}

void goBackN(int frames, int N, int lostFrame) {
    int Sb = 0;
    int Sm;
    int Sn;
    int step = 1, total = 0;

    printf("\n================ GO-BACK-N SIMULATION ================\n");

    while (Sb < frames) {
        Sm = Sb + N - 1;
        int current_limit = (Sm < frames - 1 ? Sm : frames - 1);
        int lost_occurred = 0;
        int target_lost_frame = -1;

        printf("[Step %d] Current Window: [", step++);
        for (int i = Sb; i <= current_limit; i++) {
            printf("%d%s", i, (i == current_limit) ? "" : ", ");
        }
        printf("]\n");

        for (Sn = Sb; Sn <= Sm && Sn < frames; Sn++) {
            total++;
            printf("  -> Transmission: Sending Frame %d\n", Sn);

            if (Sn == lostFrame) {
                printf("  -> Status      : LOST\n");
                printf("  -> Event       : Receiver discards frame %d and all subsequent frames!\n", Sn);
                lostFrame = -1;
                lost_occurred = 1;
                target_lost_frame = Sn;
                break;
            } else {
                printf("  -> Status      : SUCCESS (ACK %d Received)\n", Sn + 1);
                if (!lost_occurred) {
                    Sb++;
                }
            }
        }

        if (lost_occurred) {
            printf("  -> Event       : *** TIMEOUT *** for missing Frame %d\n", target_lost_frame);
            printf("  -> Action      : Go-Back-N triggered! Rolling back window base to frame %d.\n\n", target_lost_frame);
            Sb = target_lost_frame;
        } else {
            printf("  -> Action      : Sliding window forward.\n\n");
        }
    }

    printf("======================================================\n");
    printf("Simulation Completed!\n");
    printf("Total Frames Delivered : %d\n", frames);
    printf("Total Transmissions    : %d\n", total);
    printf("======================================================\n\n");
}

void selectiveRepeat(int frames, int N, int lostFrame) {
    int Sb = 0;
    int step = 1, total = 0;

    int *acked = (int *)calloc(frames, sizeof(int));

    printf("\n============ SELECTIVE REPEAT SIMULATION ============\n");

    while (Sb < frames) {
        printf("[Step %d] Current Window: [", step++);

        // Dynamically find and print up to N un-ACKed or upcoming valid frames
        int printed_count = 0;
        int last_valid_idx = -1;

        // First pass: Find the last index we will print to handle formatting commas correctly
        for (int i = Sb; i < frames && printed_count < N; i++) {
            if (i == Sb || acked[i] == 0) {
                last_valid_idx = i;
                printed_count++;
            }
        }

        printed_count = 0;
        for (int i = Sb; i < frames && printed_count < N; i++) {
            // Always display the base frame (even if ACKed but not slid yet), or any unACKed lookahead frames
            if (i == Sb || acked[i] == 0) {
                printf("%d%s", i, (i == last_valid_idx) ? "" : ", ");
                printed_count++;
            }
        }
        printf("]\n");

        // Transmit the active frames that are inside this current window layout pass
        int transmit_count = 0;
        for (int Sn = Sb; Sn < frames && transmit_count < N; Sn++) {
            if (Sn != Sb && acked[Sn] == 1) {
                continue; // Skip already acknowledged frames from transmission list
            }

            transmit_count++;

            // If the base frame itself was already ACKed, skip sending but keep it as window filler
            if (Sn == Sb && acked[Sn] == 1) {
                continue;
            }

            total++;
            printf("  -> Transmission: Sending Frame %d\n", Sn);

            if (Sn == lostFrame) {
                printf("  -> Status      : LOST\n");
                printf("  -> Action      : Frame %d lost. Receiver skips buffering. Will retry later.\n", Sn);
                lostFrame = -1;
            } else {
                printf("  -> Status      : SUCCESS (ACK %d Received)\n", Sn + 1);
                acked[Sn] = 1;
                printf("  -> Action      : Frame %d buffered by receiver.\n", Sn);
            }
        }

        // Slide the window base past all consecutively acknowledged frames
        int oldSb = Sb;
        while (Sb < frames && acked[Sb] == 1) {
            Sb++;
        }

        if (Sb > oldSb) {
            printf("  -> Window Slide: Base frame(s) ACKed. Window moved forward to base %d.\n\n", Sb);
        } else {
            printf("  -> Window Slide: Base frame %d still missing ACK. Retrying unACKed window frames...\n\n", Sb);
        }
    }

    printf("=====================================================\n");
    printf("Simulation Completed!\n");
    printf("Total Frames Delivered : %d\n", frames);
    printf("Total Transmissions    : %d\n", total);
    printf("=====================================================\n\n");

    free(acked);
}

int main() {
    int choice;
    int n, w, lost;

    do {
        printf("========== SLIDING WINDOW MENU ==========\n");
        printf("  1. Stop-and-Wait\n");
        printf("  2. Go-Back-N\n");
        printf("  3. Selective Repeat\n");
        printf("  4. Exit\n");
        printf("==========================================\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        printf("\n");

        if (choice == 4) {
            printf("Exiting...\n");
            break;
        }
        if (choice < 1 || choice > 4) {
            printf("Invalid choice! Try again.\n\n");
            continue;
        }

        printf("No. of frames  : ");
        scanf("%d", &n);
        printf("Frame to lose  : ");
        scanf("%d", &lost);

        if (choice != 1) {
            printf("Window size (N): ");
            scanf("%d", &w);
        } else {
            w = 1;
        }

        if (n <= 0 || n > MAX || w <= 0 || w > n) {
            printf("\nInvalid input configurations!\n\n");
            continue;
        }
        if (lost >= n) {
            printf("\nInvalid lost frame number!\n\n");
            continue;
        }

        switch (choice) {
            case 1: stopAndWait(n, lost); break;
            case 2: goBackN(n, w, lost); break;
            case 3: selectiveRepeat(n, w, lost); break;
        }
    } while (choice != 4);

    return 0;
}
