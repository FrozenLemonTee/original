#ifndef MATHS_H
#define MATHS_H

namespace original{

    #define E 2.7182818284590452353602874713527
    #define PI 3.1415926535897932384626433832795

    template<typename TYPE>
    TYPE abs(TYPE a);

    template<typename TYPE>
    TYPE max(TYPE a, TYPE b);

    template<typename TYPE>
    TYPE min(TYPE a, TYPE b);
}

    template<typename TYPE>
    TYPE original::abs(TYPE a) {
        return a > 0 ? a : -a;
    }

    template<typename TYPE>
    TYPE original::max(TYPE a, TYPE b) {
        return a > b ? a : b;
    }

    template<typename TYPE>
    TYPE original::min(TYPE a, TYPE b) {
        return a < b ? a : b;
    }

#endif //MATHS_H
