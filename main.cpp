#include <iostream>

int main(int argc, char* argv[])
{
    std::cout << "Hello World, " << argc << "!\n";
    for (ssize_t i = 0; i < argc; ++i)
        std::cout << argv[i] << "\n";
    return 0;
}
