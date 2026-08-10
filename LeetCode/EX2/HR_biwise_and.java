import java.io.*;
import java.util.*;

public class Solution {
    public static void main(String[] args) {
        Scanner in = new Scanner(System.in);
        int t = in.nextInt(); 
        
        for (int i = 0; i < t; i++) {
            int n = in.nextInt();
            int k = in.nextInt();
            
            int target = k - 1;
            
            if ((target | (target + 1)) <= n) {
                System.out.println(target);
            } else {
                System.out.println(target - 1);
            }
        }
        in.close();
    }
}
