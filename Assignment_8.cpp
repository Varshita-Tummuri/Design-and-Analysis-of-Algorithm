#include <bits/stdc++.h>
using namespace std;
const double INF_D = 1e18;

struct Node {
    vector<vector<double>> reduced; // reduced cost matrix
    double cost;     
    int level;      
    int city;       
    vector<int> path; 
    double timeSoFar; 
    double optimisticTimeLB; 
  
    bool operator<(Node const& o) const { return cost > o.cost; } 
};

double reduceMatrix(vector<vector<double>>& mat) {
    int n = mat.size();
    double red = 0.0;
    for (int i = 0; i < n; ++i) {
        double rowMin = INF_D;
        for (int j = 0; j < n; ++j) rowMin = min(rowMin, mat[i][j]);
        if (rowMin > 0 && rowMin < INF_D) {
            red += rowMin;
            for (int j = 0; j < n; ++j) if (mat[i][j] < INF_D) mat[i][j] -= rowMin;
        }
    }
    for (int j = 0; j < n; ++j) {
        double colMin = INF_D;
        for (int i = 0; i < n; ++i) colMin = min(colMin, mat[i][j]);
        if (colMin > 0 && colMin < INF_D) {
            red += colMin;
            for (int i = 0; i < n; ++i) if (mat[i][j] < INF_D) mat[i][j] -= colMin;
        }
    }
    return red;
}

pair<double, vector<int>> greedy_initial(const vector<vector<double>>& costMat, int start) {
    int n = costMat.size();
    vector<int> visited(n, 0);
    vector<int> path;
    path.reserve(n+1);
    int cur = start;
    path.push_back(cur);
    visited[cur] = 1;
    double total = 0.0;
    for (int step = 1; step < n; ++step) {
        int next = -1;
        double best = INF_D;
        for (int j = 0; j < n; ++j) {
            if (!visited[j] && costMat[cur][j] < best) { best = costMat[cur][j]; next = j; }
        }
        if (next == -1) return {INF_D, {}};
        total += costMat[cur][next];
        cur = next;
        visited[cur] = 1;
        path.push_back(cur);
    }
    if (costMat[cur][start] >= INF_D) return {INF_D, {}};
    total += costMat[cur][start];
    path.push_back(start);
    return {total, path};
}

