/*
 * SPDX-FileCopyrightText: Copyright (c) 2021-2022 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "gpu/gpu.h"
#include "os/os.h"
#include "nverror.h"

#include "published/hopper/gh100/hwproject.h"
#include "published/hopper/gh100/dev_gc6_island.h"
#include "published/hopper/gh100/dev_pmc.h"
#include "published/hopper/gh100/dev_xtl_ep_pcfg_gpu.h"
#include "published/hopper/gh100/pri_nv_xal_ep.h"

#include "gpu/gsp/gsp_static_config.h"

/*!
 * @brief Write to non-private registers in XTL using config cycles
 *
 * @param[in] pGpu       GPU object pointer
 * @param[in] index      Register offset in PCIe config space
 * @param[in] value      Write this value to the register
 *
 * @returns    NV_OK on success
 */
NV_STATUS
gpuWriteBusConfigReg_GH100
(
    OBJGPU    *pGpu,
    NvU32      index,
    NvU32      value
)
{
    return gpuWriteBusConfigCycle(pGpu, index, value);
}

/*!
 * @brief Read the non-private registers in XTL using config cycles
 *
 * @param[in] pGpu       GPU object pointer
 * @param[in] index      Register offset in PCIe config space
 * @param[out] pData     Value of the register
 *
 * @returns    NV_OK on success
 */
NV_STATUS
gpuReadBusConfigReg_GH100
(
    OBJGPU    *pGpu,
    NvU32      index,
    NvU32     *pData
)
{
    return gpuReadBusConfigCycle(pGpu, index, pData);
}

/*!
 * @brief Get GPU ID based on PCIE config reads.
 * Also determine other properties of the PCIE capabilities.
 *
 * @param[in]   pGpu  OBJGPU pointer
 * @returns void.
 */
void
gpuGetIdInfo_GH100
(
    OBJGPU *pGpu
)
{
    NvU32   data;
    NvU32   deviceId;

    if (GPU_BUS_CFG_CYCLE_RD32(pGpu,
                             NV_EP_PCFG_GPU_REVISION_ID_AND_CLASSCODE,
                             &data) != NV_OK)
    {
        NV_PRINTF(LEVEL_ERROR,
                  "unable to read NV_EP_PCFG_GPU_REVISION_ID_AND_CLASSCODE\n");
        return;
    }

    // we only need the FIB and MASK values
    pGpu->idInfo.PCIRevisionID = (data & ~GPU_DRF_SHIFTMASK(NV_EP_PCFG_GPU_REVISION_ID_AND_CLASSCODE_PGM_INTERFACE)
                                       & ~GPU_DRF_SHIFTMASK(NV_EP_PCFG_GPU_REVISION_ID_AND_CLASSCODE_SUB_CLASSCODE)
                                       & ~GPU_DRF_SHIFTMASK(NV_EP_PCFG_GPU_REVISION_ID_AND_CLASSCODE_BASE_CLASSCODE));

    if (GPU_BUS_CFG_CYCLE_RD32(pGpu, NV_EP_PCFG_GPU_SUBSYSTEM_ID,
                             &data) != NV_OK)
    {
        NV_PRINTF(LEVEL_ERROR, "unable to read NV_EP_PCFG_GPU_SUBSYSTEM_ID\n");
        return;
    }
    pGpu->idInfo.PCISubDeviceID = data;

    if (GPU_BUS_CFG_CYCLE_RD32(pGpu, NV_EP_PCFG_GPU_ID, &deviceId) != NV_OK)
    {
        NV_PRINTF(LEVEL_ERROR, "unable to read NV_EP_PCFG_GPU_ID\n");
        return;
    }
    // For universal GPU use unlatched value
    pGpu->idInfo.PCIDeviceID = deviceId;

}

/*!
 * @brief Returns the physical address width for the given @ref NV_ADDRESS_SPACE
 */
NvU32 gpuGetPhysAddrWidth_GH100
(
    OBJGPU          *pGpu,
    NV_ADDRESS_SPACE addrSp
)
{
    // Currently this function supports only sysmem addresses
    NV_ASSERT_OR_RETURN(ADDR_SYSMEM == addrSp, 0);

    return NV_CHIP_EXTENDED_SYSTEM_PHYSICAL_ADDRESS_BITS;
}

