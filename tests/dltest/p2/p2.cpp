#include <iostream>

void foo() {
    std::cerr << "Hi, I'm the second plugin" << std::endl;
}

extern "C" {
    void init_p2() {
	std::cerr << "Initializing the second plugin" << std::endl;
	foo();
    }
}
