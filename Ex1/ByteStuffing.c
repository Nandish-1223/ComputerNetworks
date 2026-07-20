#include <stdio.h>
#include <string.h>

#define MAX 100

int dataBytes[MAX];
int dataBits[MAX * 8];
int stuffedBytes[MAX * 2];
int sender_framed[MAX * 2];
int sender_framelen = 0;
int destuffedBytes[MAX];

int flagByte, escByte;

int bitsToByte(int bits[]) {
   int val = 0;
   int i;
   for (i = 0; i < 8; i++) {
      val = val * 2 + bits[i];
   }
   return val;
}

void printByteBinary(int Byte) {
   int i;
   for (i = 7; i >= 0; i--) {
      printf("%d", (Byte >> i) & 1);
   }
}

void printByteAsBinary(const char * label, int ass[], int n) {
   int i;
   printf("%s:", label);
   for (i = 0; i < n; i++) {
      printByteBinary(ass[i]);
      printf(" ");
   }
   printf("\n");
}

int byte_stuffing(int input_bytes[], int input_len, int output_bytes[]) {
   int j = 0;
   int i;
   for (i = 0; i < input_len; i++) {
      if (input_bytes[i] == flagByte || input_bytes[i] == escByte) {
         output_bytes[j++] = escByte;
         output_bytes[j++] = input_bytes[i];
      } else {
         output_bytes[j++] = input_bytes[i];
      }
   }
   return j;
}

int byte_destuffing(int input_framed[], int total_len, int output_bytes[]) {
   int m = 0;
   int i;

   if (input_framed[0] != flagByte || input_framed[total_len - 1] != flagByte) {
      printf("\nFrame Missing Start/End Flag\n");
      return -1;
   }

   for (i = 1; i < total_len - 1; i++) {
      if (input_framed[i] == escByte) {
         if (i + 1 >= total_len - 1) {
            printf("\nDangling ESC byte\n");
            return -1;
         }
         i++;
         output_bytes[m++] = input_framed[i];
      } else if (input_framed[i] == flagByte) {
         printf("\nUnexpected unescaped flag\n");
         return -1;
      } else {
         output_bytes[m++] = input_framed[i];
      }
   }
   return m;
}

void decode_bytes_to_string(int input_bytes[], int input_len, char target_str[]) {
   int i;
   for (i = 0; i < input_len; i++) {
      target_str[i] = (char)input_bytes[i];
   }
   target_str[input_len] = '\0';
}

int main() {
   int choice;
   int fbits[8], ebits[8];
   char message[MAX];
   int i;

   printf("Enter Flag Byte: ");
   for (i = 0; i < 8; i++) scanf("%d", &fbits[i]);
   flagByte = bitsToByte(fbits);

   printf("Enter ESC Byte: ");
   for (i = 0; i < 8; i++) scanf("%d", &ebits[i]);
   escByte = bitsToByte(ebits);
   getchar();

   while (1) {
      printf("1. Send Side\n");
      printf("2. Receive Side\n");
      printf("3. Exit\n");
      printf("Enter your choice : ");
      scanf("%d", &choice);
      getchar();

      if (choice == 3) {
         printf("Exiting...\n");
         break;
      }

      switch (choice) {
         case 1: {
            int numBytes;
            int bitIdx = 0;
            int stuffedLen;

            printf("\nEnter Message String: ");
            scanf("%[^\n]",&message);
            getchar();

            numBytes = strlen(message);

            for (i = 0; i < numBytes; i++) {
               int asciiVal = (int)message[i];
               int b;
               for (b = 7; b >= 0; b--) {
                  dataBits[bitIdx++] = (asciiVal >> b) & 1;
               }
            }

            for (i = 0; i < numBytes; i++) {
               int tmp[8];
               int b;
               for (b = 0; b < 8; b++) {
                  tmp[b] = dataBits[i * 8 + b];
               }
               dataBytes[i] = bitsToByte(tmp);
            }

            printByteAsBinary("Original Data", dataBytes, numBytes);

            stuffedLen = byte_stuffing(dataBytes, numBytes, stuffedBytes);
            printByteAsBinary("Stuffed Data ", stuffedBytes, stuffedLen);

            sender_framelen = 0;
            sender_framed[sender_framelen++] = flagByte;
            for (i = 0; i < stuffedLen; i++) {
               sender_framed[sender_framelen++] = stuffedBytes[i];
            }
            sender_framed[sender_framelen++] = flagByte;

            printByteAsBinary("Framed Data  ", sender_framed, sender_framelen);
            break;
         }

         case 2: {
            int rx_choice;
            int active_framed[MAX * 2];
            int active_framelen = 0;
            int destuffedLen;

            printf("1. Converted Data\n");
            printf("2. Enter Data\n");
            printf("Enter choice (1 or 2): ");
            scanf("%d", &rx_choice);
            getchar();

            if (rx_choice == 1) {
               if (sender_framelen == 0) {
                  printf("\nNo active Sender Data avaiable\n");
                  break;
               }
               active_framelen = sender_framelen;
               for (i = 0; i < sender_framelen; i++) {
                  active_framed[i] = sender_framed[i];
               }
               printByteAsBinary("\nSender Data", active_framed, active_framelen);
            }
            else if (rx_choice == 2) {
               printf("\nEnter total bytes:");
               scanf("%d", &active_framelen);

               printf("Enter each byte:\n");
               for (i = 0; i < active_framelen; i++) {
                  char binary_string[9];
                  int tmp_bits[8];
                  int b;
                  printf("Byte %d: ", i + 1);
                  scanf("%s", binary_string);

                  for (b = 0; b < 8; b++) {
                     tmp_bits[b] = binary_string[b] - '0';
                  }
                  active_framed[i] = bitsToByte(tmp_bits);
               }
               getchar();
               printByteAsBinary("\nEntered Data", active_framed, active_framelen);
            }
            else {
               printf("Invalid option\n");
               break;
            }

            destuffedLen = byte_destuffing(active_framed, active_framelen, destuffedBytes);

            if (destuffedLen == -1) {
               printf("Frame rejected due to transmission validation\n");
            } else {
               char outputString[MAX];
               printByteAsBinary("Destuffed Data", destuffedBytes, destuffedLen);

               decode_bytes_to_string(destuffedBytes, destuffedLen, outputString);

               printf("\nReceived Message String: %s\n", outputString);
            }
            break;
         }

         default:
            printf("Invalid\n");
      }
   }
   return 0;
}
