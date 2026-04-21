#include <bits/stdc++.h>
#include "skip_list.hpp"
int main(){
    // Minimal sanity check. Do nothing heavy.
    SkipList<int> sl;
    sl.insert(1); sl.insert(2); sl.insert(2); sl.deleteItem(3); sl.deleteItem(2);
    if(!sl.search(1)) return 1;
    return 0;
}
