#include "frontend/frontend.h"
#include "simplex/reader.h"


int main(int argc, char** argv) {
    Reader reader(argv[1]);
    Compute simplex(reader.Read());

    Frontend frontend(std::move(simplex));

    frontend.Run();

    return 0;
}
