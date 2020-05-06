#include "compressor.h"
#include "descompressor.h"

int main(int argc, char const *argv[]) {

    FILE *file = NULL;

    BMPFILEHEADER *bmpFile = (BMPFILEHEADER *)malloc(14);
    BMPINFOHEADER *bmpInfo = (BMPINFOHEADER *)malloc(40);

    file = fopen("images/8x8.bmp", "rb"); // Openning image that we want to compress.

    if (file == NULL) { // Checking if there was an error opening the image.
        printf("error reading file");
        return ERROR;
    }

    // Reading the bmp file header and info header so we can read image data without troubles.
    if (!readBMPFileHeader(file, bmpFile) || !readBMPInfoHeader(file, bmpInfo))
        return ERROR;

    // Moving our file pointer to the bitmap data region.
    moveToBitmapData(file, bmpFile);

    // We're going to split the RGB channels into these 3 matrices below:
    unsigned char **R = NULL, **G = NULL, **B = NULL;

    // Allocating enough memory to store R, G and B channels.
    R = allocMatrix(R, getHeight(bmpInfo), getWidth(bmpInfo));
    G = allocMatrix(G, getHeight(bmpInfo), getWidth(bmpInfo));
    B = allocMatrix(B, getHeight(bmpInfo), getWidth(bmpInfo));

    // Separates the bitmap data into its RGB components.
    separateComponents(file, bmpInfo, R, G, B);

    // Now we're going to convert from RGB to YCbCr to increase DCT performance.
    float **Y = NULL, **Cb = NULL, **Cr = NULL;

    Y = allocFloatMatrix(Y, getHeight(bmpInfo), getWidth(bmpInfo));
    Cb = allocFloatMatrix(Cb, getHeight(bmpInfo), getWidth(bmpInfo));
    Cr = allocFloatMatrix(Cr, getHeight(bmpInfo), getWidth(bmpInfo));

    rgbToYcbcr(R, G, B, Y, Cb, Cr, getHeight(bmpInfo), getWidth(bmpInfo));

    // Dividing each component into 8x8 matrices in order to use DCT (Discrete Cosine Transform) algorithm,
    // apply quantization and vectorization steps at each 8x8 matrix, due to some researchs proving that this
    // division increases the efficiency of these steps.

    FILE *compressed = fopen("compressed.bin", "wb+"); // File to save compressed image
    long int auxY, auxCb;                      // aux variables to store where which component ends

    Y = divideMatrices(compressed, Y, getHeight(bmpInfo), getWidth(bmpInfo), bmpInfo, bmpFile);
    auxY = ftell(compressed);

    Cb = divideMatrices(compressed, Cb, getHeight(bmpInfo), getWidth(bmpInfo), bmpInfo, bmpFile);
    auxCb = ftell(compressed);

    Cr = divideMatrices(compressed, Cr, getHeight(bmpInfo), getWidth(bmpInfo), bmpInfo, bmpFile);

    fclose(compressed);

    /* INÍCIO DA PARTE DA DES   COMPRESSÃO */
    // descompressor(bmpInfo, compressed, auxY, auxCb, 0);

    // Free allocated memory.
    fclose(file);
    free(bmpFile);
    free(bmpInfo);
    freeMatrix(R, getHeight(bmpInfo));
    freeMatrix(G, getHeight(bmpInfo));
    freeMatrix(B, getHeight(bmpInfo));
    freeFloatMatrix(Y, getHeight(bmpInfo));
    freeFloatMatrix(Cb, getHeight(bmpInfo));
    freeFloatMatrix(Cr, getHeight(bmpInfo));

    return SUCCESS;
}