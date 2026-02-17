// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2020.2 (64-bit)
// Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
// ==============================================================
#ifndef __linux__

#include "xstatus.h"
#include "xparameters.h"
#include "xsobel_hw_stream.h"

extern XSobel_hw_stream_Config XSobel_hw_stream_ConfigTable[];

XSobel_hw_stream_Config *XSobel_hw_stream_LookupConfig(u16 DeviceId) {
	XSobel_hw_stream_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XSOBEL_HW_STREAM_NUM_INSTANCES; Index++) {
		if (XSobel_hw_stream_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XSobel_hw_stream_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XSobel_hw_stream_Initialize(XSobel_hw_stream *InstancePtr, u16 DeviceId) {
	XSobel_hw_stream_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XSobel_hw_stream_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XSobel_hw_stream_CfgInitialize(InstancePtr, ConfigPtr);
}

#endif

