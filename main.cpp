#include "frontend/frontend.h"
#include "compute/reader.h"


int main(int argc, char** argv) {
    Reader reader(argv[1]);
    Compute<MAX_POINTS> compute(reader.Read());

    Frontend frontend(std::move(compute));

    frontend.Run();

    // manual benchmark
//    auto fut = std::async(std::launch::async, &Compute<MAX_POINTS>::FindSimplexDrawIndices<3>, &compute, 1.6);
//    auto start = std::chrono::steady_clock::now();
//    while (fut.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
//        std::printf("%d\n", compute.current_simplices);
//        std::this_thread::sleep_for(std::chrono::milliseconds(60));
//    }
//    auto result = fut.get();
//    auto duration = std::chrono::steady_clock::now() - start;
//    std::printf("\n\n");
//    std::printf("%lldms\n", std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
//    std::printf("%llu simplices\n", result.size());

    // for AMD uProf
//    compute.FindSimplexDrawIndices<3>(0.6);

    return 0;
}
