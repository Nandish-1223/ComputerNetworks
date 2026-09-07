class Solution {
public:
    int networkDelayTime(vector<vector<int>>& times, int n, int k) {
        vector<vector<pair<int, int>>> adjList(n + 1);
        for (auto& edge : times) {
            int u = edge[0];
            int v = edge[1];
            int weight = edge[2];

            adjList[u].push_back({v, weight});
        }

        priority_queue<pair<int,int>,vector<pair<int,int>>,greater<pair<int,int>>>pq;
        vector<int>dist(n+1,INT_MAX);
        dist[k] = 0;
        pq.push({0,k});

        while(!pq.empty()){
            auto[distance,node] = pq.top();
            pq.pop();

            if(distance > dist[node]) continue;

            for(auto [neighbor,weight] : adjList[node]){
                if(distance + weight < dist[neighbor]){
                    dist[neighbor] = distance + weight;
                    pq.push({dist[neighbor],neighbor});
                }
            }
        }
        int ans = 0;

        for(int i = 1; i <= n; i++) {

            if(dist[i] == INT_MAX)
                return -1;

            ans = max(ans, dist[i]);
        }

        return ans;
    }
};
