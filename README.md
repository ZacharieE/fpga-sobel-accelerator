# FPGA Sobel Edge Detector (AXI-Stream + DMA + PYNQ)

Hardware-accelerated Sobel edge detector implemented using Vitis HLS and deployed on a PYNQ-Z2 platform.  
The design streams 256×256 grayscale images through AXI-DMA into a custom HLS IP core using a 3×3 sliding window architecture with line buffers.

---

## Project Overview

This project implements a hardware Sobel edge detector using:

- Vitis HLS (C++ → RTL)
- AXI-Stream interfaces
- AXI-DMA (MM2S + S2MM)
- Vivado Block Design
- PYNQ Python control

The system processes full images via streaming and returns the computed gradient magnitude.

---

## Architecture

### HLS IP Core

- AXI-Stream input/output (`ap_axiu<8>`)
- 3×3 sliding window
- Two line buffers (BRAM inferred)
- Pipelined with II=1
- Output: `|Gx| + |Gy|` clipped to 255
- TLAST asserted on final pixel

### Sobel Kernels

Gx =
[ -1   0   1
  -2   0   2
  -1   0   1 ]

Gy =
[-1  -2  -1
 0   0   0
 1   2   1]



---

## Vivado Block Design

- Zynq PS
- AXI DMA
- Custom Sobel HLS IP
- Shared clock: `FCLK_CLK0`
- Reset: `peripheral_aresetn`

**Configuration:**

- Stream data width: 8-bit
- Memory-mapped width: 32-bit
- Scatter-Gather: Disabled
- Single channel DMA

### DMA Execution Order

1. Start Sobel IP
2. Start S2MM (receive channel)
3. Start MM2S (send channel)
4. Wait for completion

---

## Software Control (PYNQ)

Python script performs:

- Bitstream loading
- DMA buffer allocation
- Cache flush/invalidate handling
- Transfer synchronization
- Output visualization

Supported formats:

- PGM (P5, 8-bit)
- PNG/JPG (converted to grayscale)

---

## Repository Structure

/hls/        -> HLS source (sobel_hw_stream.cpp)
/vivado/     -> Block design screenshots
/notebooks/  -> PYNQ control notebook
/images/     -> Input/output examples
README.md



---

## Key Technical Points

- Fully streaming architecture (no full-frame buffering)
- Sliding window + BRAM line buffers
- II=1 pipeline
- Correct TLAST handling for DMA stability
- Debugged AXI-DMA handshake and stall conditions
- Verified via:
  - HLS C simulation
  - RTL simulation
  - On-board hardware validation

---

## Results

Input → Hardware Sobel Output

Edge detection correctly highlights object boundaries using real hardware acceleration.

---

## Future Improvements

- Parameterizable image size
- Runtime-configurable kernel
- Throughput benchmarking
- AXI4-Lite configuration registers for dynamic control
