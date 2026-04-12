#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <hls_stream.h>
#include <ap_int.h>
#include <ap_axi_sdata.h>

#define WIDTH  256
#define HEIGHT 256

typedef ap_axiu<8,0,0,0> axis_pixel;

// Declare the DUT (design under test)
void sobel_hw_stream(hls::stream<axis_pixel>& in_pix,
                     hls::stream<axis_pixel>& out_pix);

static bool read_pgm(const char* path, unsigned char img[HEIGHT][WIDTH]) {
    FILE* f = std::fopen(path, "rb");
    if (!f) return false;

    char magic[3] = {0};
    if (std::fscanf(f, "%2s", magic) != 1) { std::fclose(f); return false; }
    if (std::strcmp(magic, "P5") != 0) { std::fclose(f); return false; }

    int w=0,h=0,maxv=0;
    if (std::fscanf(f, "%d %d", &w, &h) != 2) { std::fclose(f); return false; }
    if (std::fscanf(f, "%d", &maxv) != 1) { std::fclose(f); return false; }
    if (w != WIDTH || h != HEIGHT || maxv != 255) { std::fclose(f); return false; }

    std::fgetc(f); // consume single whitespace after header
    size_t n = std::fread(img, 1, WIDTH*HEIGHT, f);
    std::fclose(f);
    return (n == (size_t)(WIDTH*HEIGHT));
}

static void sobel_reference(unsigned char in[HEIGHT][WIDTH],
                            unsigned char out[HEIGHT][WIDTH])
{
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++)
            out[y][x] = 0;

    for (int y = 2; y < HEIGHT; y++) {
        for (int x = 2; x < WIDTH; x++) {
            int gx =
                -in[y-2][x-2] + in[y-2][x]
                -2*in[y-1][x-2] + 2*in[y-1][x]
                -in[y][x-2] + in[y][x];

            int gy =
                -in[y-2][x-2] -2*in[y-2][x-1] -in[y-2][x]
                +in[y][x-2]   +2*in[y][x-1]   +in[y][x];

            int mag = std::abs(gx) + std::abs(gy);
            if (mag > 255) mag = 255;
            out[y][x] = (unsigned char)mag;
        }
    }
}

static void write_pgm(const char* path, unsigned char img[HEIGHT][WIDTH])
{
    FILE* f = std::fopen(path, "wb");
    if (!f) {
        std::printf("ERROR: cannot open %s\n", path);
        return;
    }
    std::fprintf(f, "P5\n%d %d\n255\n", WIDTH, HEIGHT);
    std::fwrite(img, 1, WIDTH*HEIGHT, f);
    std::fclose(f);
}

int main()
{
    static unsigned char in[HEIGHT][WIDTH];
    static unsigned char out_hw[HEIGHT][WIDTH];
    static unsigned char out_ref[HEIGHT][WIDTH];

    if (read_pgm("real_256.pgm", in)) {
        std::printf("Loaded real_256.pgm\n");
    } else {
        std::printf("real_256.pgm not found -> using synthetic square\n");
        std::memset(in, 0, sizeof(in));
        for (int y = 80; y < 176; y++)
            for (int x = 80; x < 176; x++)
                in[y][x] = 255;
    }

    // Streams are now AXIS packets
    hls::stream<axis_pixel> s_in;
    hls::stream<axis_pixel> s_out;

    // Push pixels into input stream (raster order)
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            axis_pixel pkt;
            pkt.data = (ap_uint<8>)in[y][x];
            pkt.last = (y == HEIGHT-1 && x == WIDTH-1); // TLAST on final pixel
            s_in.write(pkt);
        }
    }

    // Run hardware stream core
    sobel_hw_stream(s_in, s_out);

    // Read pixels out
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            axis_pixel outpkt = s_out.read();
            out_hw[y][x] = (unsigned char)outpkt.data;
        }
    }

    // Reference
    sobel_reference(in, out_ref);

    // Compare
    int errors = 0;
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (out_hw[y][x] != out_ref[y][x]) {
                if (errors < 10) {
                    std::printf("Mismatch at (%d,%d): hw=%d ref=%d\n",
                                y, x, (int)out_hw[y][x], (int)out_ref[y][x]);
                }
                errors++;
            }
        }
    }

    // Save outputs
    write_pgm("real_input.pgm", in);
    write_pgm("real_edges_hw.pgm", out_hw);
    write_pgm("real_edges_ref.pgm", out_ref);

    if (errors == 0) {
        std::printf("PASS: outputs match exactly.\n");
        return 0;
    } else {
        std::printf("FAIL: %d mismatches.\n", errors);
        return 1;
    }
}
