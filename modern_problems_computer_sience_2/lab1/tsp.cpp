#include <iostream>
#include <vector>
#include <algorithm>
#include <climits>
#include <chrono>
using namespace std;

void tsp(vector<vector<int>>& cost) {
    int n = cost.size();
    if (n <= 1) return;// n == 1 ? cost[0][0] : 0;
    
    // maximum cost to visit all cities
    const int INF = INT_MAX;
    int FULL = 1 << n, fullMask = FULL - 1;
    
    // dp[mask][i] represents the minimum cost to visit all cities
    // corresponding to the set bits in 'mask', ending at city 'i'
    vector<vector<int>> dp(FULL, vector<int>(n, INF));
    dp[1][0] = 0; 
    
    // iterate over all subsets of cities
    for (int mask = 1; mask < FULL; mask++) {
        for (int i = 0; i < n; i++) {
            
            // skip if city i is not included in mask
            if (!(mask & (1 << i))) continue;
            if (dp[mask][i] == INF) continue;
            
            // try to go to every unvisited city j
            for (int j = 0; j < n; j++) {
                
                // skip if city j is already visited
                if (mask & (1 << j)) continue;
                
                // cost to visit new city j from city i
                // such that previously visited cities
                // remain visited
                int nxt = mask | (1 << j);
                dp[nxt][j] = min(dp[nxt][j],  dp[mask][i] + cost[i][j]);
            }
        }
    }
    
    int ans = INF;
    for (int i = 0; i < n; i++) {
        
        // if last city on path is i and 
        // cost of path is not infinity
        if (dp[fullMask][i] != INF) 
            
            // update net cost such that city 0 is visited in last
            ans = min(ans, dp[fullMask][i] + cost[i][0]);
    }

    //return ans;
}

int main(int argc, char** argv) {
    if(argc!=2){
        cout<<"no arguments";
        return -4;
    }
    int n=stoi(argv[1]);

    vector<vector<int>> cost(n,vector<int>(n,0));
    for(int i=0;i<n;++i){
        for(int j=0;j<n;++j) cost[i][j]=rand()%100;
    }

    auto start_time=std::chrono::steady_clock::now();
    tsp(cost);
    auto end_time=std::chrono::steady_clock::now();
    auto res=std::chrono::duration_cast<std::chrono::milliseconds>(end_time-start_time);
    cout<<res.count()<<'\n';

    //int res = tsp(cost);
    //cout << res;
}