/**
 * @brief Check if the GPU supports ATS with SMC memory partitioning enabled.
 *
 * @param pGpu
 *
 * @return NV_TRUE if ATS is supported with SMC memory partitioning
 *         NV_FALSE otherwise
 */
NvBool gpuIsAtsSupportedWithSmcMemPartitioning_GH100(OBJGPU *pGpu)
{

    if (IS_SIMULATION(pGpu))
    {
        return NV_TRUE;
    }

    return NV_FALSE;
}

/*!
 * @brief Read back device ID and subsystem ID
 *
 * @note This function is designed to avoid OBJBIF dependence for the case that
 *       RM need to get device id and subsystem id early in the init process,
 *       before OBJBIF is initialized.
 *
 * @param[in]  pGpu   GPU object pointer
 * @param[out] pDevId Return value of device ID
 * @param[out] pSsId  Return value of subsystem ID
 */
void
gpuReadDeviceId_GH100
(
    OBJGPU *pGpu,
    NvU32   *pDevId,
    NvU32   *pSsId
)
{
    NvU32   deviceId;
    NvU32   ssId;

    if (pDevId == NULL || pSsId == NULL) return;

    if (GPU_BUS_CFG_CYCLE_RD32(pGpu, NV_EP_PCFG_GPU_ID, &deviceId) != NV_OK)
    {
        NV_PRINTF(LEVEL_ERROR,
                  "Unable to read NV_EP_PCFG_GPU_ID\n");
        return;
    }
    *pDevId = deviceId;

    if (GPU_BUS_CFG_CYCLE_RD32(pGpu, NV_EP_PCFG_GPU_SUBSYSTEM_ID, &ssId) != NV_OK)
    {
        NV_PRINTF(LEVEL_ERROR,
                  "Unable to read NV_EP_PCFG_GPU_SUBSYSTEM_ID\n");
        return;
    }
    *pSsId = ssId;
}

/*!
 * @brief        Handle SEC_FAULT
 *
 * @param[in]    pGpu   GPU object pointer
 */
