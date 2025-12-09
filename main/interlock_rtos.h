#pragma once

// This file exists for a dumb reason. I need (want) to define 
// configSUPPORT_STATIC_ALLOCATION but there's no way to do it in cmake or with
// menuconfig without breaking things. 
//
// To fix it, we will only use this header when including FreeRTOS headers as
// we have the definition here.

#define configSUPPORT_STATIC_ALLOCATION
