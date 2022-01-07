# Simplex
A visualization for simplices in low dimensions, and can compute dimensions of homology groups of simplicial complices over Z2.

Written for Orientation on Mathematical research. 

## How to operate?

 - You can generate points with the script `src/datagen/generate.py`, or place your own csv file with points somewhere.
 - Run the program from the command line with a few parameters:
    - for the frontend mode, run it with `Simplex.exe <file with points> frontend` where `<file with points>` is the path to the csv file with input points.
    - for the barcode mode, run it with `Simplex.exe <file with points> barcode <start> <end> <step> <output file>` where:
        - `<file with points>` is the path to the csv file with input points
        - `<start>` is a floating point value for the lowest epsilon in the barcode
        - `<end>` is a floating point value for the highest epsilon in the barcode
        - `<step>` is the step of epsilons. So for `<start> <end> <step>` as `0.1 0.2 0.01` it will compute the basis for homology for epsilons `0.1`, `0.11`, `0.12`, up to `0.2`.
        - `<output file>` is a (csv) file where the program will output the homology dimension, an index for the hole and an epsilon at which this hole existed.
 - Plot the barcode with the script `src/plot/plot.py`

Some results and a built binary with maximum barcode homology dimension 1 and maximum input points 512 will be posted in the releases tab. To change these values, please change the corresponding parameters in `include/default.h` and rebuild.
