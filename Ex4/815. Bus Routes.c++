class Solution {
public:
    int numBusesToDestination(vector<vector<int>>& routes, int source, int target) {
        // stop : route number
        unordered_map<int,vector<int>> mp;
        
        for(int i = 0; i<routes.size(); i++){
            for(int j : routes[i]){
                mp[j].push_back(i);
            }
        }
        //stop s, min buses taken to reach stop s
        queue<pair<int,int>>q;

        q.push({source, 0});

        //to store visited stops
        unordered_set<int>vis = {source};
        while(!q.empty()){
            int s = q.front().first;
            int buses = q.front().second;

            q.pop();

            if(s == target) return buses;

            for(int i : mp[s]){
                for(int j : routes[i]){
                    if(vis.find(j) == vis.end()){
                        vis.insert(j);
                        q.push({j, buses+1});
                    }
                }
                routes[i].clear();
            }
        }
        return -1;
    }
};
