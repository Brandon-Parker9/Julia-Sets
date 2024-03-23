#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <png.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define WIDTH 10000
#define HEIGHT 10000
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

int generate_png(int width, int height, int color_choice, double real, double imaginary);
void map_to_color(int iteration, int *red, int *green, int *blue, int color_choice);
double hue_to_rgb(double hue, double saturation, double lightness);

int generate_png(int width, int height, int color_choice, double real, double imaginary) {

    // Buffer to hold the filename
    char filename[100]; 

    // Format the filename with height, width, color choice, and maximum iteration
    snprintf(filename, sizeof(filename), "julia-set_%dx%d_color-%d_iterations-%d_real-%f_imaginary-%f.png", width, height, color_choice, MAX_ITERATION, real, imaginary);

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

            // Map pixel coordinates (x, y) directly to the rectangular region in the complex plane
            // The complex plane is mapped to a rectangular region defined by:
            // - Real part (x-axis): Range from -1.75 (leftmost) to 1.75 (rightmost)
            // - Imaginary part (y-axis): Range from -1.75 (bottom) to 1.75 (top)
            // The width and height of the rectangular region are adjusted to match the aspect ratio of the image.
            Complex z = {.real = x / (double)WIDTH * 3.5 - 1.75, .imag = y / (double)HEIGHT * 3.5 - 1.75};
            int iteration = 0;
            while (z.real * z.real + z.imag * z.imag <= 4.0 && iteration < MAX_ITERATION) {
                double temp = z.real * z.real - z.imag * z.imag + constant.real;
                z.imag = 2.0 * z.real * z.imag + constant.imag;
                z.real = temp;
                iteration++;
            }

    // for (int y = 0; y < HEIGHT; y++) {
    //     for (int x = 0; x < WIDTH; x++) {
    //         // Map pixel coordinates to a circular region in the complex plane
    //         double radius = sqrt((x - WIDTH / 2.0) * (x - WIDTH / 2.0) + (y - HEIGHT / 2.0) * (y - HEIGHT / 2.0));
    //         double theta = atan2(y - HEIGHT / 2.0, x - WIDTH / 2.0);

    //         // Convert polar coordinates to Cartesian coordinates
    //         Complex z = {.real = radius / (double)WIDTH * 3.5, .imag = theta / (double)HEIGHT * 2.0};

    //         int iteration = 0;
    //         while (z.real * z.real + z.imag * z.imag <= 4.0 && iteration < MAX_ITERATION) {
    //             double temp = z.real * z.real - z.imag * z.imag + constant.real;
    //             z.imag = 2.0 * z.real * z.imag + constant.imag;
    //             z.real = temp;
    //             iteration++;
    //         }

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
        
        case 13:
            hue = 6.0 * t;
            int sector = (int)floor(hue); // Integer part determines color sector
            double offset = hue - sector;

            switch (sector % 6) {
                case 0:
                    *red = 255;
                    *green = (int)(255 * offset);
                    *blue = 0;
                    break;
                case 1:
                    *red = (int)(255 * (1 - offset));
                    *green = 255;
                    *blue = 0;
                    break;
                case 2:
                    *red = 0;
                    *green = 255;
                    *blue = (int)(255 * offset);
                    break;
                case 3:
                    *red = 0;
                    *green = (int)(255 * (1 - offset));
                    *blue = 255;
                    break;
                case 4:
                    *red = (int)(255 * offset);
                    *green = 0;
                    *blue = 255;
                    break;
                case 5:
                    *red = 255;
                    *green = 0;
                    *blue = (int)(255 * (1 - offset));
                    break;
            }
            break;

        case 14:
            hue = 6.0 * t;
            *red = (int)(255 * (1 - fabs(4 * hue - 2)) * pow(fabs(4 * hue - 2), 2)); // Emphasize red
            *green = (int)(255 * fabs(4 * hue - 3) * pow(fabs(4 * hue - 3), 1.5)); // Emphasize green less
            *blue = (int)(255 * fabs(4 * hue - 4)); // Blue not emphasized
            break;

        case 15:
            hue = fmod(t, 1.0); // Wrap hue value between 0 and 1
            float angle = M_PI * 2.0 * hue;
            float radius = 1.0;

            *red = (int)(255 * (radius * cos(angle) + 0.5));
            *green = (int)(255 * (radius * sin(angle) + 0.5));
            *blue = (int)(255 * (1.0 - radius));
            break;
        
        case 16:
            hue = 6.0 * t;
            int sector2 = (int)floor(hue); // Integer part determines color sector
            double offset2 = hue - sector2;

            switch (sector2 % 6) {
                case 0:
                    *red = 255;
                    *green = (int)(255 * offset2);
                    *blue = 0;
                    break;
                case 1:
                    *red = (int)(255 * (1 - offset2));
                    *green = 255;
                    *blue = 0;
                    break;
                case 2:
                    *red = 0;
                    *green = 255;
                    *blue = (int)(255 * offset2);
                    break;
                case 3:
                    *red = 0;
                    *green = (int)(255 * (1 - offset2));
                    *blue = 255;
                    break;
                case 4:
                    *red = (int)(255 * offset2);
                    *green = 0;
                    *blue = 255;
                    break;
                case 5:
                    *red = 255;
                    *green = 0;
                    *blue = (int)(255 * (1 - offset2));
                    break;
            }

            // Add secondary inverted rainbow with transparency
            switch ((sector2 + 3) % 6) {
                case 0:
                    *red += (int)(128 * (1 - offset2));
                    break;
                case 1:
                    *green += (int)(128 * (1 - offset2));
                    break;
                case 2:
                    *blue += (int)(128 * (1 - offset2));
                    break;
                case 3:
                    *red += (int)(128 * offset2);
                    break;
                case 4:
                    *green += (int)(128 * offset2);
                    break;
                case 5:
                    *blue += (int)(128 * offset2);
                    break;
            }
            break;
        
        case 17:

            double y = t * 2.0 - 1.0; // Normalize and scale y-axis for effect
            double h1 = fmod(atan2(y, 1.0) / (2.0 * M_PI) + 0.5, 1.0); // Hue for fire
            double h2 = fmod(atan2(-y, 1.0) / (2.0 * M_PI) + 0.5, 1.0); // Hue for ice

            // Fire colors with smooth transition
            *red = (int)(255 * (1.0 - h1) * pow(h1, 2));
            *green = (int)(255 * h1 * pow(h1, 1.5));
            *blue = 0;

            // Ice colors with smooth transition and transparency
            *red += (int)(128 * (1.0 - h2) * pow(h2, 3));
            *green += (int)(128 * h2 * pow(h2, 2));
            *blue += (int)(255 * h2);

            break;

        case 18:
            // Spring color scheme:
            // Starts with light green, transitions to vibrant greens and yellows
            *red = (int)(150 * (1 - t));
            *green = (int)(255 * t);
            *blue = (int)(100 * (1 - t) + 155 * t);
            break;

        case 19:
            // Sakura color scheme:
            // Shades of pink and white, resembling cherry blossom petals
            *red = (int)(255 * (0.9 + 0.1 * cos(2 * M_PI * t)));
            *green = (int)(200 * (0.5 + 0.5 * sin(2 * M_PI * t)));
            *blue = (int)(255 * (0.9 + 0.1 * cos(2 * M_PI * t)));
            break;

        case 20:
            // Autumn Leaves color scheme:
            // Starts with deep orange, transitions to red and brown hues
            *red = (int)(255 * (0.9 + 0.1 * cos(2 * M_PI * t)));
            *green = (int)(100 * (0.5 + 0.5 * sin(2 * M_PI * t)));
            *blue = (int)(0 * (0.9 + 0.1 * cos(2 * M_PI * t)));
            break;

        case 21:
            // Mystic Forest color scheme:
            // Mixture of dark greens and purples, evoking a mysterious atmosphere
            *red = (int)(30 + 50 * sin(2 * M_PI * t));
            *green = (int)(80 + 50 * sin(2 * M_PI * t + M_PI / 2));
            *blue = (int)(100 + 50 * sin(2 * M_PI * t + M_PI));
            break;

        case 22:
            // Golden Sunset color scheme:
            // Starts with warm yellow, transitions to orange and deep red
            *red = (int)(255 * (0.9 + 0.1 * cos(2 * M_PI * t)));
            *green = (int)(200 * (0.6 + 0.4 * sin(2 * M_PI * t)));
            *blue = (int)(50 * (0.5 + 0.5 * sin(2 * M_PI * t)));
            break;

        case 23:
            hue = 0.66 * t + 0.16; // Adjust hue range for desired colors
            *red = (int)(96 * (1 - fabs(4 * hue - 2)) * pow(fabs(4 * hue - 2), 0.5)); // Emphasize red with smooth falloff
            *green = (int)(144 * (fabs(4 * hue - 3) - fabs(4 * hue - 1)) * pow(fabs(4 * hue - 2.5), 0.75)); // Emphasize green with smoother falloff
            *blue = (int)(85 * (1 - fabs(2 * hue - 1)) * pow(1.0 - fabs(2 * hue - 1), 1.25)); // Blue fades smoothly to black

            // Adjust falloff power terms and multipliers for finer control

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