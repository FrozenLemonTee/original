#include "array.h"
#include "maths.h"

int main(){
    auto arr1 = original::array<int>(4);
    printf("%s", arr1.toString(true).c_str());
    original::array arr2 = {1, 4, 5, 3, 8};
    printf("%s", arr2.toString(true).c_str());
    printf("max(5,3):%d\n", original::max(5, 3));
    auto arr3 = arr2;
    printf("arr3:%p, arr2:%p\n", &arr3, &arr2);
    printf("arr3:%s\narr2:%s\n", arr3.toString().c_str(), arr2.toString().c_str());
    auto arr4 = original::array(arr2);
    printf("arr4:%p, arr2:%p\n", &arr4, &arr2);
    printf("arr4:%s\narr2:%s\n", arr4.toString().c_str(), arr2.toString().c_str());
    printf("arr2[2]:%d\n", arr2[2]);
    printf("arr3[-1]:%d\n", arr3[-1]);
    auto arr5 = original::array<int>(0);
    return 0;
}
