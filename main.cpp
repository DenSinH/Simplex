#include "frontend/frontend.h"
#include "compute/reader.h"

#include <memory>
#include <fstream>
#include <string>
#include <algorithm>


/*
 * In order to change the maximum homology dimension or the maximum amount of points that can be passed,
 * edit the file include/default.h and rebuild!
 * */


enum class Mode {
    Frontend,
    Barcode,
    Benchmark,
    Testing,
};


int main(int argc, char** argv) {
    if (argc == 1) {
      std::printf("Please enter a file with points\n");
      exit(1);
    }
    auto reader = std::make_unique<Reader>(argv[1]);
    auto points = reader->Read();
    auto compute = std::make_unique<Compute<MAX_POINTS>>(points);
    Mode mode;
    if (argc == 2) {
        mode = Mode::Frontend;
    }
    else {
        std::string mode_string{argv[2]};
        std::transform(mode_string.begin(), mode_string.end(), mode_string.begin(), [](char c) { return std::tolower(c); });
        if (mode_string == "frontend") mode = Mode::Frontend;
        else if (mode_string == "barcode") mode = Mode::Barcode;
        else {
           std::printf("Please enter a valid mode (frontend or barcode), got %s\n", argv[2]);
           exit(1);
        }
    }

    if (mode == Mode::Frontend) {
        auto frontend = std::make_unique<Frontend>(std::move(compute));

        frontend->Run();
    }
    else if (mode == Mode::Barcode) {
        if (argc < 5) {
            std::printf("Please enter valid parameters for the barcode (end, output_file), got %d parameters\n", argc);
            exit(1);
        }
        double end;
        try {
            end = std::stod(argv[3]);
        }
        catch (std::out_of_range) {
            std::printf("Could not parse barcode end, please enter valid floating point values\n");
            exit(1);
        }
        std::string output_file = argv[4];

        auto barcode = compute->FindBarcode(end);
        std::ofstream csv(output_file);
        csv << "homology dimension,start,end" << std::endl;

        for (int dim = 0; dim < barcode.size(); dim++) {
            for (const auto [start, end] : barcode[dim]) {
                csv << dim << "," << start << "," << end << std::endl;
            }
        }
    }
    else if (mode == Mode::Benchmark) {
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
        auto [dim, _] = compute->FindHBasisDrawIndices(0.2, 2);
        std::printf("%lld\n", dim);
    }
    else {
        compute->FindHBasisDrawIndices(0.2, 1);
    }
    return 0;
}
