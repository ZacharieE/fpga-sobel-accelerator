// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2020.2 (64-bit)
// Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
// ==============================================================
#ifndef XSOBEL_HW_STREAM_H
#define XSOBEL_HW_STREAM_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#ifndef __linux__
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_io.h"
#else
#include <stdint.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>
#endif
#include "xsobel_hw_stream_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#else
typedef struct {
    u16 DeviceId;
    u32 Ctrl_BaseAddress;
} XSobel_hw_stream_Config;
#endif

typedef struct {
    u64 Ctrl_BaseAddress;
    u32 IsReady;
} XSobel_hw_stream;

typedef u32 word_type;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XSobel_hw_stream_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XSobel_hw_stream_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XSobel_hw_stream_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XSobel_hw_stream_ReadReg(BaseAddress, RegOffset) \
    *(volatile u32*)((BaseAddress) + (RegOffset))

#define Xil_AssertVoid(expr)    assert(expr)
#define Xil_AssertNonvoid(expr) assert(expr)

#define XST_SUCCESS             0
#define XST_DEVICE_NOT_FOUND    2
#define XST_OPEN_DEVICE_FAILED  3
#define XIL_COMPONENT_IS_READY  1
#endif

/************************** Function Prototypes *****************************/
#ifndef __linux__
int XSobel_hw_stream_Initialize(XSobel_hw_stream *InstancePtr, u16 DeviceId);
XSobel_hw_stream_Config* XSobel_hw_stream_LookupConfig(u16 DeviceId);
int XSobel_hw_stream_CfgInitialize(XSobel_hw_stream *InstancePtr, XSobel_hw_stream_Config *ConfigPtr);
#else
int XSobel_hw_stream_Initialize(XSobel_hw_stream *InstancePtr, const char* InstanceName);
int XSobel_hw_stream_Release(XSobel_hw_stream *InstancePtr);
#endif

void XSobel_hw_stream_Start(XSobel_hw_stream *InstancePtr);
u32 XSobel_hw_stream_IsDone(XSobel_hw_stream *InstancePtr);
u32 XSobel_hw_stream_IsIdle(XSobel_hw_stream *InstancePtr);
u32 XSobel_hw_stream_IsReady(XSobel_hw_stream *InstancePtr);
void XSobel_hw_stream_EnableAutoRestart(XSobel_hw_stream *InstancePtr);
void XSobel_hw_stream_DisableAutoRestart(XSobel_hw_stream *InstancePtr);


void XSobel_hw_stream_InterruptGlobalEnable(XSobel_hw_stream *InstancePtr);
void XSobel_hw_stream_InterruptGlobalDisable(XSobel_hw_stream *InstancePtr);
void XSobel_hw_stream_InterruptEnable(XSobel_hw_stream *InstancePtr, u32 Mask);
void XSobel_hw_stream_InterruptDisable(XSobel_hw_stream *InstancePtr, u32 Mask);
void XSobel_hw_stream_InterruptClear(XSobel_hw_stream *InstancePtr, u32 Mask);
u32 XSobel_hw_stream_InterruptGetEnabled(XSobel_hw_stream *InstancePtr);
u32 XSobel_hw_stream_InterruptGetStatus(XSobel_hw_stream *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
