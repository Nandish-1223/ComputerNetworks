import java.util.*;

class Solution {
    public double[] medianSlidingWindow(int[] nums, int k) {
        int n = nums.length;
        double[] result = new double[n - k + 1];
        PriorityQueue<Integer> minHeap = new PriorityQueue<>();
        PriorityQueue<Integer> maxHeap = new PriorityQueue<>(Collections.reverseOrder());
        Map<Integer, Integer> invalidElements = new HashMap<>();

        for (int i = 0; i < k; i++) {
            maxHeap.add(nums[i]);
        }
        for (int i = 0; i < k / 2; i++) {
            minHeap.add(maxHeap.poll());
        }

        result[0] = getMedian(maxHeap, minHeap, k);

        for (int i = k; i < n; i++) {
            int outNum = nums[i - k];
            int inNum = nums[i];
            int balance = 0;

            balance += (outNum <= maxHeap.peek()) ? -1 : 1;
            invalidElements.put(outNum, invalidElements.getOrDefault(outNum, 0) + 1);

            if (!maxHeap.isEmpty() && inNum <= maxHeap.peek()) {
                balance++;
                maxHeap.add(inNum);
            } else {
                balance--;
                minHeap.add(inNum);
            }

            if (balance < 0) {
                maxHeap.add(minHeap.poll());
                balance++;
            }
            if (balance > 0) {
                minHeap.add(maxHeap.poll());
                balance--;
            }

            while (!maxHeap.isEmpty() && invalidElements.getOrDefault(maxHeap.peek(), 0) > 0) {
                int top = maxHeap.poll();
                invalidElements.put(top, invalidElements.get(top) - 1);
            }
            while (!minHeap.isEmpty() && invalidElements.getOrDefault(minHeap.peek(), 0) > 0) {
                int top = minHeap.poll();
                invalidElements.put(top, invalidElements.get(top) - 1);
            }

            result[i - k + 1] = getMedian(maxHeap, minHeap, k);
        }

        return result;
    }

    private double getMedian(PriorityQueue<Integer> maxHeap, PriorityQueue<Integer> minHeap, int k) {
        if (k % 2 == 1) {
            return (double) maxHeap.peek();
        } else {
            return ((double) maxHeap.peek() + (double) minHeap.peek()) * 0.5;
        }
    }
}
