#include <bits/stdc++.h>
using namespace std;

struct Edge {
  int to;
  double base_cost;
  double cur_cost;
};

int main() {
ios::sync_with_stdio(false);
cin.tie(nullptr);
int S;
cout << "Enter number of stages: "<<flush;
cin >> S;
vector<int> stage_count(S);
long long N = 0;
cout << "Enter number of nodes in each stage (" << S << " values): "<<flush;
for (int i = 0; i < S; ++i) {
cin >> stage_count[i];
N += stage_count[i];
}
vector<int> stage_start(S);
int idx = 0;
for (int i = 0; i < S; ++i) {
stage_start[i] = idx;

idx += stage_count[i];
}
int M;
cout << "Enter number of edges: "<<flush;
cin >> M;
vector<vector<Edge>> adj(N);
vector<vector<int>> rev_adj(N);
cout << "Enter each edge as: u v cost\n"<<flush;
for (int i = 0; i < M; ++i) {
int u, v;
double cost;
cin >> u >> v >> cost;
if (u < 0 || u >= N || v < 0 || v >= N) {
cerr << "Invalid edge node id\n";
return 0;
}
Edge e{v, cost, cost};
adj[u].push_back(e);
rev_adj[v].push_back(u);
}
const double INF = 1e30;
vector<double> best_cost(N, INF);
vector<int> next_node(N, -1);
int last_stage = S - 1;
for (int k = 0; k < stage_count[last_stage]; ++k) {
int node = stage_start[last_stage] + k;
best_cost[node] = 0.0;
next_node[node] = -1;
}
for (int st = S - 2; st >= 0; --st) {
for (int k = 0; k < stage_count[st]; ++k) {
int u = stage_start[st] + k;
double best = INF;
int bestv = -1;
for (auto &e : adj[u]) {
int v = e.to;
double cost = e.cur_cost;
if (best_cost[v] + cost < best) {
best = best_cost[v] + cost;
bestv = v;
}

}
best_cost[u] = best;
next_node[u] = bestv;
}
}
cout << fixed << setprecision(6);
cout << "\nBest costs from Stage-0 nodes:\n";
for (int k = 0; k < stage_count[0]; ++k) {
int u = stage_start[0] + k;
if (best_cost[u] >= INF/2) cout << "Node " << u << ": unreachable\n";
else cout << "Node " << u << ": cost = " << best_cost[u] << "\n";
}
cout << "\nEnter a source node id (in stage 0) to print path, or -1 to skip: "<<flush;
int src;
cin >> src;
if (src >= 0 && src < N) {
if (best_cost[src] >= INF/2) {
cout << "No route from " << src << "\n";
} else {
cout << "Path from " << src << " : ";
int cur = src;
double total = 0.0;
while (cur != -1) {
cout << cur;
int nxt = next_node[cur];
if (nxt != -1) {
double ecost = 0;
bool found = false;
for (auto &e : adj[cur]) if (e.to == nxt) { ecost = e.cur_cost; found=true; break; }
if (!found) ecost = 0;
total += ecost;
cout << " -> ";
} else break;
cur = nxt;
}
cout << "\nTotal route cost (sum edges): " << total << "\n";
}
}
cout << "\nEnter number of live updates to edge costs (0 to finish): "<<flush;
int Q; cin >> Q;
while (Q-- > 0) {

int u, v;
double multiplier;
cout << "Enter edge update (u v multiplier): "<<flush;
cin >> u >> v >> multiplier;
for (auto &e : adj[u]) {
if (e.to == v) {
e.cur_cost = e.base_cost * multiplier;
}
}
auto recompute_node = [&](int node) -> double {
double best = INF;
int bestv = -1;
for (auto &e : adj[node]) {
if (best_cost[e.to] >= INF/2) continue;
double cand = e.cur_cost + best_cost[e.to];
if (cand < best) { best = cand; bestv = e.to; }
}
next_node[node] = bestv;
return best;
};
queue<int> q;
double newcost_u = recompute_node(u);
if (abs(newcost_u - best_cost[u]) > 1e-9) {
best_cost[u] = newcost_u;
q.push(u);
}
while (!q.empty()) {
int node = q.front(); q.pop();
for (int pred : rev_adj[node]) {
double newc = recompute_node(pred);
if (abs(newc - best_cost[pred]) > 1e-9) {
best_cost[pred] = newc;
q.push(pred);
}
}
}
}
cout << "\nAfter updates, best costs from Stage-0 nodes:\n";
for (int k = 0; k < stage_count[0]; ++k) {
int u = stage_start[0] + k;

if (best_cost[u] >= INF/2) cout << "Node " << u << ": unreachable\n";
else cout << "Node " << u << ": cost = " << best_cost[u] << "\n";
}
cout << "\nEnter a source node id (in stage 0) to print path, or -1 to skip: "<<flush;
cin >> src;
if (src >= 0 && src < N) {
if (best_cost[src] >= INF/2) {
cout << "No route from " << src << "\n";
} else {
cout << "Path from " << src << " : ";
int cur = src;
double total = 0.0;
while (cur != -1) {
cout << cur;
int nxt = next_node[cur];
if (nxt != -1) {
// find edge cost for display
double ecost = 0;
bool found = false;
for (auto &e : adj[cur]) if (e.to == nxt) { ecost = e.cur_cost; found=true; break; }
if (!found) ecost = 0;
total += ecost;
cout << " -> ";
} else break;
cur = nxt;
}
cout << "\nTotal route cost (sum edges): " << total << "\n";
}
}
return 0;
}
