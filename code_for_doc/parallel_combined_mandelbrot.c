
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // Needed for usleep function
#include <time.h> // Needed for time functions
#include <math.h>
#include <png.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define WIDTH 100
#define HEIGHT 100
#define MAX_ITERATION 1000

#define COLOR_CHOICE 1

void calculate_mandelbrot_array_range(int width, int start_row, int end_row, int *result);
void map_to_color(int iteration, int *red, int *green, int *blue, int color_choice);
double hue_to_rgb(double hue, double saturation, double lightness);


void calculate_mandelbrot_array_range(int width, int start_row, int end_row, int *result) {
    
    // Define the boundaries of the Mandelbrot set in the complex plane
    double xmin = -2.0, xmax = 1.0, ymin = -1.5, ymax = 1.5;
    
    // Calculate the step size in the x and y directions
    double xstep = (xmax - xmin) / width;
    double ystep = (ymax - ymin) / HEIGHT;

    // Iterate through rows within the specified range
    for (int y = start_row; y < end_row; y++) {
        // Calculate the imaginary part of the complex number corresponding to the current row
        double y0 = ymin + y * ystep;
        
        // Iterate through columns
        for (int x = 0; x < width; x++) {
            // Calculate the real part of the complex number corresponding to the current column
            double x0 = xmin + x * xstep;
            
            // Initialize variables for the real and imaginary parts of the complex number
            double xx = 0.0, yy = 0.0;
            
            // Initialize the iteration count
            int iteration = 0;

            // Iterate until the magnitude of the complex number exceeds 2 or maximum iterations are reached
            while (xx * xx + yy * yy <= 4.0 && iteration < MAX_ITERATION) {
                // Update the real part of the complex number
                double xtemp = xx * xx - yy * yy + x0;
                
                // Update the imaginary part of the complex number
                yy = 2 * xx * yy + y0;
                
                // Update the real part for the next iteration
                xx = xtemp;
                
                // Increment the iteration count
                iteration++;
            }

            // Store the result in the result array based on the iteration count
            if (iteration == MAX_ITERATION) {
                result[(y - start_row) * width + x] = 0;  // Inside Mandelbrot set
            } else {
                result[(y - start_row) * width + x] = iteration;  // Outside Mandelbrot set
            }
        }
    }
}

void map_to_color(int iteration, int *red, int *green, int *blue, int color_choice) {
    
    // .
    // .
    // .
    // .
    // Same as julia set
    // .
    // .
    // .
    // .

}

double hue_to_rgb(double hue, double saturation, double lightness) {
    
    // .
    // .
    // .
    // .
    // Same as julia set
    // .
    // .
    // .
    // .
    
}