void
gpuHandleSecFault_GH100
(
    OBJGPU *pGpu
)
{
    NvU32   secDebug = 0;

    //
    // Read SEC_FAULT config space to determine what went wrong.
    // Do not return early on error, we must take the GPU down.
    //
    NV_ASSERT_OK(GPU_BUS_CFG_CYCLE_RD32(pGpu, NV_EP_PCFG_GPU_VSEC_DEBUG_SEC, &secDebug));

    MODS_ARCH_ERROR_PRINTF("NV_EP_PCFG_GPU_VSEC_DEBUG_SEC:0x%x\n", secDebug);
    NV_PRINTF(LEVEL_FATAL, "SEC_FAULT lockdown detected. This is fatal. "
                            "RM will now shut down. NV_EP_PCFG_GPU_VSEC_DEBUG_SEC: 0x%x\n", secDebug);

// (void)REF_NUM grants us mcheck protection
#define LOG_SEC_FAULT(field) \
    (void)REF_NUM(NV_EP_PCFG_GPU_VSEC_DEBUG_SEC##field, 0); \
    if (DRF_VAL(_EP_PCFG_GPU, _VSEC_DEBUG_SEC, field, secDebug) != 0) \
    { \
        MODS_ARCH_ERROR_PRINTF("NV_EP_PCFG_GPU_VSEC_DEBUG_SEC" #field "\n"); \
        NV_PRINTF(LEVEL_FATAL, "SEC_FAULT type: " #field "\n"); \
        nvErrorLog_va((void *)(pGpu), SEC_FAULT_ERROR, \
                      "SEC_FAULT: " #field ); \
    }

    LOG_SEC_FAULT(_FAULT_FUSE_POD);
    LOG_SEC_FAULT(_FAULT_FUSE_SCPM);
    LOG_SEC_FAULT(_FAULT_FSP_SCPM);
    LOG_SEC_FAULT(_FAULT_SEC2_SCPM);
    LOG_SEC_FAULT(_FAULT_FSP_DCLS);
    LOG_SEC_FAULT(_FAULT_SEC2_DCLS);
    LOG_SEC_FAULT(_FAULT_GSP_DCLS);
    LOG_SEC_FAULT(_FAULT_PMU_DCLS);
    LOG_SEC_FAULT(_FAULT_SEQ_TOO_BIG);
    LOG_SEC_FAULT(_FAULT_PRE_IFF_CRC);
    LOG_SEC_FAULT(_FAULT_POST_IFF_CRC);
    LOG_SEC_FAULT(_FAULT_ECC);
    LOG_SEC_FAULT(_FAULT_CMD);
    LOG_SEC_FAULT(_FAULT_PRI);
    LOG_SEC_FAULT(_FAULT_WDG);
    LOG_SEC_FAULT(_FAULT_BOOTFSM);
    LOG_SEC_FAULT(_IFF_POS);

#undef LOG_SEC_FAULT

    //
    // After SEC_FAULT occurs, the GPU will only return SCPM dummy values until properly reset.
    // Only cold reset will clear SEC_FAULT, not hot reset. This GPU is as good as lost.
    // handleGpuLost first to setGpuDisconnectedProperties so that another reg read does not
    // happen when the notifier is sent below.
    //
    osHandleGpuLost(pGpu);

    //
    // Send SEC_FAULT notification. This should tells any MODS test testing for this
    // error to pass and exit
    //
    gpuNotifySubDeviceEvent(pGpu,
                            NV2080_NOTIFIERS_SEC_FAULT_ERROR,
                            NULL,
                            0,
                            0,
                            SEC_FAULT_ERROR);
}

/*!
 * @brief Perform gpu-dependent error handling for error during register read sanity check
 *
 * @param[in]       pGpu        GPU object pointer
 * @param[in]       addr        Value address
 * @param[in]       value       Value read during check
 */
void
gpuHandleSanityCheckRegReadError_GH100
(
    OBJGPU *pGpu,
    NvU32 addr,
    NvU32 value
)
{
    // SEC_FAULT possibly detected, confirm by reading NV_PMC_BOOT_0
    if ((value == NV_XAL_EP_SCPM_PRI_DUMMY_DATA_PATTERN_INIT) &&
        (osGpuReadReg032(pGpu, NV_PMC_BOOT_0) == NV_XAL_EP_SCPM_PRI_DUMMY_DATA_PATTERN_INIT))
    {
        gpuHandleSecFault_HAL(pGpu);
    }
    else
    {
        NvU32 intr = ~0U;
        {
            //
            // Read the interrupt status using the direct OS reg read call so we don't recurs
            // if we happen to see GPU_READ_PRI_ERROR_CODE there as well (bug 799876)
            //
            intr = osGpuReadReg032(pGpu, NV_XAL_EP_INTR_0);
        }


        // To be sure, filter this down further by checking the related pri interrupts:
        if (FLD_TEST_DRF(_XAL_EP, _INTR_0, _PRI_FECSERR, _PENDING, intr) ||
            FLD_TEST_DRF(_XAL_EP, _INTR_0, _PRI_REQ_TIMEOUT, _PENDING, intr) ||
            FLD_TEST_DRF(_XAL_EP, _INTR_0, _PRI_RSP_TIMEOUT, _PENDING, intr))
        {
#if NV_PRINTF_STRINGS_ALLOWED
            const char *errorString = "Unknown SYS_PRI_ERROR_CODE";
            gpuGetSanityCheckRegReadError_HAL(pGpu, value,
                                              &errorString);
            NV_PRINTF(LEVEL_ERROR,
                      "Possible bad register read: addr: 0x%x,  regvalue: 0x%x,  error code: %s\n",
                      addr, value, errorString);
#else // NV_PRINTF_STRINGS_ALLOWED
            NV_PRINTF(LEVEL_ERROR,
                      "Possible bad register read: addr: 0x%x,  regvalue: 0x%x\n",
                      addr, value);
#endif // NV_PRINTF_STRINGS_ALLOWED
        }
    }
}

/*!
 * @brief Returns FLA VASpace Size for Hopper+
 *
 * @param[in] pGpu     OBJGPU pointer
 * @param[in] bNvSwitchVirtualization     boolean
 *
 * @returns NvU64 -> size of FLA VASpace
 */
NvU64
gpuGetFlaVasSize_GH100
(
    OBJGPU *pGpu,
    NvBool  bNvswitchVirtualization
)
{
    return 0x8000000000;  // 512GB
}

//
// List of GPU children that present for the chip. List entries contain$
// {CLASS-ID, # of instances} pairs, e.g.: {CE, 2} is 2 instance of OBJCE. This$
// list controls only engine presence. Order is defined by$
// gpuGetChildrenOrder_HAL.$
//
// IMPORTANT: This function is to be deleted. Engine removal should instead be$
// handled by <eng>ConstructEngine returning NV_ERR_NOT_SUPPORTED. PLEASE DO NOT$
// FORK THIS LIST!$
//
// List entries contain {CLASS-ID, # of instances} pairs.
//

static const GPUCHILDPRESENT gpuChildrenPresent_GH100[] =
{
    { classId(OBJSWENG), 1 },
    { classId(OBJUVM), 1 },
    { classId(OBJACR), 1 },
    { classId(OBJBIF), 1 },
    { classId(KernelBif), 1 },
    { classId(OBJBSP), 8 },
    { classId(OBJBUS), 1 },
    { classId(KernelBus), 1 },
    { classId(OBJCE), 10 },
    { classId(KernelCE), 10 },
    { classId(OBJCIPHER), 1 },
    { classId(ClockManager), 1 },
    { classId(OBJDISP), 1 },
    { classId(KernelDisplay), 1 },
    { classId(VirtMemAllocator), 1 },
    { classId(OBJDPAUX), 1 },
    { classId(Fan), 1 },
    { classId(OBJHSHUBMANAGER), 1 },
    { classId(Hshub), 5 },
    { classId(MemorySystem), 1 },
    { classId(KernelMemorySystem), 1 },
    { classId(MemoryManager), 1 },
    { classId(OBJFBFLCN), 1 },
    {classId(KernelFifo), 1},
    { classId(OBJFIFO), 1 },
    { classId(OBJGMMU), 1 },
    { classId(KernelGmmu), 1},
    { classId(OBJGPULOG), 1 },
    { classId(OBJGPUMON), 1 },
    { classId(GraphicsManager), 1 },
    { classId(MIGManager), 1 },
    { classId(KernelMIGManager), 1 },
    { classId(KernelGraphicsManager), 1 },
    { classId(Graphics), 8 },
    { classId(KernelGraphics), 8 },
    { classId(OBJHDACODEC), 1 },
    { classId(OBJHWPM), 1 },
    { classId(OBJINFOROM), 1 },
    { classId(Intr), 1 },
    { classId(Lpwr   ), 1 },
    { classId(OBJLSFM), 1 },
    { classId(OBJMC), 1 },
    { classId(KernelMc), 1 },
    { classId(PrivRing), 1 },
    { classId(SwIntr), 1 },
    { classId(OBJNVJPG), 8 },
    { classId(NvDebugDump), 1 },
    { classId(KernelNvlink), 1 },
    { classId(Nvlink), 1 },
    { classId(Perf), 1 },
    { classId(KernelPerf), 1 },
    { classId(Pmgr), 1 },
    { classId(Pmu), 1 },
    { classId(KernelPmu), 1 },
    { classId(OBJSEC2), 1 },
    { classId(Gsp), 1 },
    { classId(OBJFSP), 1 },
    { classId(KernelFsp), 1 },
    { classId(Therm), 1 },
    { classId(OBJTMR), 1 },
    { classId(OBJVOLT), 1 },
    { classId(OBJGRIDDISPLAYLESS), 1 },
    { classId(OBJFAS), 1 },
    { classId(OBJVMMU), 1 },
    { classId(OBJOFA), 1 },
    { classId(KernelGsp), 1},
    { classId(KernelCcu), 1 },
};

const GPUCHILDPRESENT *
gpuGetChildrenPresent_GH100(OBJGPU *pGpu, NvU32 *pNumEntries)
{
    *pNumEntries = NV_ARRAY_ELEMENTS32(gpuChildrenPresent_GH100);
    return gpuChildrenPresent_GH100;
}

