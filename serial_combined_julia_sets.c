#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <png.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define WIDTH 1000
#define HEIGHT 1000
#define MAX_ITERATION 1000

#define REAL_NUMBER -0.8
#define IMAGINARY_NUMBER -0.156

// so far 1, 3 are actually kind of nice lolol
#define COLOR_CHOICE 12

typedef struct {
    double real;
    double imag;
} Complex;

int generate_png(int width, int height, int color_choice, double real, double imaginary);
void map_to_color(int iteration, int *red, int *green, int *blue, int color_choice);
double hue_to_rgb(double hue, double saturation, double lightness);

int generate_png(int width, int height, int color_choice, double real, double imaginary) {

    // Buffer to hold the filename
    char filename[100]; 

    // Format the filename with height, width, color choice, and maximum iteration
    snprintf(filename, sizeof(filename), "output_%dx%d_color-%d_iterations-%d_real-%f_imaginary-%f.png", width, height, color_choice, MAX_ITERATION, real, imaginary);

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

    int result;

    // Initialize current pixel count
    unsigned long long current_pixel = 0;

    // Define constant for Julia set
    Complex constant = {.real = real, .imag = imaginary}; // Example constant

    // Generate the Julia set and PNG image
    // Iterate through each pixel and calculate the Julia set value
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            Complex z = {.real = x / (double)WIDTH * 3.5 - 1.75, .imag = y / (double)HEIGHT * 2.0 - 1.0};
            int iteration = 0;
            while (z.real * z.real + z.imag * z.imag <= 4.0 && iteration < MAX_ITERATION) {
                double temp = z.real * z.real - z.imag * z.imag + constant.real;
                z.imag = 2.0 * z.real * z.imag + constant.imag;
                z.real = temp;
                iteration++;
            }

            // Determine color based on iteration count
            if (iteration == MAX_ITERATION) {
                // Black
                result = 0;
            } else {
                // Set color based on iteration count
                result = iteration;
            }

            int red, green, blue;

            // Map iteration count to RGB color
            map_to_color(result, &red, &green, &blue, color_choice);

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
                printf("\rPNG Pixel Progress: %.2f%% Pixel Count: %llu", (double)current_pixel / ((double)width * height) * 100, current_pixel);
            }
        }

        // Write current row to PNG
        png_write_row(png_ptr, &image_data[0]);
    }

    // Print newline after progress percentage  
    printf("\n");

    // Write the end of the PNG information
    png_write_end(png_ptr, info_ptr);

    // Clean up
    free(image_data);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);

    // Print success message
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
    // Calculate chroma (color intensity)
    double chroma = (1 - fabs(2 * lightness - 1)) * saturation;

    // Convert hue to hue_mod, which is a value between 0 and 6
    double hue_mod = hue * 6;

    // Calculate intermediate value x
    double x = chroma * (1 - fabs(fmod(hue_mod, 2) - 1));

    double r, g, b;

    // Determine RGB components based on hue_mod
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

    // Calculate lightness modifier (m)
    double m = lightness - 0.5 * chroma;

    // Return final RGB value by adding lightness modifier to each RGB component
    return r + m;
}

int main() {

    clock_t start_time, end_time;
    double elapsed_time;

    // Start measuring time
    start_time = clock();

    generate_png(WIDTH, HEIGHT, COLOR_CHOICE, REAL_NUMBER, IMAGINARY_NUMBER);

    // Stop measuring time
    end_time = clock();

    // Calculate elapsed time in seconds
    elapsed_time = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;

    // Print runtime in seconds
    printf("Runtime: %.3f seconds\n", elapsed_time);

    return 0;
}