int main(int argc, char *argv[]) {

    int rank, size;
    double start_time, end_time, elapsed_time, tick;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Returns the precision of the results returned by MPI_Wtime
    tick = MPI_Wtick();

    // Ensures all processes will enter the measured section of the code at the same time
    MPI_Barrier(MPI_COMM_WORLD);

    start_time = MPI_Wtime();

   // Determine rows to compute for each process
    int rows_per_process = HEIGHT / size;
    int remaining_rows = HEIGHT % size; // Rows left after distributing evenly

    int start_row, end_row;

    if (rank < remaining_rows) {
        // Distribute remaining rows evenly among the first 'remaining_rows' processes
        start_row = rank * (rows_per_process + 1);
        end_row = start_row + (rows_per_process + 1);
    } else {
        // Distribute remaining rows among the remaining processes
        start_row = rank * rows_per_process + remaining_rows;
        end_row = start_row + rows_per_process;
    }

    int local_total_elements = WIDTH * (end_row - start_row);

     // Allocate memory for local Mandelbrot sets on each process
    int *local_mandelbrot_set;
    local_mandelbrot_set = malloc(sizeof(int) * local_total_elements);
    if (local_mandelbrot_set == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        MPI_Finalize();
        return 1;
    }

    // Generate the Mandelbrot set
    calculate_mandelbrot_array_range(WIDTH, start_row, end_row, local_mandelbrot_set);

    // Send and Receive local results (instead of Gather)
    if (rank != 0) {

        // Send local_mandelbrot_set size (consider uneven distribution)
        MPI_Send(&local_total_elements, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(local_mandelbrot_set, local_total_elements, MPI_INT, 0, 1, MPI_COMM_WORLD);

    } else { // Root process receives from all processes

        char filename[100]; // Buffer to hold the filename

        // Format the filename with height and width
        snprintf(filename, sizeof(filename), "mandelbrot_%dx%d_color-%d_iterations-%d.png", WIDTH, HEIGHT, COLOR_CHOICE, MAX_ITERATION);

        // Open file for writing (binary mode)
        FILE *fp = fopen(filename, "wb");
        if (!fp) {
            fprintf(stderr, "Error opening file for writing\n");
            return 1;
        }

        // Create PNG structures
        png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!png_ptr) {
            fclose(fp);
            fprintf(stderr, "Error creating PNG write structure\n");
            return 1;
        }

        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr) {
            png_destroy_write_struct(&png_ptr, NULL);
            fclose(fp);
            fprintf(stderr, "Error creating PNG info structure\n");
            return 1;
        }

        // Error handling setup
        if (setjmp(png_jmpbuf(png_ptr))) {
            png_destroy_write_struct(&png_ptr, &info_ptr);
            fclose(fp);
            fprintf(stderr, "Error during PNG creation\n");
            return 1;
        }

        // Set image properties
        png_set_IHDR(png_ptr, info_ptr, WIDTH, HEIGHT, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                    PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_BASE);

        // Initialize I/O for writing to file
        png_init_io(png_ptr, fp);

        // Write PNG header (including all required information)
        png_write_info(png_ptr, info_ptr);

        // Initialize current pixel count
        unsigned long long current_pixel = 0; 
        int* array;
        int received_size;

        for (int i = 0; i < size; i++) {

            if (i == 0){
                
                array = local_mandelbrot_set;
                received_size = local_total_elements;

            } else {

                // Allocate memory for received data
                received_size;
                MPI_Recv(&received_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                array = malloc(sizeof(int) * received_size);
                MPI_Recv(array, received_size, MPI_INT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            }

            // Allocate memory for section of image data
            png_bytep image_data = (png_bytep)malloc(received_size * 4 * sizeof(png_byte)); // 4 bytes per pixel for RGBA

            if (!image_data) {
                fprintf(stderr, "Error allocating memory for image data\n");
                png_destroy_write_struct(&png_ptr, &info_ptr);
                fclose(fp);
                return 1;
            }

            // Fill image data with solid blue color
            for (int y = 0; y < (received_size/WIDTH); y++) {
                for (int x = 0; x < WIDTH; x++) {

                    // Get pixel colour
                    int red, green, blue;
                    map_to_color(array[y * WIDTH + x], &red, &green, &blue, COLOR_CHOICE);

                    // Calculate offset for pixel
                    int offset = x * 4; // 4 bytes per pixel

                    // Assign RGBA values to image data
                    image_data[offset] = red;         // Red
                    image_data[offset + 1] = green;   // Green
                    image_data[offset + 2] = blue;    // Blue
                    image_data[offset + 3] = 255;     // Alpha (fully opaque)

                    // Increment current pixel count
                    current_pixel++;

                    // Print progress percentage
                    if (current_pixel % (WIDTH / 10) == 0){
                        printf("\rPNG Pixel Progress: %.2f%% Pixel Count: %llu", (double)current_pixel / ((double)WIDTH * HEIGHT) * 100, current_pixel);
                    }
                }

                // Write current row to PNG
                png_write_row(png_ptr, &image_data[0]);
            }

            free(image_data);
            free(array);
        }

        // Print newline after progress percentage  
        printf("\n");

        // Write the end of the PNG information
        png_write_end(png_ptr, info_ptr);

        // Clean up
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);

        // Print success message
        printf("\nPNG image created successfully: %s \n", filename);

    }

    // Ensures all processes will enter the measured section of the code at the same time
    MPI_Barrier(MPI_COMM_WORLD);

    end_time = MPI_Wtime();

    // Calculate the elapsed time
    elapsed_time = end_time - start_time;

    MPI_Finalize();

    // if rank is 0, print out the time analysis for merging arrays
    if (rank == 0) {
        printf("\n********** PNG Creation Time **********\n");
        printf("Total processes: %d\n", size);
        printf("Total computation time: %e seconds\n", elapsed_time);
        printf("Computation time per process: %e seconds\n", elapsed_time / size);
        printf("Resolution of MPI_Wtime: %e seconds\n", tick);
        printf("%d,%d,%d,%e,%e,%e",WIDTH, HEIGHT, size, elapsed_time, (elapsed_time / size), tick);
    }

    return 0;

}
