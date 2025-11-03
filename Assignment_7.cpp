#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using chrono_clock = chrono::high_resolution_clock;

struct Result {
    vector<int> color; // color per vertex (0..k-1), -1 if uncolored
    int numColors;
    double timeMs;
};

vector<vector<int>> build_graph(int N, const vector<vector<int>>& student_courses) {
    vector<unordered_set<int>> adjset(N);
    for (const auto& courses : student_courses) {
        int k = (int)courses.size();
        for (int i = 0; i < k; ++i) {
            for (int j = i + 1; j < k; ++j) {
                int a = courses[i], b = courses[j];
                if (a == b) continue;
                adjset[a].insert(b);
                adjset[b].insert(a);
            }
        }
    }
    vector<vector<int>> adj(N);
    for (int i = 0; i < N; ++i) {
        adj[i].reserve(adjset[i].size());
        for (int v : adjset[i]) adj[i].push_back(v);
        // sort adjacency for determinism
        sort(adj[i].begin(), adj[i].end());
    }
    return adj;
}

// Basic greedy coloring with a specified vertex order
Result greedy_coloring(const vector<vector<int>>& adj, const vector<int>& order) {
    int n = (int)adj.size();
    vector<int> color(n, -1);
    vector<int> used; used.reserve(n);
    vector<int> mark(n, -1);
    auto t0 = chrono_clock::now();

    for (int idx = 0; idx < n; ++idx) {
        int u = order.empty() ? idx : order[idx];
        // mark neighbor colors
        used.clear();
        for (int v : adj[u]) {
            if (color[v] != -1 && mark[color[v]] != u) {
                mark[color[v]] = u;
                used.push_back(color[v]);
            }
        }
        // find smallest non-used color
        int c = 0;
        if (!used.empty()) {
            sort(used.begin(), used.end());
            for (int x : used) {
                if (x == c) ++c; else if (x > c) break;
            }
        }
        color[u] = c;
    }

    int maxc = -1;
    for (int c: color) if (c > maxc) maxc = c;
    auto t1 = chrono_clock::now();
    double ms = chrono::duration_cast<chrono::duration<double, milli>>(t1 - t0).count();
    return {color, maxc + 1, ms};
}

// Welsh-Powell: sort vertices by decreasing degree then greedy
Result welsh_powell(const vector<vector<int>>& adj) {
    int n = (int)adj.size();
    vector<pair<int,int>> deg_v;
    deg_v.reserve(n);
    for (int i = 0; i < n; ++i) deg_v.emplace_back((int)adj[i].size(), i);
    sort(deg_v.begin(), deg_v.end(), [](const auto& a, const auto& b){
        if (a.first != b.first) return a.first > b.first;
        return a.second < b.second;
    });
    vector<int> order;
    order.reserve(n);
    for (auto &p : deg_v) order.push_back(p.second);
    return greedy_coloring(adj, order);
}

// DSATUR implementation
Result dsatur_coloring(const vector<vector<int>>& adj) {
    int n = (int)adj.size();
    vector<int> color(n, -1);
    vector<unordered_set<int>> neigh_colors(n); // set of colors among neighbors
    vector<int> degree(n);
    for (int i = 0; i < n; ++i) degree[i] = (int)adj[i].size();

    auto t0 = chrono_clock::now();

    int colored = 0;
    // choose first vertex: highest degree
    int first = max_element(degree.begin(), degree.end()) - degree.begin();
    color[first] = 0;
    ++colored;
    for (int v : adj[first]) neigh_colors[v].insert(0);

    while (colored < n) {
        // select uncolored vertex with highest saturation degree (size of neigh_colors), tie break by degree
        int best = -1;
        int best_sat = -1;
        int best_deg = -1;
        for (int v = 0; v < n; ++v) {
            if (color[v] != -1) continue;
            int sat = (int)neigh_colors[v].size();
            if (sat > best_sat || (sat == best_sat && degree[v] > best_deg) ) {
                best = v; best_sat = sat; best_deg = degree[v];
            }
        }
        // assign smallest available color for best
        int c = 0;
        while (neigh_colors[best].count(c)) ++c;
        color[best] = c;
        ++colored;
        for (int v : adj[best]) {
            if (color[v] == -1) neigh_colors[v].insert(c);
        }
    }

    int maxc = -1;
    for (int c: color) if (c > maxc) maxc = c;
    auto t1 = chrono_clock::now();
    double ms = chrono::duration_cast<chrono::duration<double, milli>>(t1 - t0).count();
    return {color, maxc + 1, ms};
}