double optimistic_time_lower_bound(const vector<vector<double>>& timeMat, const vector<int>& visitedMaskVec, int curCity) {
    int n = timeMat.size();
    vector<int> visited(n, 0);
    for (int v : visitedMaskVec) visited[v] = 1;
    visited[curCity] = 1;
    double lb = 0.0;
   
    for (int i = 0; i < n; ++i) {
        if (visited[i]) continue;
        double mn = INF_D;
        for (int j = 0; j < n; ++j) {
            if (i==j) continue;
            if (timeMat[i][j] < mn) mn = timeMat[i][j];
        }
        if (mn >= INF_D) return INF_D; // unreachable
        lb += mn; // optimistic (underestimate)
    }
      return lb;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int N;
    if (!(cin >> N)) {
        cerr << "Failed to read N\n";
        return 1;
    }
    vector<vector<double>> dist(N, vector<double>(N));
    vector<vector<double>> fuelFactor(N, vector<double>(N));
    vector<vector<double>> timeMat(N, vector<double>(N));
    for (int i = 0; i < N; ++i) for (int j = 0; j < N; ++j) cin >> dist[i][j];
    for (int i = 0; i < N; ++i) for (int j = 0; j < N; ++j) cin >> fuelFactor[i][j];
    for (int i = 0; i < N; ++i) for (int j = 0; j < N; ++j) cin >> timeMat[i][j];
    double timeLimit; cin >> timeLimit;
    int start = 0;
    if (!cin.eof()) {
        if (cin >> ws && !cin.eof()) {
            if ((cin >> start).fail()) start = 0;
        }
    }
    vector<vector<double>> costMat(N, vector<double>(N, INF_D));
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            if (i == j) continue; // prohibit self-loop
                 costMat[i][j] = dist[i][j] * fuelFactor[i][j];
            if (!(costMat[i][j] >= 0.0) || isnan(costMat[i][j]) ) costMat[i][j] = INF_D;
        }
    }
    auto greedy = greedy_initial(costMat, start);
    double bestCost = greedy.first;
    vector<int> bestPath = greedy.second;
    if (bestCost >= INF_D) {
       
        bestCost = INF_D;
    }

    vector<vector<double>> initMat = costMat;
    double initReduction = reduceMatrix(initMat);
    Node root;
    root.reduced = initMat;
    root.cost = initReduction;
    root.level = 0;
    root.city = start;
    root.path = {start};
    root.timeSoFar = 0.0;
    root.optimisticTimeLB = optimistic_time_lower_bound(timeMat, root.path, root.city);

    priority_queue<Node> pq;
    pq.push(root);

    double nodesExpanded = 0;
    double startTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();

    vector<int> finalPath;
    double finalCost = INF_D;

    vector<double> minOutTime(N, INF_D);
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) if (i != j) minOutTime[i] = min(minOutTime[i], timeMat[i][j]);
    }

    while (!pq.empty()) {
        Node node = pq.top(); pq.pop();
        nodesExpanded++;
      
        if (node.cost >= bestCost) continue;
        double optimisticRemainingTime = optimistic_time_lower_bound(timeMat, node.path, node.city);
        if (node.timeSoFar + optimisticRemainingTime > timeLimit) continue;
        if (node.level == N - 1) {
            if (costMat[node.city][start] >= INF_D) continue; // can't close
            double finalTourCost = node.cost + node.reduced[node.city][start]; 
         
            double realCost = 0.0;
            for (size_t i = 0; i + 1 < node.path.size(); ++i) {
                realCost += costMat[node.path[i]][node.path[i+1]];
            }
            realCost += costMat[node.city][start];
            double totalTime = node.timeSoFar + timeMat[node.city][start];
            if (totalTime <= timeLimit && realCost < bestCost) {
                bestCost = realCost;
                finalCost = realCost;
                finalPath = node.path;
                finalPath.push_back(start);
            }
            continue;
        }
        for (int next = 0; next < N; ++next) {
            // skip if visited already
            bool visited = false;
            for (int v : node.path) if (v == next) { visited = true; break; }
            if (visited) continue;
            if (node.reduced[node.city][next] >= INF_D) continue; // no edge

            // Construct child node
            Node child;
            child.reduced = node.reduced; // copy
            // Add cost: cost so far + reduced cost of chosen edge
            double addedCost = node.reduced[node.city][next];
            child.cost = node.cost + addedCost;
            child.level = node.level + 1;
            child.city = next;
            child.path = node.path;
            child.path.push_back(next);
            // update timeSoFar
            child.timeSoFar = node.timeSoFar + timeMat[node.city][next];

            // Quick time feasibility: optimistic lower bound for remaining times
            child.optimisticTimeLB = optimistic_time_lower_bound(timeMat, child.path, child.city);
            if (child.timeSoFar + child.optimisticTimeLB > timeLimit) continue; // prune on time

            // Modify reduced matrix: set row of node.city to INF, col of next to INF, and (next,start) ??? per Little's algorithm
            int n = N;
            for (int j = 0; j < n; ++j) child.reduced[node.city][j] = INF_D;
            for (int i = 0; i < n; ++i) child.reduced[i][next] = INF_D;
            // Also block return edge that would create subtours: set next->start? Standard approach is set next->start to INF only when necessary.
            // Little's method sets child.reduced[next][start] = INF if it creates subtour - but detecting subtours requires more logic.
            // A simpler safe approach: disallow edges that return to an already visited vertex other than finishing; we've already done visited check.
            // Now reduce matrix and add reduction to child.cost
            double red = reduceMatrix(child.reduced);
            child.cost += red;

            // If child lower bound >= bestCost, prune
            if (child.cost >= bestCost) continue;

            // If child optimistic time exceeds time limit, prune
            if (child.timeSoFar + child.optimisticTimeLB > timeLimit) continue;

            // Otherwise push into PQ
            pq.push(child);
        }
    } // pq loop

    double endTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
    double elapsedMs = endTime - startTime;

    // Output results
    cout << fixed << setprecision(6);
    if (finalPath.empty()) {
        cout << "No feasible tour found within time limit.\n";
        if (bestCost < INF_D) {
            cout << "Best feasible (greedy) cost: " << bestCost << " (path shown)\n";
            for (size_t i = 0; i < greedy.second.size(); ++i) {
                cout << greedy.second[i] << (i+1<greedy.second.size()? " -> " : "\n");
            }
        }
    } else {
        cout << "Optimal tour cost: " << finalCost << "\n";
        cout << "Tour path (cities): ";
        for (size_t i = 0; i < finalPath.size(); ++i) {
            cout << finalPath[i] << (i+1<finalPath.size()? " -> " : "\n");
        }
    }
    cout << "Nodes expanded: " << nodesExpanded << "\n";
    cout << "Time used (ms): " << elapsedMs << "\n";
    return 0;
}


