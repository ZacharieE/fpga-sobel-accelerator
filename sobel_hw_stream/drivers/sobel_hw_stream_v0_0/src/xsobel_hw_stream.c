// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2020.2 (64-bit)
// Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
// ==============================================================
/***************************** Include Files *********************************/
#include "xsobel_hw_stream.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XSobel_hw_stream_CfgInitialize(XSobel_hw_stream *InstancePtr, XSobel_hw_stream_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Ctrl_BaseAddress = ConfigPtr->Ctrl_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XSobel_hw_stream_Start(XSobel_hw_stream *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSobel_hw_stream_ReadReg(InstancePtr->Ctrl_BaseAddress, XSOBEL_HW_STREAM_CTRL_ADDR_AP_CTRL) & 0x80;
    XSobel_hw_stream_WriteReg(InstancePtr->Ctrl_BaseAddress, XSOBEL_HW_STREAM_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XSobel_hw_stream_IsDone(XSobel_hw_stream *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSobel_hw_stream_ReadReg(InstancePtr->Ctrl_BaseAddress, XSOBEL_HW_STREAM_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XSobel_hw_stream_IsIdle(XSobel_hw_stream *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSobel_hw_stream_ReadReg(InstancePtr->Ctrl_BaseAddress, XSOBEL_HW_STREAM_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XSobel_hw_stream_IsReady(XSobel_hw_stream *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSobel_hw_stream_ReadReg(InstancePtr->Ctrl_BaseAddress, XSOBEL_HW_STREAM_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XSobel_hw_stream_EnableAutoRestart(XSobel_hw_stream *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSobel_hw_stream_WriteReg(InstancePtr->Ctrl_BaseAddress, XSOBEL_HW_STREAM_CTRL_ADDR_AP_CTRL, 0x80);
}

void XSobel_hw_stream_DisableAutoRestart(XSobel_hw_stream *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSobel_hw_stream_WriteReg(InstancePtr->Ctrl_BaseAddress, XSOBEL_HW_STREAM_CTRL_ADDR_AP_CTRL, 0);
}

void XSobel_hw_stream_InterruptGlobalEnable(XSobel_hw_stream *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSobel_hw_stream_WriteReg(InstancePtr->Ctrl_BaseAddress, XSOBEL_HW_STREAM_CTRL_ADDR_GIE, 1);
}

void XSobel_hw_stream_InterruptGlobalDisable(XSobel_hw_stream *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSobel_hw_stream_WriteReg(InstancePtr->Ctrl_BaseAddress, XSOBEL_HW_STREAM_CTRL_ADDR_GIE, 0);
}

void XSobel_hw_stream_InterruptEnable(XSobel_hw_stream *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XSobel_hw_stream_ReadReg(InstancePtr->Ctrl_BaseAddress, XSOBEL_HW_STREAM_CTRL_ADDR_IER);
    XSobel_hw_stream_WriteReg(InstancePtr->Ctrl_BaseAddress, XSOBEL_HW_STREAM_CTRL_ADDR_IER, Register | Mask);
}

void XSobel_hw_stream_InterruptDisable(XSobel_hw_stream *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XSobel_hw_stream_ReadReg(InstancePtr->Ctrl_BaseAddress, XSOBEL_HW_STREAM_CTRL_ADDR_IER);
    XSobel_hw_stream_WriteReg(InstancePtr->Ctrl_BaseAddress, XSOBEL_HW_STREAM_CTRL_ADDR_IER, Register & (~Mask));
}

void XSobel_hw_stream_InterruptClear(XSobel_hw_stream *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSobel_hw_stream_WriteReg(InstancePtr->Ctrl_BaseAddress, XSOBEL_HW_STREAM_CTRL_ADDR_ISR, Mask);
}

u32 XSobel_hw_stream_InterruptGetEnabled(XSobel_hw_stream *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XSobel_hw_stream_ReadReg(InstancePtr->Ctrl_BaseAddress, XSOBEL_HW_STREAM_CTRL_ADDR_IER);
}

u32 XSobel_hw_stream_InterruptGetStatus(XSobel_hw_stream *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XSobel_hw_stream_ReadReg(InstancePtr->Ctrl_BaseAddress, XSOBEL_HW_STREAM_CTRL_ADDR_ISR);
}

