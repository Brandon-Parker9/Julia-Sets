
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

int generate_png(int width, int height, int array[], int color_choice) {

    char filename[100]; // Buffer to hold the filename

    // Format the filename with height and width
    snprintf(filename, sizeof(filename), "output_%dx%d_color-%d.png", WIDTH, HEIGHT, COLOR_CHOICE);

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
    // png_bytep image_data = (png_bytep)malloc(height * width * 4 * sizeof(png_byte)); // 4 bytes per pixel for RGBA
    png_bytep image_data = (png_bytep)malloc(width * 4 * sizeof(png_byte)); // 4 bytes per pixel for RGBA

    if (!image_data) {
        fprintf(stderr, "Error allocating memory for image data\n");
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return 1;
    }

    unsigned long long current_pixel = 0; // Initialize current pixel count

    // Fill image data with solid blue color
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {

            int red, green, blue;
            map_to_color(array[x], &red, &green, &blue, color_choice);

            // int offset = (y * width + x) * 4; // 4 bytes per pixel
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

    // Write image data
    // for (int y = 0; y < height; y++) {
    //     png_write_row(png_ptr, &image_data[y * width * 4]);
    // }

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
    double chroma = (1 - fabs(2 * lightness - 1)) * saturation;
    double hue_mod = hue * 6;
    double x = chroma * (1 - fabs(fmod(hue_mod, 2) - 1));
    double r, g, b;
    
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

    double m = lightness - 0.5 * chroma;
    return r + m;
}

int main(int argc, char *argv[]) {

    // int rank, size;
    // double start_time, end_time, elapsed_time, tick;

    // MPI_Init(&argc, &argv);
    // MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // MPI_Comm_size(MPI_COMM_WORLD, &size);

    // // Returns the precision of the results returned by MPI_Wtime
    // tick = MPI_Wtick();

    // // Ensures all processes will enter the measured section of the code at the same time
    // MPI_Barrier(MPI_COMM_WORLD);

    // start_time = MPI_Wtime();

    // // if rank is 0
    // if (rank == 0) {
        



    // } else {
        




    // }

    // // Ensures all processes will enter the measured section of the code at the same time
    // MPI_Barrier(MPI_COMM_WORLD);

    // end_time = MPI_Wtime();

    // // Calculate the elapsed time
    // elapsed_time = end_time - start_time;

    // MPI_Finalize();

    // if rank is 0, print out the time analysis for merging arrays
    // if (rank == 0) {
    //     printf("\n********** Array Merge Time **********\n");
    //     printf("Total processes: %d\n", size);
    //     printf("Total computation time: %e seconds\n", elapsed_time);
    //     printf("Computation time per process: %e seconds\n", elapsed_time / size);
    //     printf("Resolution of MPI_Wtime: %e seconds\n", tick);
    // }


    // Allocate memory for the Mandelbrot set
    int *mandelbrotSet = malloc(sizeof(int) * WIDTH * HEIGHT);
    if (mandelbrotSet == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return 1;
    }

    // Generate the Mandelbrot set
    calculate_mandelbrot_array(WIDTH, HEIGHT, mandelbrotSet);

    generate_png(WIDTH, HEIGHT, mandelbrotSet, COLOR_CHOICE);

    // Free memory
    free(mandelbrotSet);

    return 0;

}
