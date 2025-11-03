#include <bits/stdc++.h>
using namespace std;
// ---------- Brute Force (Recursive) ----------
int knapsackBruteForce(int W, vector<int>& wt, vector<int>& val, int n) {
if (n == 0 || W == 0)
return 0;
if (wt[n - 1] > W)
return knapsackBruteForce(W, wt, val, n - 1);
return max(val[n - 1] + knapsackBruteForce(W - wt[n - 1], wt, val, n - 1),
knapsackBruteForce(W, wt, val, n - 1));
}
// ---------- Dynamic Programming ----------
int knapsackDP(int W, vector<int>& wt, vector<int>& val, int n) {
vector<vector<int>> dp(n + 1, vector<int>(W + 1, 0));
for (int i = 1; i <= n; i++) {
for (int w = 1; w <= W; w++) {
if (wt[i - 1] <= w)
dp[i][w] = max(val[i - 1] + dp[i - 1][w - wt[i - 1]], dp[i - 1][w]);
else
dp[i][w] = dp[i - 1][w];
}
}
return dp[n][W];
}
// ---------- Greedy (approximation) ----------
double knapsackGreedy(vector<int>& wt, vector<int>& val, int W) {
int n = wt.size();
vector<pair<double, int>> ratio;
for (int i = 0; i < n; i++)
ratio.push_back({(double)val[i] / wt[i], i});
sort(ratio.rbegin(), ratio.rend());
double totalValue = 0.0;
int currentWeight = 0;

6

for (auto& p : ratio) {
int i = p.second;
if (currentWeight + wt[i] <= W) {
currentWeight += wt[i];
totalValue += val[i];
}
}
return totalValue;
}
int main() {
// Example data: (Weights in kg, Utilities in importance score)
vector<int> weight = {10, 20, 30, 15, 25}; // Example items
vector<int> utility = {60, 100, 120, 90, 75};
int W = 50; // Max truck capacity
int n = weight.size();
cout << "===== Disaster Relief Optimization using 0/1 Knapsack =====\n";
cout << "Truck Capacity (W): " << W << " kg\n";
cout << "Number of Items: " << n << "\n\n";
cout << "Item\tWeight\tUtility\n";
for (int i = 0; i < n; i++)
cout << i + 1 << "\t" << weight[i] << "\t" << utility[i] << "\n";
cout << "\n--- Results ---\n";
cout << "Brute Force Optimal Utility: " << knapsackBruteForce(W, weight, utility, n) << "\n";
cout << "DP Optimal Utility: " << knapsackDP(W, weight, utility, n) << "\n";
cout << "Greedy (Approx.) Utility: " << knapsackGreedy(weight, utility, W) << "\n";
cout << "\nNote: Brute force = exact but slow; DP = exact & efficient; Greedy = fast but
approximate.\n";
return 0;
}
