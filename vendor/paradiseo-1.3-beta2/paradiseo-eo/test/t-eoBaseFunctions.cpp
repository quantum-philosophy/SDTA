
#include <eoBaseFunctions.h>

#include <iostream>

using namespace std;

struct eo1 : public eoF<void>
{
    void operator()(void) {}
};

struct eo2 : public eoF<int>
{
    int operator()(void) { return 1; }
};

int main()
{
    eo1 _1; _1();
    eo2 _2; 
    int i = _2();

    std::cout << i << '\n';
    return i;
}
