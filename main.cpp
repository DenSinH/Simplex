#include "frontend/frontend.h"
#include "compute/reader.h"


int main(int argc, char** argv) {
    Reader reader(argv[1]);
    Compute<MAX_POINTS> compute(reader.Read());

    Frontend frontend(std::move(compute));

    frontend.Run();

//    compute.FindSimplexDrawIndices<2>(1.6);

    return 0;
}
