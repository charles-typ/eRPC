#include <iostream>
#include "pc/datastructure/list.h"

using namespace pc::datastructure::list;

int main()
{
    List<long long int> l;
    for (int i = 0; i < 10; ++i)
    {
        l.push_back(i);
    }

    for (auto itr = l.begin(); itr != l.end(); ++itr)
        std::cout << *itr << ' ';
    std::cout << '\n';

    List<long long int> copy(l.begin(), l.end());
    for (auto elem : copy)
        std::cout << elem << ' ';
    std::cout << '\n';

    l.erase(l.find(5));
    for (auto elem : l)
        std::cout << elem << ' ';
    std::cout << '\n';
}