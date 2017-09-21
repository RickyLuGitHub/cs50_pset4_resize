/**
 * Copies a BMP piece by piece, just because.
 */
       
#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        fprintf(stderr, "Usage: ./copy infile outfile\n");
        return 1;
    }

    // remember filenames
    int multiplier = atoi(argv[1]);
    char *infile = argv[2];
    char *outfile = argv[3];

    // open input file 
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 2;
    }

    // open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 3;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // malloc to create space
    // copy infile headers into malloc'd space
    BITMAPFILEHEADER * bf2 = (BITMAPFILEHEADER *) malloc(sizeof(BITMAPFILEHEADER));
    BITMAPINFOHEADER * bi2 = (BITMAPINFOHEADER *) malloc(sizeof(BITMAPINFOHEADER));
    *bf2 = bf;
    *bi2 = bi;

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 || 
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }

    // adjust the file headers width, height, padding biSizeImage and bfSize
    bi2->biWidth     = bi.biWidth * multiplier;
    bi2->biHeight    = bi.biHeight * multiplier;
    int new_padding      = (4 - (bi2->biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    bi2->biSizeImage = ((sizeof(RGBTRIPLE) * bi2->biWidth) + new_padding) * abs(bi2->biHeight);
    bf2->bfSize       = bi2->biSizeImage + sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER);

    // write outfile's BITMAPFILEHEADER
    fwrite(bf2, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(bi2, sizeof(BITMAPINFOHEADER), 1, outptr);

    // determine padding for scanlines
    int padding =  (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    int h_count;
    int v_count;

    // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
    {
        for (v_count = 0; v_count < multiplier; v_count++)
        {
            // iterate over pixels in scanline
            for (int j = 0; j < bi.biWidth; j++)
            {
                // temporary storage
                RGBTRIPLE triple;

                // read RGB triple from infile
                fread(&triple, sizeof(RGBTRIPLE), 1, inptr);
                
                // print out pixel m-times
                for (h_count = 0; h_count < multiplier; h_count++)
                {
                    // write RGB triple to outfile
                    fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
                }
                // then add it back (to demonstrate how)
                for (int k = 0; k < new_padding; k++)
                {
                    fputc(0x00, outptr);
                }        
            }   //finish printing through scanline(row)
            //move fseek cursor back to beginning of line as long we are not at the last vertical loop
            if (v_count < (multiplier-1))
                fseek(inptr, -1 * ((int)sizeof(RGBTRIPLE)*bi.biWidth), SEEK_CUR);
        }
        //skip over padding
        fseek(inptr, padding, SEEK_CUR);
    }

    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    /*free the malloced stuff*/
    free(bf2);
    free(bi2);

    // success
    return 0;
}
