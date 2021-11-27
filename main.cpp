#include "frontend/frontend.h"
#include "simplex/reader.h"


int main(int argc, char** argv) {
    Reader reader(argv[1]);
    Compute<MAX_POINTS> compute(reader.Read());

    Frontend frontend(std::move(compute));

    frontend.Run();

    compute.FindSimplexDrawIndices<2>(0.6);

    return 0;
}
