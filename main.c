#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // Needed for usleep function
#include <time.h> // Needed for time functions
#include <png.h>

#define WIDTH 100
#define HEIGHT 100
#define MAX_ITERATION 1000

void calculate_mandelbrot_array(int width, int height, int (*result)[width]);
int generate_png(int width, int height, int (*array)[width]);
void map_to_color(int iteration, int *red, int *green, int *blue);

void calculate_mandelbrot_array(int width, int height, int (*result)[width]) {

    double xmin = -2.0, xmax = 2.0, ymin = -2.0, ymax = 2.0;
    double xstep = (xmax - xmin) / width;
    double ystep = (ymax - ymin) / height;

    int current_pixel = 0; // Initialize current pixel count

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
                result[y][x] = 0; // Black
            } else {
                // Set color based on iteration count
                result[y][x] = iteration;
            }

            // Increment current pixel count
            current_pixel++;

            if (current_pixel % (WIDTH / 10) == 0){
                printf("\rMandelbrot Pixel Progress: %.2f%%", (float)current_pixel / (width * height) * 100);
            }
        }
    }

    // new line after progress percentage 
    printf("\n");
}

int generate_png(int width, int height, int (*array)[width]) {

    char filename[100]; // Buffer to hold the filename

    // Format the filename with height and width
    snprintf(filename, sizeof(filename), "output_%dx%d.png", WIDTH, HEIGHT);

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
    png_bytep image_data = (png_bytep)malloc(height * width * 4 * sizeof(png_byte)); // 4 bytes per pixel for RGBA
    if (!image_data) {
        fprintf(stderr, "Error allocating memory for image data\n");
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return 1;
    }

    int current_pixel = 0; // Initialize current pixel count

    // Fill image data with solid blue color
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {

            int red, green, blue;
            map_to_color(array[y][x], &red, &green, &blue);

            int offset = (y * width + x) * 4; // 4 bytes per pixel
            image_data[offset] = red;           // Red
            image_data[offset + 1] = green;       // Green
            image_data[offset + 2] = blue;     // Blue
            image_data[offset + 3] = 255;     // Alpha (fully opaque)

            // Increment current pixel count
            current_pixel++;

            if (current_pixel % (WIDTH / 10) == 0){
                printf("\rPNG Pixel Progress: %.2f%%", (float)current_pixel / (width * height) * 100);
            }
        }
    }

    // new line after progress percentage 
    printf("\n");

    // Write image data
    for (int y = 0; y < height; y++) {
        png_write_row(png_ptr, &image_data[y * width * 4]);
    }

    // Write the end of the PNG information
    png_write_end(png_ptr, info_ptr);

    // Clean up
    free(image_data);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);

    printf("PNG image created successfully: %s \n", filename);
    return 0;
}

// Function to map iteration count to color based on a gradient
void map_to_color(int iteration, int *red, int *green, int *blue) {
    // Calculate smooth gradient colors
    double t = (double)iteration / MAX_ITERATION; // Normalize iteration count to range [0, 1]

    // Smooth gradient colors (e.g., from blue to white)
    *red = (int)(9 * (1 - t) * t * t * t * 255);
    *green = (int)(15 * (1 - t) * (1 - t) * t * t * 255);
    *blue = (int)(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
}

int main() {

    // Allocate memory for the Mandelbrot set
    int (*mandelbrotSet)[WIDTH] = malloc(sizeof(int[HEIGHT][WIDTH]));
    if (mandelbrotSet == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return 1;
    }

    // Generate the Mandelbrot set
    calculate_mandelbrot_array(WIDTH, HEIGHT, mandelbrotSet);

    // Output the Mandelbrot set (for testing)
    // printf("Mandelbrot set:\n");
    // for (int y = 0; y < HEIGHT; y++) {
    //     for (int x = 0; x < WIDTH; x++) {
    //         printf("%3d ", mandelbrotSet[y][x]);
    //     }
    //     printf("\n");
    // }

    generate_png(WIDTH, HEIGHT, mandelbrotSet);

    // Free memory
    free(mandelbrotSet);

    return 0;

}
