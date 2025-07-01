
#include "lodepng.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// fred data structure for box blur
typedef struct {
    unsigned char *src;       // Pointer to the original image data
    unsigned char *dst;       // Pointer to the blurred image data
    unsigned int width;       // Image width plotis
    unsigned int height;      // Image height aukstis
    unsigned int startRow;    // Starting row (inclusive)
    unsigned int endRow;      // Ending row (exclusive)
} BlurThreadData;

// funkcija to generate a 4x4 PNG with solid color quadrants
void generate4x4PNG(const char *filename) {
    unsigned int width = 4, height = 4;
    unsigned char image[4 * 4 * 4]; // 4x4 image with Red Ggreen Blue Alpha channels

// Fill each pixel with  quadrant colors
    for (unsigned int y = 0; y < height; y++) {
        for (unsigned int x = 0; x < width; x++) {
            unsigned int index = 4 * (y * width + x);
            if (x < 2 && y < 2) { // virsutine-kaire: Blue
                image[index + 0] = 0;   // Red
                image[index + 1] = 0;   // Green
                image[index + 2] = 255; // Blue
                image[index + 3] = 255; // Alpha
            } else if (x >= 2 && y < 2) { // virsutine desines Red
                image[index + 0] = 255; // Red
                image[index + 1] = 0;   // Green
                image[index + 2] = 0;   // Blue
                image[index + 3] = 255; // Alpha
            } else if (x < 2 && y >= 2) { // apazia-kaire: Yellow
                image[index + 0] = 255; // Red
                image[index + 1] = 255; // Green
                image[index + 2] = 0;   // Blue
                image[index + 3] = 255; // Alpha
            } else { // apacia-desine: Green
                image[index + 0] = 0;   // Red
                image[index + 1] = 255; // Green
                image[index + 2] = 0;   // Blue
                image[index + 3] = 255; // Alpha
            }
        }
    }
// isaugo the generated image to the file
    unsigned int error = lodepng_encode32_file(filename, image, width, height);
    if (error) {
        printf("Error %u: %s\n", error, lodepng_error_text(error));
        exit(1);
    }
    printf("Generated %s successfully.\n", filename);
}

// Threaded function to apply the 3x3 box blur for assigned rows
void *threadedBlur(void *arg) {
    BlurThreadData *data = (BlurThreadData *)arg; // Cast the argument to BlurThreadData
    unsigned char *image = data->src;
    unsigned char *blurredImage = data->dst;
    unsigned int width = data->width;
    unsigned int height = data->height;

    for (unsigned int row = data->startRow; row < data->endRow; row++) {
        for (unsigned int col = 0; col < width; col++) {
            unsigned int sumR = 0, sumG = 0, sumB = 0, sumA = 0;
            unsigned int count = 0; // Counter for the number of contributing pixels

            // 3x3 Kernel
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    int curRow = row + i;
                    int curCol = col + j;
                    
// Patikrinkite, ar gretimas pikselis yra ribose
                    if (curRow >= 0 && curRow < height && curCol >= 0 && curCol < width) {
                        unsigned int index = (curRow * width + curCol) * 4;
                        sumR += image[index + 0];
                        sumG += image[index + 1];
                        sumB += image[index + 2];
                        sumA += image[index + 3];
                        count++;
                    }
                }
            }
            unsigned int pixelIndex = (row * width + col) * 4;
            blurredImage[pixelIndex + 0] = sumR / count; // Red
            blurredImage[pixelIndex + 1] = sumG / count; // Green
            blurredImage[pixelIndex + 2] = sumB / count; // Blue
            blurredImage[pixelIndex + 3] = sumA / count; // Alpha
        }
    }
    pthread_exit(NULL);// Exit the fred
}

// Function to apply box blur in parallel using multiple fred
void applyBoxBlurParallel(unsigned char *image, unsigned char *blurredImage, 
                          unsigned int width, unsigned int height, int threadCount) {
    pthread_t threads[threadCount];//store thread identifiers
    BlurThreadData threadData[threadCount];//store thread-specific data


 // Calculate the number of rows each fred should process
    unsigned int rowsPerThread = height / threadCount;
    unsigned int remainingRows = height % threadCount;
    unsigned int currentStart = 0;

    for (int t = 0; t < threadCount; t++) {
        threadData[t].src = image;
        threadData[t].dst = blurredImage;
        threadData[t].width = width;
        threadData[t].height = height;
        threadData[t].startRow = currentStart;
        threadData[t].endRow = currentStart + rowsPerThread + (t < remainingRows ? 1 : 0);

        currentStart = threadData[t].endRow;
        pthread_create(&threads[t], NULL, threadedBlur, &threadData[t]);
    }
 // waiting for all freds to complete
    for (int t = 0; t < threadCount; t++) {
        pthread_join(threads[t], NULL);
    }
}

int main(int argc, char **argv) {
// Check is using correct 
    if (argc != 4) {
        printf("Usage: %s <input_png> <output_png> <num_threads>\n", argv[0]);
        return 1;
    }

    const char *inputFile = argv[1];
    const char *outputFile = argv[2];
    int threadCount = atoi(argv[3]);

    if (threadCount <= 0) {
        printf("Error: Thread count must be a positive integer.\n");
        return 1;
    }

    // Generate input image
    generate4x4PNG(inputFile);

    // Decode the generated PNG
    unsigned char *image, *blurredImage;
    unsigned int width, height;
    unsigned int error = lodepng_decode32_file(&image, &width, &height, inputFile);
    if (error) {
        printf("Error %u: %s\n", error, lodepng_error_text(error));
        return 1;
    }

    // Allocate memory for blurred image
    blurredImage = (unsigned char *)malloc(width * height * 4);
    if (!blurredImage) {
        printf("Failed to allocate memory for blurred image.\n");
        free(image);
        return 1;
    }

    // Apply blur
    applyBoxBlurParallel(image, blurredImage, width, height, threadCount);

    // Save the blurred image
    error = lodepng_encode32_file(outputFile, blurredImage, width, height);
    if (error) {
        printf("Error %u: %s\n", error, lodepng_error_text(error));
    } else {
        printf("Blurred image saved to %s\n", outputFile);//output file
    }

    free(image);
    free(blurredImage);
    return 0;
}



// cd /home/2334768/Desktop/task4Mut

//gcc -o task4mult task4mult.c lodepng.c -lpthread -lm

//./task4mult 4x4.png 4x4_blurred.png 4


// can change 4x4 in 5x5 
// on end 4 how mane thread is using







