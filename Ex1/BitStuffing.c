#include<stdio.h>
#include<string.h>

#define max 100
#define max_bit (max*8)
#define max_stuff (max_bit*2)

int data[max_bit], stuff[max_stuff], destuff[max_bit];
int sender_framed[max_stuff];
int sender_framelen = 0;

char str[max];
char output_str[max];
int flag[]={0,1,1,1,1,1,1,0};
int flaglen=8;

void print(char *label, int arr[], int n){
   int i;
   printf("%s ", label);
   for(i=0; i<n; i++){
      printf("%d", arr[i]);
      if((i+1) % 8 == 0){
         printf(" ");
      }
   }
   printf("\n");
}

int bit_stuffing(int input_bits[], int input_len, int output_bits[]) {
   int i,ones = 0, j = 0;
   for(i=0; i<input_len; i++){
      output_bits[j++] = input_bits[i];
      ones = (input_bits[i] == 1) ? ones + 1 : 0;
      if(ones == 5){
         output_bits[j++] = 0;
         ones = 0;
      }
   }
   return j;
}

int bit_destuffing(int input_framed[], int total_len, int output_bits[]) {
   int i,ones = 0, j = 0;
   for(i=flaglen; i<total_len-flaglen; i++){
      output_bits[j++] = input_framed[i];
      ones = (input_framed[i] == 1) ? ones + 1 : 0;
      if(ones == 5){
         if(input_framed[i+1] != 0){
            printf("\nError: Invalid stuffing detected\n");
            return -1;
         }
         i++;
         ones = 0;
      }
   }
   return j;
}

void decode_binary_to_string(int input_bits[], int input_len, char target_str[]) {
   int i,b,out_idx = 0;
   for(i=0; i<input_len; i+=8){
      char ch = 0;
      for(b=0; b<8; b++){
         ch = (ch << 1) | input_bits[i+b];
      }
      target_str[out_idx++] = ch;
   }
   target_str[out_idx] = '\0';
}

int main(){
   int choice;

   while(1) {
      printf("1. Send Side\n");
      printf("2. Receive Side\n");
      printf("3. Exit\n");
      printf("Enter your choice: ");
      scanf("%d", &choice);
      getchar();

      if(choice == 3) {
         printf("Exiting...\n");
         break;
      }

      switch(choice) {
         case 1: {
            int i,b,bit = 0;
            printf("\nEnter string to send: ");
            scanf("%[^\n]", str);
            getchar();

            int str_len = strlen(str);
            for(i=0; i<str_len; i++){
               char ch = str[i];
               for(b=7; b>=0; b--){
                  data[bit++] = (ch >> b) & 1;
               }
            }
            print("\nData: ", data, bit);

            int stufflen = bit_stuffing(data, bit, stuff);
            print("Stuffed Bits:    ", stuff, stufflen);

            sender_framelen = 0;
            for(i=0; i<flaglen; i++) sender_framed[sender_framelen++] = flag[i];
            for(i=0; i<stufflen; i++) sender_framed[sender_framelen++] = stuff[i];
            for(i=0; i<flaglen; i++) sender_framed[sender_framelen++] = flag[i];

            print("Framed Bits:", sender_framed, sender_framelen);
            break;
         }

         case 2: {
            int i,rx_choice;
            int active_framed[max_stuff];
            int active_framelen = 0;

            printf("1. Coverted Data\n");
            printf("2. Enter Data\n");
            printf("Enter choice : ");
            scanf("%d", &rx_choice);
            getchar();

            if(rx_choice == 1) {
               if(sender_framelen == 0) {
                  printf("\nNo sender data available\n");
                  break;
               }
               active_framelen = sender_framelen;
               for(i=0; i<sender_framelen; i++) {
                  active_framed[i] = sender_framed[i];
               }
               print("\nSender Data: ", active_framed, active_framelen);
            }
            else if(rx_choice == 2) {
               char bit_string[max_stuff];
               printf("\nEnter receiver Data:\n");
               scanf("%s", bit_string);

               active_framelen = strlen(bit_string);
               for(i=0; i<active_framelen; i++) {
                  active_framed[i] = bit_string[i] - '0';
               }
            }
            else {
               printf("Invalid selection choice\n");
               break;
            }

            int destufflen = bit_destuffing(active_framed, active_framelen, destuff);

            if(destufflen == -1) {
               printf("Frame rejected due to transmission error\n");
            } else {
               print("\nDestuffed Bits:  ", destuff, destufflen);

               decode_binary_to_string(destuff, destufflen, output_str);
               printf("Decoded String:  %s\n", output_str);
            }
            break;
         }

         default:
            printf("Invalid main choice option\n");
      }
   }

   return 0;
}
