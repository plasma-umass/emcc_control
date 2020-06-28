// #include <emscripten/emscripten.h>
// #include "../include/continuations.h"
// #include <stdio.h>
// #include <stdint.h>
// #include <stdlib.h>
// #include <stdarg.h>
// #include <vector>
#include <iostream>
#include <vector>

void foo() {
    throw 1234;
}

// DEFINE_HANDLER(the_main, k, u, {
//     std::cout << "Before foo" << std::endl;

//     try {
//         foo();
//     } catch(int e) {
//         std::cout << "Caught: " << e << std::endl;
//     }

//     std::cout << "After foo" << std::endl;

//     restore(k, 5);
// })


int main() {
    // initialize_continuations();

    std::cout << "Before control" << std::endl;

    std::vector<int> myvector (10);

    int x = 9;

    try {
        // foo();
        // throw "cat";
        x = myvector.at(23);
    } catch(...) {
        std::cout << "Caught: " << std::endl;
    }

    std::cout << "x = " << x << std::endl;

    return 0;
}