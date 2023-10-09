#include <ranges>
#include <string>
#include <print>
#include <iostream>
#include <vector>

using std::string;
using namespace std;

int main() {
    string str("hello world test split");
    auto sv = str
        | ranges::views::split(' ') 
        | ranges::views::transform([](auto&& i){
            return i | ranges::to<string>(); }) 
        | ranges::to<vector>();
        
    for(auto&& s: sv) {
        cout<<s<<"\n";
    }

}
