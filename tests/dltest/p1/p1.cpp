#include <iostream>

void foo() {
    std::cerr << "Hi, I'm the first plugin" << std::endl;
}

extern "C" {
    void init_p1() {
	std::cerr << "Initializing the first plugin" << std::endl;
	foo();
    }
}
