#include "frontend/frontend.h"
#include "compute/reader.h"
#include "compute/column.h"

#include <memory>


int main(int argc, char** argv) {
    auto reader = std::make_unique<Reader>(argv[1]);
    auto points = reader->Read();
    auto compute = std::make_unique<Compute<MAX_POINTS>>(points);

    auto frontend = std::make_unique<Frontend>(std::move(*compute));

    frontend->Run();

    // manual benchmark
//    constexpr size_t dim = 3;
//    auto fut = std::async(std::launch::async, &Compute<MAX_POINTS>::FindSimplexDrawIndices<dim>, compute.get(), 1.6);
//    auto start = std::chrono::steady_clock::now();
//    while (fut.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
//        std::printf("%d\n", compute->current_simplices);
//        std::this_thread::sleep_for(std::chrono::milliseconds(60));
//    }
//    auto result = fut.get();
//    auto duration = std::chrono::steady_clock::now() - start;
//    std::printf("\n\n");
//    std::printf("%lldms\n", std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
//    std::printf("%llu simplices\n", result.size() / (1 + dim));

    // for AMD uProf
//    compute->FindSimplexDrawIndices<dim>(0.6);

    return 0;
}
