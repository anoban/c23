// a simple application that prints the ASCII art of a Windows .BMP file.

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>

typedef struct RGBQUAD {
        unsigned char rgbBlue;
        unsigned char rgbGreen;
        unsigned char rgbRed;
        unsigned char rgbReserved;
} RGBQUAD;

// https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader
typedef struct BITMAPINFOHEADER {
        unsigned       biSize;
        int            biWidth;
        int            biHeight;
        unsigned short biPlanes;
        unsigned short biBitCount;
        unsigned       biCompression;
        unsigned       biSizeImage;
        int            biXPelsPerMeter;
        int            biYPelsPerMeter;
        unsigned       biClrUsed;
        unsigned       biClrImportant;
} BITMAPINFOHEADER;

// https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapfileheader

#pragma pack(push, 2) // this is critical here as this struct is supposed to be 14 bytes in size

typedef struct BITMAPFILEHEADER {
        unsigned short bfType;
        unsigned       bfSize;
        unsigned short bfReserved1;
        unsigned short bfReserved2;
        unsigned       bfOffBits;
} BITMAPFILEHEADER;

#pragma pack(pop)

enum COMPRESSION {
    BI_RGB       = 0x1,
    BI_RLE8      = 0x2,
    BI_RLE4      = 0x3,
    BI_BITFIELDS = 0x4,
    BI_JPEG      = 0x5,
    BI_PNG       = 0x6,
    BI_CMYK      = 0xB,
    BI_CMYKRLE8  = 0xC,
    BI_CMYKRLE4  = 0xD,
};

[[nodiscard]] static inline unsigned char* imopen(const char* const restrict fpath, long* const nreadbytes) {
    *nreadbytes             = 0;
    unsigned char* buffer   = nullptr;
    struct stat    filestat = {};
    long           nbytes   = 0;
    int            fdesc    = 0;

    if ((fdesc = open(fpath, O_RDONLY)) == -1) { // if open() failed, the return value will be -1
        fprintf(stderr, "Call to open() failed inside %s at line %d!; errno %d\n", __FUNCTION__, __LINE__, errno);
        return nullptr;
    }

    if (fstat(fdesc, &filestat)) { // if succeeds, 0 is returned, -1 if fails
        fprintf(stderr, "Call to fstat() failed inside %s at line %d!; errno %d\n", __FUNCTION__, __LINE__, errno);
        goto CLOSE_FDESC_AND_RETURN;
    }

    if (!(buffer = malloc(filestat.st_size))) { // caller is responsible for freeing this buffer
        fprintf(stderr, "Call to malloc() failed inside %s at line %d!\n", __FUNCTION__, __LINE__);
        goto CLOSE_FDESC_AND_RETURN;
    }

    if ((nbytes = read(fdesc, buffer, filestat.st_size)) != -1) {
        *nreadbytes = nbytes;
    } else {
        fprintf(stderr, "Call to read() failed inside %s at line %lu!; errno %d\n", __FUNCTION__, __LINE__, errno);
        free(buffer);
        buffer = nullptr;
    }

CLOSE_FDESC_AND_RETURN:
    // close() returns 0 on success and -1 on failure
    if (close(fdesc)) fprintf(stderr, "Call to close() failed inside %s at line %d!; errno %d\n", __FUNCTION__, __LINE__, errno);
    return buffer;
}

static inline BITMAPFILEHEADER parse_fileheader(const unsigned char* restrict imstream, const unsigned long fsize) {
    static_assert(sizeof(BITMAPFILEHEADER) == 14);
    assert(fsize >= sizeof(BITMAPFILEHEADER));

    BITMAPFILEHEADER header = { .bfType = 0, .bfSize = 0, .bfReserved1 = 0, .bfReserved2 = 0, .bfOffBits = 0 };

    header.bfType           = (((unsigned short) (*(imstream + 1))) << 8) | ((unsigned short) (*imstream));
    if (header.bfType != (((unsigned short) 'M' << 8) | (unsigned short) 'B')) {
        fputs("Error in parse_bitmapfile_header, file appears not to be a Windows BMP file\n", stderr);
        return header;
    }
    header.bfSize    = *((unsigned*) (imstream + 2));
    header.bfOffBits = *((unsigned*) (imstream + 10));
    return header;
}

