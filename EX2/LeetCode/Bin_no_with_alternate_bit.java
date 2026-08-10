class Solution {
    public boolean hasAlternatingBits(int n) {
        int shifted = n >> 1;
        int xorSum = n ^ shifted;
        return (xorSum & (xorSum + 1)) == 0;
    }
}
