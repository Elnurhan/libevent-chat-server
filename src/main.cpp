#include "server.hpp"

int main()
{
    try {
        Server server("0.0.0.0", 12345);
        server.start();
    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
