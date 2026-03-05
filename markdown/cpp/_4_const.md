# parameter
- const int * a = 0;
 > int const * a =  0;
- int  * const a = 0;

# function
```c++
//only useful in class or struct
class Entity
{
private:
int x, y;
public:
    int GetX () const{
    // x = 1;
    return x;
    }
}
```
# mutable