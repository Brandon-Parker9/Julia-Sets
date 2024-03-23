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

#define REAL_NUMBER -0.8
#define IMAGINARY_NUMBER -0.089

// so far 1, 3, 16 are actually kind of nice lolol 
// 14 are a bit odd 
#define COLOR_CHOICE 1

typedef struct {
    double real;
    double imag;
} Complex;

void calculate_julia_array_range(int width, int start_row, int end_row, int *result, double real, double imaginary);
void map_to_color(int iteration, int *red, int *green, int *blue, int color_choice);
double hue_to_rgb(double hue, double saturation, double lightness);


void calculate_julia_array_range(int width, int start_row, int end_row, int *result, double real, double imaginary) {
    
    // Define constant for Julia set
    Complex constant = {.real = real, .imag = imaginary}; // Example constant

    // Iterate through rows within the specified range
    for (int y = start_row; y < end_row; y++) {
        for (int x = 0; x < width; x++) {

            // Map pixel coordinates (x, y) directly to the rectangular region in the complex plane
            // The complex plane is mapped to a rectangular region defined by:
            // - Real part (x-axis): Range from -1.75 (leftmost) to 1.75 (rightmost)
            // - Imaginary part (y-axis): Range from -1.75 (bottom) to 1.75 (top)
            // The width and height of the rectangular region are adjusted to match the aspect ratio of the image.
            Complex z = {.real = x / (double)width * 3.5 - 1.75, .imag = y / (double)HEIGHT * 3.5 - 1.75};
            int iteration = 0;
            while (z.real * z.real + z.imag * z.imag <= 4.0 && iteration < MAX_ITERATION) {
                double temp = z.real * z.real - z.imag * z.imag + constant.real;
                z.imag = 2.0 * z.real * z.imag + constant.imag;
                z.real = temp;
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
    double t;
    double hue;
    
    if (iteration == 0 || iteration == MAX_ITERATION) {
        // Inside remains black
        *red = *green = *blue = 0;
        return;
    } else {
        // Normalize iteration count to range [0, 1]
        t = (double)iteration / MAX_ITERATION;
    }

    switch (color_choice) {
        case 1:
            // Existing smooth gradient scheme (blue to white)
            *red = (int)(9 * (1 - t) * t * t * t * 255);
            *green = (int)(15 * (1 - t) * (1 - t) * t * t * 255);
            *blue = (int)(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
            break;

        case 2:
            // New color scheme with better distribution for large canvases:
            // Wider range of colors, starting with blue, cycling through green, yellow, red, and back to blue
            hue = 0.66 * t + 0.16;  // Adjust hue range for desired colors
            *red = (int)(96 * (1 - fabs(4 * hue - 2)) * 255);
            *green = (int)(144 * (fabs(4 * hue - 3) - fabs(4 * hue - 1)) * 255);
            *blue = (int)(85 * (1 - fabs(2 * hue - 1)) * 255);
            break;

        case 3:
            // Fire-like color scheme:
            // Starts with dark red, transitions to orange and yellow, then fades to white
            *red = (int)(255 * sqrt(t));
            *green = (int)(185 * sqrt(t));
            *blue = (int)(85 * sqrt(t));
            break;

        case 4:
            // Autumn foliage color scheme
            *red = (int)(255 * (0.5 + 0.5 * cos(2 * M_PI * t)));
            *green = (int)(255 * (0.2 + 0.3 * cos(2 * M_PI * t + 2 * M_PI / 3)));
            *blue = (int)(255 * (0.1 + 0.1 * cos(2 * M_PI * t + 4 * M_PI / 3)));
            break;

        case 5:
            // Ocean-like color scheme:
            // Starts with deep blue, transitions to lighter blues and greens
            *red = (int)(50 + 205 * t);
            *green = (int)(100 + 155 * t);
            *blue = (int)(150 + 105 * t);
            break;

        case 6:
            // Rainbow color scheme:
            // Cycles through the rainbow spectrum
            *red = (int)(255 * (1 - t));
            *green = (int)(255 * fabs(0.5 - t));
            *blue = (int)(255 * t);
            break;

        case 7:
            // Desert color scheme:
            // Starts with sandy brown, transitions to reddish-brown
            *red = (int)(220 * (1 - t));
            *green = (int)(180 * (1 - t));
            *blue = (int)(130 * (1 - t));
            break;

        case 8:
            // Pastel color scheme:
            // Delicate, soft colors inspired by pastel art
            *red = (int)(220 * (0.5 + 0.5 * sin(2 * M_PI * t)));
            *green = (int)(205 * (0.5 + 0.5 * sin(2 * M_PI * t + 2 * M_PI / 3)));
            *blue = (int)(255 * (0.5 + 0.5 * sin(2 * M_PI * t + 4 * M_PI / 3)));
            break;

        case 9:
            // Night sky color scheme:
            // Deep blue hues with hints of purple, reminiscent of a starry night sky
            *red = (int)(20 + 100 * sin(2 * M_PI * t));
            *green = (int)(10 + 50 * sin(2 * M_PI * t + M_PI / 2));
            *blue = (int)(50 + 100 * sin(2 * M_PI * t + M_PI));
            break;

        case 10:
            // Smooth transition through the entire spectrum, with black for the "inside"
            // Inside remains black
             // Transition through the entire spectrum outside
            hue = 0.5 + t * 0.5; // Smoothly increase hue from 0.5 (green) to 1.0 (red)
            *red = (int)(255 * hue_to_rgb(hue, 0.8, 0.5));
            *green = (int)(255 * hue_to_rgb(hue - 1.0/3, 0.8, 0.5));
            *blue = (int)(255 * hue_to_rgb(hue - 2.0/3, 0.8, 0.5));
            break;
        
        case 11:
            // Twilight Sky Color Scheme
            *red = (int)(0 * (1 - t) + 30 * t);
            *green = (int)(0 * (1 - t) + 0 * t);
            *blue = (int)(128 * (1 - t) + 128 * t);
            break;

        case 12: 
            // Summer Sunset Color Scheme:
            *red = (int)(255 * (1 - t));
            *green = (int)(69 * (1 - t) + 128 * t);
            *blue = (int)(0 * (1 - t) + 128 * t);
            break;

        default:
            // Default to black for unknown color choice
            *red = *green = *blue = 0;
            break;
    }
}

double hue_to_rgb(double hue, double saturation, double lightness) {

    // Calculate chroma (color intensity) based on lightness and saturation
    double chroma = (1 - fabs(2 * lightness - 1)) * saturation;
    
    // Modify hue to fit within the range [0, 6) to simplify color mapping
    double hue_mod = hue * 6;
    
    // Calculate intermediate value 'x' based on chroma and hue
    double x = chroma * (1 - fabs(fmod(hue_mod, 2) - 1));
    
    // Initialize variables for red, green, and blue components
    double r, g, b;
    
    // Determine RGB values based on the hue value
    if (hue_mod < 1) {
        r = chroma;
        g = x;
        b = 0;
    } else if (hue_mod < 2) {
        r = x;
        g = chroma;
        b = 0;
    } else if (hue_mod < 3) {
        r = 0;
        g = chroma;
        b = x;
    } else if (hue_mod < 4) {
        r = 0;
        g = x;
        b = chroma;
    } else if (hue_mod < 5) {
        r = x;
        g = 0;
        b = chroma;
    } else {
        r = chroma;
        g = 0;
        b = x;
    }

    // Calculate the 'm' parameter to shift color intensity
    double m = lightness - 0.5 * chroma;
    
    // Return the final RGB color value
    return r + m;
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
    int *local_julia_set;
    local_julia_set = malloc(sizeof(int) * local_total_elements);
    if (local_julia_set == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        MPI_Finalize();
        return 1;
    }

    // Generate the Mandelbrot set
    calculate_julia_array_range(WIDTH, start_row, end_row, local_julia_set, REAL_NUMBER, IMAGINARY_NUMBER);

    // Send and Receive local results (instead of Gather)
    if (rank != 0) {

        // Send local_julia_set size (consider uneven distribution)
        MPI_Send(&local_total_elements, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(local_julia_set, local_total_elements, MPI_INT, 0, 1, MPI_COMM_WORLD);

    } else { // Root process receives from all processes

        char filename[100]; // Buffer to hold the filename

        // Format the filename with height and width
        snprintf(filename, sizeof(filename), "julia-set_%dx%d_color-%d_iterations-%d_real-%f_imaginary-%f.png", WIDTH, HEIGHT, COLOR_CHOICE, MAX_ITERATION, REAL_NUMBER, IMAGINARY_NUMBER);

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
                
                array = local_julia_set;
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
