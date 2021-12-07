#include "frontend/frontend.h"
#include "compute/reader.h"

#include <memory>
#include <fstream>


enum class Mode {
    Frontend,
    Barcode,
    Benchmark,
};


int main(int argc, char** argv) {
    auto reader = std::make_unique<Reader>(argv[1]);
    auto points = reader->Read();
    auto compute = std::make_unique<Compute<MAX_POINTS>>(points);
    constexpr Mode mode = Mode::Barcode;

    if constexpr (mode == Mode::Frontend) {
        auto frontend = std::make_unique<Frontend>(std::move(compute));

        frontend->Run();
    }
    else if constexpr (mode == Mode::Barcode) {
        auto barcode = compute->FindBarcode(0.1, 0.8, 0.001);
        std::ofstream csv("./src/plot/barcode.csv");
        csv << "homology dimension,line index,epsilon" << std::endl;

        for (int dim = 0; dim < barcode.size(); dim++) {
            for (int i = 0; i < barcode[dim].size(); i++) {
                for (float eps : barcode[dim][i]) {
                    csv << dim << "," << i << "," << eps << std::endl;
                }
            }
        }
    }
    else if constexpr (mode == Mode::Benchmark) {
        // manual benchmark
//        constexpr size_t dim = 3;
//        auto fut = std::async(std::launch::async, &ComputeBase::FindHBasisDrawIndices, compute.get(), 1.6, 2);
//        auto start = std::chrono::steady_clock::now();
//        while (fut.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
//            std::printf("%d\n", compute->current_simplices);
//            std::this_thread::sleep_for(std::chrono::milliseconds(60));
//        }
//        auto result = fut.get();
//        auto duration = std::chrono::steady_clock::now() - start;
//        std::printf("\n\n");
//        std::printf("%lldms\n", std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
//        std::printf("%llu basis vectors\n", result.first);

        // for AMD uProf
        auto [dim, _] = compute->FindHBasisDrawIndices(1.6, 2);
        std::printf("%lld\n", dim);
    }
    return 0;
}