// Compute course sizes (enrollment) from student enrollments
vector<int> compute_course_sizes(int N, const vector<vector<int>>& student_courses) {
    vector<int> sizes(N, 0);
    for (const auto& courses : student_courses) {
        for (int c: courses) ++sizes[c];
    }
    return sizes;
}

unordered_map<int, vector<int>> allocate_rooms_for_slot(
    vector<int> rooms, // capacities; will be mutated as remaining capacities
    const vector<int>& room_ids, // indices corresponding to original rooms
    const vector<int>& exams,
    const vector<int>& course_sizes)
{
    // Build a vector of pairs (size, course)
    vector<pair<int,int>> ex;
    for (int c : exams) ex.emplace_back(course_sizes[c], c);
    // sort descending by size to pack big exams first
    sort(ex.begin(), ex.end(), greater<>());

    unordered_map<int, vector<int>> allocation;
    // build vector of (cap, id) for rooms sorted desc
    vector<pair<int,int>> roomVec;
    for (size_t i = 0; i < rooms.size(); ++i) roomVec.emplace_back(rooms[i], room_ids[i]);
    sort(roomVec.begin(), roomVec.end(), greater<>());

    // Check global feasibility quickly
    ll totalCapacity = 0;
    for (auto &p : roomVec) totalCapacity += p.first;
    ll totalStudents = 0;
    for (auto &p : ex) totalStudents += p.first;
    if (totalCapacity < totalStudents) {
        // cannot fit
        return {};
    }

    // Greedy allocation: for each exam, try to assign smallest number of largest rooms until the sum >= size.
    for (auto &ec : ex) {
        int need = ec.first;
        int course = ec.second;
        vector<int> assignedRooms;
        for (auto &rp : roomVec) {
            if (need <= 0) break;
            if (rp.first <= 0) continue;
            int take = min(rp.first, need);
            // we'll "use" the room's capacity by decreasing it; but we want to assign whole room index to this exam
            // to simplify, if room has any remaining capacity, assign the room (rooms can be shared across exams within slot only if capacity remains)
            assignedRooms.push_back(rp.second);
            need -= rp.first;
            rp.first = 0; // mark as consumed for simplicity; this is conservative but safe
        }
        if (need > 0) {
            // Shouldn't happen due to earlier total capacity check, but handle
            return {};
        }
        allocation[course] = assignedRooms;
    }
    return allocation;
}

unordered_map<int, unordered_map<int, vector<int>>> allocate_rooms_all_slots(
    const vector<int>& rooms,
    const Result& result,
    const vector<int>& course_sizes)
{
    int n = (int)result.color.size();
    unordered_map<int, vector<int>> slot_exams;
    for (int i = 0; i < n; ++i) {
        int c = result.color[i];
        slot_exams[c].push_back(i);
    }
    unordered_map<int, unordered_map<int, vector<int>>> alloc_all;
    // for each slot, attempt allocation
    for (auto &p : slot_exams) {
        int slot = p.first;
        const auto& exams = p.second;
        // prepare rooms vector and ids
        vector<int> rooms_copy = rooms;
        vector<int> room_ids(rooms.size());
        iota(room_ids.begin(), room_ids.end(), 0);
        auto allocation = allocate_rooms_for_slot(rooms_copy, room_ids, exams, course_sizes);
        if (allocation.empty()) {
            // allocation failed; return empty map to indicate failure
            return {};
        }
        alloc_all[slot] = std::move(allocation);
    }
    return alloc_all;
}

// Helper to print result & allocations
void print_result(const string& name, const Result& res) {
    cout << "Algorithm: " << name << "\n";
    cout << "  Time: " << fixed << setprecision(3) << res.timeMs << " ms\n";
    cout << "  Colors used (slots): " << res.numColors << "\n";
}

// small utility to generate order vector [0..n-1]
vector<int> identity_order(int n) {
    vector<int> o(n);
    iota(o.begin(), o.end(), 0);
    return o;
}

