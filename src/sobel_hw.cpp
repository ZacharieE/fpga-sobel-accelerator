#include <ap_int.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <cmath>

#define WIDTH  256
#define HEIGHT 256

// 8-bit AXI-Stream with no extra sideband fields
typedef ap_axiu<8,0,0,0> axis_pixel;

void sobel_hw_stream(hls::stream<axis_pixel>& in_pix,
                     hls::stream<axis_pixel>& out_pix)
{
#pragma HLS INTERFACE axis port=in_pix
#pragma HLS INTERFACE axis port=out_pix
#pragma HLS INTERFACE s_axilite port=return bundle=CTRL

    // Line buffers (previous two rows)
    static ap_uint<8> line1[WIDTH];
    static ap_uint<8> line2[WIDTH];

    // Initialize only once
    static bool init_done = false;
    if (!init_done) {
        for (int i = 0; i < WIDTH; i++) {
#pragma HLS PIPELINE II=1
            line1[i] = 0;
            line2[i] = 0;
        }
        init_done = true;
    }

    ap_uint<8> w00=0,w01=0,w02=0;
    ap_uint<8> w10=0,w11=0,w12=0;
    ap_uint<8> w20=0,w21=0,w22=0;

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
#pragma HLS PIPELINE II=1

            // Read input stream
            axis_pixel in_pkt = in_pix.read();
            ap_uint<8> pix = in_pkt.data;

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
                    -2*(int)w10 + 2*(int)w12
                    -(int)w20 + (int)w22;

                int gy =
                    -(int)w00 -2*(int)w01 -(int)w02
                    +(int)w20 +2*(int)w21 +(int)w22;

                int mag = std::abs(gx) + std::abs(gy);
                if (mag > 255) mag = 255;
                outval = (ap_uint<8>)mag;
            }

            // Prepare output packet
            axis_pixel out_pkt;
            out_pkt.data = outval;

            // These are important for DMA stability
            out_pkt.keep = 1;   // 1 byte valid
            out_pkt.strb = 1;   // strobe active

            // TLAST only on final pixel
            out_pkt.last = (y == HEIGHT-1 && x == WIDTH-1);

            out_pix.write(out_pkt);
        }
    }
}
