#include <ap_int.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <cmath>

#define WIDTH  640
#define HEIGHT 480

// 24-bit RGB AXI stream with TUSER/TLAST support
typedef ap_axiu<24,1,1,1> axis_pixel;

// Simple RGB888 -> grayscale conversion
static ap_uint<8> rgb_to_gray(ap_uint<24> rgb)
{
#pragma HLS INLINE
    ap_uint<8> r = rgb.range(23, 16);
    ap_uint<8> g = rgb.range(15, 8);
    ap_uint<8> b = rgb.range(7, 0);

    // Simple average for first working version
    ap_uint<10> sum = (ap_uint<10>)r + (ap_uint<10>)g + (ap_uint<10>)b;
    return (ap_uint<8>)(sum / 3);
}

void sobel_video_stream(hls::stream<axis_pixel>& in_pix,
                        hls::stream<axis_pixel>& out_pix)
{
#pragma HLS INTERFACE axis port=in_pix
#pragma HLS INTERFACE axis port=out_pix
#pragma HLS INTERFACE ap_ctrl_none port=return

    // Two previous rows
    static ap_uint<8> line1[WIDTH];
    static ap_uint<8> line2[WIDTH];
#pragma HLS BIND_STORAGE variable=line1 type=ram_2p impl=bram
#pragma HLS BIND_STORAGE variable=line2 type=ram_2p impl=bram

    // Reset line buffers at start of each frame
    for (int i = 0; i < WIDTH; i++) {
#pragma HLS PIPELINE II=1
        line1[i] = 0;
        line2[i] = 0;
    }

    ap_uint<8> w00 = 0, w01 = 0, w02 = 0;
    ap_uint<8> w10 = 0, w11 = 0, w12 = 0;
    ap_uint<8> w20 = 0, w21 = 0, w22 = 0;

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
#pragma HLS PIPELINE II=1

            axis_pixel in_pkt = in_pix.read();
            ap_uint<8> pix = rgb_to_gray(in_pkt.data);

            ap_uint<8> p1 = line1[x];
            ap_uint<8> p2 = line2[x];

            // Shift 3x3 window
            w00 = w01; w01 = w02; w02 = p2;
            w10 = w11; w11 = w12; w12 = p1;
            w20 = w21; w21 = w22; w22 = pix;

            // Update line buffers
            line2[x] = line1[x];
            line1[x] = pix;

            ap_uint<8> outval = 0;

            if (y >= 2 && x >= 2) {
                int gx =
                    -(int)w00 + (int)w02
                    -2 * (int)w10 + 2 * (int)w12
                    -(int)w20 + (int)w22;

                int gy =
                    -(int)w00 - 2 * (int)w01 - (int)w02
                    + (int)w20 + 2 * (int)w21 + (int)w22;

                int mag = std::abs(gx) + std::abs(gy);
                if (mag > 255) mag = 255;
                outval = (ap_uint<8>)mag;
            }

            // Replicate grayscale edge output into RGB888
            ap_uint<24> out_rgb = 0;
            out_rgb.range(23, 16) = outval;
            out_rgb.range(15, 8)  = outval;
            out_rgb.range(7, 0)   = outval;

            axis_pixel out_pkt;
            out_pkt.data = out_rgb;
            out_pkt.keep = 0x7;
            out_pkt.strb = 0x7;
            out_pkt.user = (x == 0 && y == 0) ? 1 : 0;   // SOF
            out_pkt.last = (x == WIDTH - 1) ? 1 : 0;     // EOL
            out_pkt.id   = 0;
            out_pkt.dest = 0;

            out_pix.write(out_pkt);
        }
    }
}
