# Julia Set Generation

This project includes implementations for generating the Mandelbrot set and Julia sets, two famous fractals in mathematics. The provided files demonstrate how to calculate these sets and visualize them using the PNG image format.

## `parallel_combined_mandelbrot.c`

### Overview

- This file contains the main implementation of the parallel Mandelbrot set generation using MPI.
- It includes functions for calculating Mandelbrot set points, mapping iterations to colors, and writing the results to a PNG image file.
- The program distributes the computation of Mandelbrot set points among multiple MPI processes to leverage parallelism.

### Compilation and Execution

- To compile the code, you need an MPI implementation such as Open MPI
- Compile the code using a suitable MPI compiler wrapper. For example:
  ```bash
  mpicc parallel_combined_mandelbrot.c -o parallel_combined_mandelbrot -lm -lpng
  ```

### Parameters

- `WIDTH` and `HEIGHT`: Define the dimensions of the image (in pixels) representing the Mandelbrot set.
- `MAX_ITERATION`: Maximum number of iterations used to determine if a point is in the Mandelbrot set.
- `COLOR_CHOICE`: Choose a color scheme for rendering the Mandelbrot set.

### Output

- The program generates a PNG image file named `mandelbrot_<WIDTH>x<HEIGHT>_color-<COLOR_CHOICE>_iterations-<MAX_ITERATION>.png`, which contains the rendered Mandelbrot set using the specified parameters.

## `parallel_combined_julia_sets.c`

### Overview

- This file contains the main implementation of the parallel Julia set generation using MPI.
- It includes functions for calculating Julia set points, mapping iterations to colors, and writing the results to a PNG image file.
- The program distributes the computation of Julia set points among multiple MPI processes to leverage parallelism.

### Compilation and Execution

- To compile the code, you need an MPI implementation such as Open MPI or MPICH installed on your system.
- Compile the code using a suitable MPI compiler wrapper. For example:
  ```bash
  parallel_combined_julia_sets.c -o parallel_combined_julia_sets -lm -lpng
  ```

### Parameters

- `WIDTH` and `HEIGHT`: Define the dimensions of the image (in pixels) representing the Julia set.
- `MAX_ITERATION`: Maximum number of iterations used to determine if a point is in the Julia set.
- `REAL_NUMBER` and `IMAGINARY_NUMBER`: Parameters defining the constant complex number used in the Julia set calculation.
- `COLOR_CHOICE`: Choose a color scheme for rendering the Julia set.

### Output

- The program generates a PNG image file named `julia-set_<WIDTH>x<HEIGHT>_color-<COLOR_CHOICE>_iterations-<MAX_ITERATION>_real-<REAL_NUMBER>_imaginary-<IMAGINARY_NUMBER>.png`, which contains the rendered Julia set using the specified parameters.

