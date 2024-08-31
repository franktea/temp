


#include <queue>
#include <tuple>
#include <array>
#include <set>
#include <iostream>
#include <cassert>
#include <map>
#include <vector>
#include <string>

using namespace std;

const array<char, 3> names = {'A', 'B', 'C'};
const array<int, 3> V = {8, 5, 3};

struct State {
    array<int, 3> w; // 记录每个桶的水
    vector<string> steps; // 记录倒水的步骤

    bool CanPour(int i, int j) const {
        // 从左边往右边倒，条件是左边有水（不为空）且右边未满。
        return w[i] > 0 && (w[j] < V[j]);
    }

    State Pour(int i, int j) const {
        assert(CanPour(i, j));
        State s2 = *this;
        string str;
        str.push_back(names[i]);
        str.append("->");
        str.push_back(names[j]);
        str.append(":");
        // 要么把第i个桶倒空
        if(s2.w[i] <= V[j] - s2.w[j]) {
            s2.w[j] += s2.w[i];
            s2.w[i] = 0;
        } else { // 要么把第i个桶倒空
            s2.w[i] -= (V[j] - s2.w[j]);
            s2.w[j] = V[j];
        }
        str += std::to_string(s2.w[0]) + std::string(", ") +
            std::to_string(s2.w[1]) + std::string(", ") +
            std::to_string(s2.w[2]);
        s2.steps.push_back(str);
        return s2;
    }
};

std::set<array<int, 3>> visited;

int main() {
    queue<State> q;
    q.push(State{{8, 0, 0}, {}});
    visited.insert(array<int, 3>{8, 0, 0});
    while(!q.empty()) {
        auto state = q.front();
        q.pop();
        for(int i = 0; i < 3; ++i) {
            for(int j = 0; j < 3; ++j) {
                if(i == j) {
                    continue;
                }

                if(state.CanPour(i, j)) {
                    State s2 = state.Pour(i, j);
                    if(s2.w[0] == 4 || s2.w[1] == 4) {
                        std::cout<<"found: \n";
                        for(const auto& str: s2.steps) {
                            std::cout<<str<<"\n";
                        }
                        return 0;
                    }
                    if(! visited.contains(s2.w)) {
                        q.push(s2);
                        visited.insert(s2.w);
                    }
                }
            }
        }
    }
}