static inline BITMAPINFOHEADER parse_infoheader(const unsigned char* const restrict imstream, const unsigned long fsize) {
    static_assert(sizeof(BITMAPINFOHEADER) == 40);
    assert(fsize >= (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)));

    BITMAPINFOHEADER header = {
        .biSize          = 0,
        .biWidth         = 0,
        .biHeight        = 0,
        .biPlanes        = 0,
        .biBitCount      = 0,
        .biCompression   = 0,
        .biSizeImage     = 0,
        .biXPelsPerMeter = 0,
        .biYPelsPerMeter = 0,
        .biClrUsed       = 0,
        .biClrImportant  = 0,
    };
    if (*((unsigned*) (imstream + 14U)) > 40) {
        fputws(L"BMP image seems to contain an unparsable file info header", stderr);
        return header;
    }
    header.biSize          = *((unsigned*) (imstream + 14U));
    header.biWidth         = *((unsigned*) (imstream + 18U));
    header.biHeight        = *((int*) (imstream + 22U));
    header.biPlanes        = *((unsigned short*) (imstream + 26U));
    header.biBitCount      = *((unsigned short*) (imstream + 28U));
    header.biCompression   = *((unsigned*) (imstream + 30U));
    header.biSizeImage     = *((unsigned*) (imstream + 34U));
    header.biXPelsPerMeter = *((unsigned*) (imstream + 38U));
    header.biYPelsPerMeter = *((unsigned*) (imstream + 42U));
    header.biClrUsed       = *((unsigned*) (imstream + 46U));
    header.biClrImportant  = *((unsigned*) (imstream + 50U));

    return header;
}

// A struct representing a BMP image.
typedef struct bmp {
        unsigned long    fsize;
        unsigned long    npixels;
        BITMAPFILEHEADER fhead;
        BITMAPINFOHEADER infhead;
        RGBQUAD*         pixel_buffer;
} bmp_t;

static inline bmp_t new_image(const unsigned char* restrict imstream /* will be freed by this procedure */, const unsigned long size) {
    assert(imstream);
    const BITMAPFILEHEADER fhead   = parse_fileheader(imstream, size);
    const BITMAPINFOHEADER infhead = parse_infoheader(imstream, size);
    bmp_t                  image   = { .fsize = size, .npixels = (size - 54) / 4, .fhead = fhead, .infhead = infhead };

    if (!(image.pixel_buffer = malloc(size - 54))) {
        fprintf(stderr, "Error in %s @ line %d: malloc falied!\n", __FUNCTION__, __LINE__);
        return (bmp_t) {};
    }

    memcpy(image.pixel_buffer, size - 54, imstream + 54, size - 54);
    free(imstream);
    imstream = nullptr;
    return image;
}