// For Welsh-Powell we already create order inside function -> but user might want both greedy (input order) and greedy random
vector<int> decreasing_degree_order(const vector<vector<int>>& adj) {
    int n = adj.size();
    vector<pair<int,int>> dv;
    for (int i = 0; i < n; ++i) dv.emplace_back((int)adj[i].size(), i);
    sort(dv.begin(), dv.end(), greater<>());
    vector<int> order; order.reserve(n);
    for (auto &p : dv) order.push_back(p.second);
    return order;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Read input
    int N, M, R;
    if (!(cin >> N >> M >> R)) {
        cerr << "Input error. Expected: N M R\n";
        return 1;
    }
    vector<vector<int>> student_courses;
    student_courses.reserve(M);
    for (int i = 0; i < M; ++i) {
        int k;
        cin >> k;
        vector<int> courses(k);
        for (int j = 0; j < k; ++j) {
            cin >> courses[j];
            if (courses[j] < 0 || courses[j] >= N) {
                cerr << "Course id out of range: " << courses[j] << "\n";
                return 1;
            }
        }
        // remove duplicates
        sort(courses.begin(), courses.end());
        courses.erase(unique(courses.begin(), courses.end()), courses.end());
        student_courses.push_back(std::move(courses));
    }
    vector<int> rooms;
    rooms.reserve(R);
    for (int i = 0; i < R; ++i) {
        int cap; cin >> cap;
        rooms.push_back(cap);
    }

    // Build graph
    auto adj = build_graph(N, student_courses);
    auto course_sizes = compute_course_sizes(N, student_courses);

    cout << "Graph: " << N << " vertices (courses), " << M << " students, " << R << " rooms\n";

    // Greedy: naive order
    auto tstart = chrono_clock::now();
    Result greedy_res = greedy_coloring(adj, identity_order(N));
    auto tend = chrono_clock::now(); // time included already in function; just for consistency

    // Welsh-Powell
    Result welsh_res = welsh_powell(adj);

    // DSATUR
    Result dsatur_res = dsatur_coloring(adj);

    cout << "\n=== Results Summary ===\n";
    print_result("Greedy (input order)", greedy_res);
    print_result("Welsh-Powell", welsh_res);
    print_result("DSATUR", dsatur_res);

    // Choose the best by smallest number of colors; tie-breaker: time
    vector<pair<Result,string>> results = {{greedy_res, "Greedy (input order)"}, {welsh_res, "Welsh-Powell"}, {dsatur_res, "DSATUR"}};
    sort(results.begin(), results.end(), [](const auto& a, const auto& b){
        if (a.first.numColors != b.first.numColors) return a.first.numColors < b.first.numColors;
        return a.first.timeMs < b.first.timeMs;
    });
    auto best = results.front();
    cout << "\nBest algorithm by slots: " << best.second << " -> " << best.first.numColors << " slots\n";

    // Room allocation (if rooms provided)
    if (R > 0) {
        cout << "\nAttempting room allocation using result from " << best.second << "...\n";
        auto alloc = allocate_rooms_all_slots(rooms, best.first, course_sizes);
        if (alloc.empty()) {
            cout << "Room allocation FAILED: total room capacity per slot insufficient for some slot.\n";
            // Optionally we can try to increase number of slots by recolouring with more colors (not implemented)
        } else {
            cout << "Room allocation SUCCESS.\n";
            // print per-slot summary
            for (int slot = 0; slot < best.first.numColors; ++slot) {
                cout << "Slot " << slot << ":\n";
                // list exams in slot
                vector<int> exams;
                for (int i = 0; i < N; ++i) if (best.first.color[i] == slot) exams.push_back(i);
                for (int c: exams) {
                    cout << "  Course " << c << " (students=" << course_sizes[c] << "): rooms [";
                    auto it = alloc[slot].find(c);
                    if (it != alloc[slot].end()) {
                        for (size_t j = 0; j < it->second.size(); ++j) {
                            cout << it->second[j];
                            if (j + 1 < it->second.size()) cout << ", ";
                        }
                    } else cout << "UNASSIGNED";
                    cout << "]\n";
                }
            }
        }
    } else {
        cout << "\nNo rooms provided (R=0). Room allocation skipped.\n";
    }

    // Print coloring mapping of best result (compact)
    cout << "\nFinal assignment (course -> slot):\n";
    for (int i = 0; i < N; ++i) {
        cout << i << ":" << best.first.color[i];
        if (i + 1 < N) cout << " ";
    }
    cout << "\n";

    return 0;
}
