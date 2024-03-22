
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

void calculate_mandelbrot_array(int width, int height, int *result);
void calculate_mandelbrot_array_range(int width, int start_row, int end_row, int *result);
int generate_png(int width, int height, int array[], int color_choice);
void map_to_color(int iteration, int *red, int *green, int *blue, int color_choice);
double hue_to_rgb(double hue, double saturation, double lightness);

void calculate_mandelbrot_array(int width, int height, int *result) {
    double xmin = -2.0, xmax = 2.0, ymin = -2.0, ymax = 2.0;
    double xstep = (xmax - xmin) / width;
    double ystep = (ymax - ymin) / height;

    unsigned long long current_pixel = 0; // Initialize current pixel count

    for (int y = 0; y < height; y++) {
        double y0 = ymin + y * ystep;
        for (int x = 0; x < width; x++) {
            double x0 = xmin + x * xstep;
            double xx = 0.0, yy = 0.0;
            int iteration = 0;

            while (xx * xx + yy * yy <= 4.0 && iteration < MAX_ITERATION) {
                double xtemp = xx * xx - yy * yy + x0;
                yy = 2 * xx * yy + y0;
                xx = xtemp;
                iteration++;
            }

            if (iteration == MAX_ITERATION) {
                result[y * width + x] = 0; // Black
            } else {
                // Set color based on iteration count
                result[y * width + x] = iteration;
            }

            // Increment current pixel count
            current_pixel++;

            if (current_pixel % (WIDTH / 10) == 0){
                printf("\rMandelbrot Pixel Progress: %.2f%% Pixel Count: %llu", (double)current_pixel / ((double)width * height) * 100, current_pixel);
            }
        }
    }

    // new line after progress percentage 
    printf("\n");
}

void calculate_mandelbrot_array_range(int width, int start_row, int end_row, int *result) {
    
    // Define the boundaries of the Mandelbrot set in the complex plane
    double xmin = -2.0, xmax = 2.0, ymin = -2.0, ymax = 2.0;
    
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


int generate_png(int width, int height, int array[], int color_choice) {

    char filename[100]; // Buffer to hold the filename

    // Format the filename with height and width
    snprintf(filename, sizeof(filename), "output_%dx%d_color-%d_iterations-%d.png", WIDTH, HEIGHT, COLOR_CHOICE, MAX_ITERATION);

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
    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_BASE);

    // Initialize I/O for writing to file
    png_init_io(png_ptr, fp);

    // Write PNG header (including all required information)
    png_write_info(png_ptr, info_ptr);

    // Allocate memory for entire image data
    png_bytep image_data = (png_bytep)malloc(width * 4 * sizeof(png_byte)); // 4 bytes per pixel for RGBA

    if (!image_data) {
        fprintf(stderr, "Error allocating memory for image data\n");
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return 1;
    }

    // Initialize current pixel count
    unsigned long long current_pixel = 0; 

    // Fill image data with solid blue color
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {

            // Get pixel colour
            int red, green, blue;
            map_to_color(array[y * width + x], &red, &green, &blue, color_choice);

            int offset = x * 4; // 4 bytes per pixel

            image_data[offset] = red;         // Red
            image_data[offset + 1] = green;   // Green
            image_data[offset + 2] = blue;    // Blue
            image_data[offset + 3] = 255;     // Alpha (fully opaque)

            // Increment current pixel count
            current_pixel++;

            if (current_pixel % (WIDTH / 10) == 0){
                printf("\rPNG Pixel Progress: %.2f%% Pixel Count: %llu", (double)current_pixel / ((double)width * height) * 100, current_pixel);
            }
        }
        png_write_row(png_ptr, &image_data[0]);

    }

    // new line after progress percentage 
    printf("\n");

    // Write the end of the PNG information
    png_write_end(png_ptr, info_ptr);

    // Clean up
    free(image_data);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);

    printf("PNG image created successfully: %s \n", filename);
    return 0;
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

    int total_elements = WIDTH * (end_row - start_row);

     // Allocate memory for local Mandelbrot sets on each process
    int *local_mandelbrot_set;
    local_mandelbrot_set = malloc(sizeof(int) * total_elements);
    if (local_mandelbrot_set == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        MPI_Finalize();
        return 1;
    }

    // Generate the Mandelbrot set
    calculate_mandelbrot_array_range(WIDTH, start_row, end_row, local_mandelbrot_set);

    // Root process pre-allocates final Mandelbrot set
    int *final_mandelbrot_set;
    if (rank == 0) {
        final_mandelbrot_set = malloc(sizeof(int) * WIDTH * HEIGHT);
        if (final_mandelbrot_set == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        MPI_Finalize();
        return 1;
        }
    }

    // Send and Receive local results (instead of Gather)
    if (rank != 0) {

        // Send local_mandelbrot_set size (consider uneven distribution)
        MPI_Send(&total_elements, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(local_mandelbrot_set, total_elements, MPI_INT, 0, 1, MPI_COMM_WORLD);

    } else { // Root process receives from all processes
        
        int offset = 0;

        // Copy received data to final_mandelbrot_set at appropriate offset
        memcpy(final_mandelbrot_set + offset, local_mandelbrot_set, sizeof(int) * total_elements);
        offset += total_elements;

        for (int i = 1; i < size; i++) {

            // Allocate memory for received data
            int received_size;
            MPI_Recv(&received_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            int *temp_buffer = malloc(sizeof(int) * received_size);
            MPI_Recv(temp_buffer, received_size, MPI_INT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // Copy received data to final_mandelbrot_set at appropriate offset
            memcpy(final_mandelbrot_set + offset, temp_buffer, sizeof(int) * received_size);
            offset += received_size;

            free(temp_buffer); // Free temporary buffer if used
        }

        generate_png(WIDTH, HEIGHT, final_mandelbrot_set, COLOR_CHOICE);
        free(final_mandelbrot_set);

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
    }

    return 0;

}