static inline void image_info(const bmp_t* const restrict image) {
    printf(
        "File size %Lf MiBs\nPixel data start offset: %d\n"
        "BITMAPINFOHEADER size: %u\nImage width: %u\nImage height: %u\nNumber of planes: %hu\n"
        "Number of bits per pixel: %hu\nImage size: %u\nResolution PPM(X): %u\nResolution PPM(Y): %u\nNumber of used colormap entries: % u\n"
        "Number of important colors: % u\n",
        (long double) (image->fhead.bfSize) / (1024 * 1024LLU),
        image->fhead.bfOffBits,
        image->infhead.biSize,
        image->infhead.biWidth,
        image->infhead.biHeight,
        image->infhead.biPlanes,
        image->infhead.biBitCount,
        image->infhead.biSizeImage,
        image->infhead.biXPelsPerMeter,
        image->infhead.biYPelsPerMeter,
        image->infhead.biClrUsed,
        image->infhead.biClrImportant
    );

    switch (image->infhead.biCompression) {
        case 0  : puts("BITMAPINFOHEADER.CMPTYPE: RGB"); break;
        case 1  : puts("BITMAPINFOHEADER.CMPTYPE: RLE4"); break;
        case 2  : puts("BITMAPINFOHEADER.CMPTYPE: RLE8"); break;
        case 3  : puts("BITMAPINFOHEADER.CMPTYPE: BITFIELDS"); break;
        default : puts("BITMAPINFOHEADER.CMPTYPE: UNKNOWN"); break;
    }

    printf(
        "%s BMP file\n"
        "BMP pixel ordering: %s\n",
        image->infhead.biSizeImage != 0 ? "Compressed" : "Uncompressed",
        image->infhead.biHeight >= 0 ? "BOTTOMUP" : "TOPDOWN"
    );
}

// ASCII characters in descending order of luminance
// static const char ascii[] = { 'N', '@', '#', 'W', '$', '9', '8', '7', '6', '5', '4', '3', '2', '1',
//                               '?', '!', 'a', 'b', 'c', ';', ':', '+', '=', '-', ',', '.', '_' };

static const char ascii[] = { '_', '.', ',', '-', '=', '+', ':', ';', 'c', 'b', 'a', '!', '?', '1',
                              '2', '3', '4', '5', '6', '7', '8', '9', '$', 'W', '#', '@', 'N' };

static char scale_pixel(const RGBQUAD* const restrict pixel) {
    return ascii[((((long) pixel->rgbBlue) + pixel->rgbGreen + pixel->rgbRed) / 3) % 27];
}

typedef struct _charbuf {
        const char*         buffer;
        const unsigned long length; // count of char s in the buffer.
} charbuf_t;

static inline charbuf_t to_text(const bmp_t* const restrict image) {
    // TODO
    // we need to downscale the ascii art if the image is larger than the console window
    // a fullscreen cmd window is 215 chars wide and 50 chars high
    char* txtbuff = malloc((image->infhead.biHeight * image->infhead.biWidth + image->infhead.biHeight) * sizeof(char));
    if (!txtbuff) {
        fwprintf_s(stderr, L"Error in %s @ line %d: malloc failed!\n", __FUNCTIONW__, __LINE__);
        return (charbuf_t) { nullptr, 0 };
    }

    unsigned long h = 0, w = 0;
    for (; h < image->infhead.biHeight; ++h) {
        w = 0;
        for (; w < image->infhead.biWidth; ++w)
            txtbuff[h * image->infhead.biWidth + w] = scale_pixel(&image->pixel_buffer[h * image->infhead.biWidth + w]);
        // txtbuff[h * w + 1] = '\n';
    }
    return (charbuf_t) { txtbuff, (image->infhead.biHeight * image->infhead.biWidth + image->infhead.biHeight) };
}

int main(const int argc, char* argv[]) {
    unsigned long fsize = 0;

    for (unsigned long i = 1; i < argc; ++i) {
        const unsigned char* buffer = imopen(argv[i], &fsize);

        if (buffer) {
            const bmp_t image = new_image(buffer, fsize);

            if (image.pixel_buffer) {
                image_info(&image);

                const charbuf_t txt = to_text(&image);
                if (txt.buffer) {
                    for (unsigned long i = 0; i < txt.length; ++i) {
                        // putwchar(txt.buffer[i]);
                        // printf_s("%d: %c ", txt.buffer[i], txt.buffer[i]);
                        putchar(txt.buffer[i]);
                    }
                    free(txt.buffer);
                }
                free(image.pixel_buffer);
            }

        } else {
            fwprintf_s(stderr, L"Skipping image %s\n", argv[i]);
            continue;
        }
    }

    return EXIT_SUCCESS;
}
