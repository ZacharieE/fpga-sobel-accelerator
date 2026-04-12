#include <iostream>
#include <fstream>
#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <ap_int.h>

// Match DUT definitions
#define WIDTH  640
#define HEIGHT 480

typedef ap_axiu<24,1,1,1> axis_pixel;

// DUT prototype
void sobel_video_stream(hls::stream<axis_pixel>& in_pix,
                        hls::stream<axis_pixel>& out_pix);

int main()
{
    hls::stream<axis_pixel> in_stream;
    hls::stream<axis_pixel> out_stream;

    // Create a simple synthetic input:
    // black background with a white rectangle in the middle
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            axis_pixel pkt;

            ap_uint<8> val = 0;
            if (x >= 200 && x < 440 && y >= 140 && y < 340) {
                val = 255;
            }

            // Pack grayscale value into RGB888
            ap_uint<24> rgb = 0;
            rgb.range(23,16) = val;
            rgb.range(15,8)  = val;
            rgb.range(7,0)   = val;

            pkt.data = rgb;
            pkt.keep = 0x7;
            pkt.strb = 0x7;
            pkt.user = (x == 0 && y == 0) ? 1 : 0;
            pkt.last = (x == WIDTH - 1) ? 1 : 0;
            pkt.id   = 0;
            pkt.dest = 0;

            in_stream.write(pkt);
        }
    }

    // Run DUT
    sobel_video_stream(in_stream, out_stream);

    // Open output image (PPM format, easy to inspect)
    std::ofstream ppm("sobel_video_out.ppm");
    ppm << "P3\n" << WIDTH << " " << HEIGHT << "\n255\n";

    int errors = 0;
    int user_count = 0;
    int last_count = 0;

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (out_stream.empty()) {
                std::cout << "ERROR: output stream ended early at ("
                          << x << ", " << y << ")\n";
                return 1;
            }

            axis_pixel pkt = out_stream.read();

            ap_uint<8> r = pkt.data.range(23,16);
            ap_uint<8> g = pkt.data.range(15,8);
            ap_uint<8> b = pkt.data.range(7,0);

            // Check grayscale RGB replication
            if (!(r == g && g == b)) {
                if (errors < 20) {
                    std::cout << "ERROR: RGB mismatch at ("
                              << x << ", " << y << ") -> "
                              << (int)r << ", "
                              << (int)g << ", "
                              << (int)b << "\n";
                }
                errors++;
            }

            // Check TUSER only on first pixel
            if (pkt.user == 1) {
                user_count++;
                if (!(x == 0 && y == 0)) {
                    std::cout << "ERROR: TUSER asserted at wrong location ("
                              << x << ", " << y << ")\n";
                    errors++;
                }
            }

            // Check TLAST at end of each line
            if (pkt.last == 1) {
                last_count++;
                if (x != WIDTH - 1) {
                    std::cout << "ERROR: TLAST asserted early at ("
                              << x << ", " << y << ")\n";
                    errors++;
                }
            } else {
                if (x == WIDTH - 1) {
                    std::cout << "ERROR: TLAST missing at end of line y="
                              << y << "\n";
                    errors++;
                }
            }

            // Write pixel to PPM
            ppm << (int)r << " " << (int)g << " " << (int)b << "\n";
        }
    }

    ppm.close();

    if (!out_stream.empty()) {
        std::cout << "ERROR: output stream has extra data left over\n";
        errors++;
    }

    std::cout << "TUSER count = " << user_count << " (expected 1)\n";
    std::cout << "TLAST count = " << last_count
              << " (expected " << HEIGHT << ")\n";
    std::cout << "Total errors = " << errors << "\n";

    if (user_count != 1) {
        std::cout << "ERROR: expected exactly one TUSER\n";
        errors++;
    }

    if (last_count != HEIGHT) {
        std::cout << "ERROR: expected TLAST once per line\n";
        errors++;
    }

    if (errors == 0) {
        std::cout << "TEST PASSED\n";
        return 0;
    } else {
        std::cout << "TEST FAILED\n";
        return 1;
    }
}
