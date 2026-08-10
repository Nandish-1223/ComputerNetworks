class Solution 
{
    public int subarrayBitwiseORs(int[] arr) 
    {
      HashSet<Integer>ans=new HashSet<>();
       HashSet<Integer>prev=new HashSet<>();
        for(int i=0;i<arr.length;i++)
        {
            HashSet<Integer>curr=new HashSet<>();
            curr.add(arr[i]);
            for(int j:prev)
            {
                curr.add(j|arr[i]);
            }
            ans.addAll(curr);
            prev=curr;
        }
     return ans.size();
    }
}
