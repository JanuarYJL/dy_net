#include <iostream>
#include "capa.h"
using namespace std;
class A
{
public:
    A()
    {
        a = 0;
    }
    ~A() {}
    PMutex aMutex;
    int a GUARDED_BY(aMutex);
    void oprA()
    {
        a = 3;
    }
};

int main()
{
    A *usea = new A;
    usea->oprA();
    cout << "test now" << endl;
    return 0;
}
