#include <iostream>
using namespace std;

int main()
{
    while (true)
    {
        int x,y,z;
        x = 1;
        y = 3;
        z = 2;
        x = y + z;
        z = x + y + z;
        cout << x << y << z << endl;
        
    }
    
    return 0;
}