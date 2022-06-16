#include "msr.h"

int validate_msr_lookup(unsigned __int64 msrRegister)
{
    int err;

    switch (machine_type)
    {
        case E_MACHINE_INTEL:
            switch (msrRegister)
            {
                case MSR_RAPL_POWER_UNIT:
                case MSR_PKG_POWER_LIMIT:
                case MSR_PKG_ENERGY_STATUS:
                case MSR_PKG_PERF_STATUS:
                case MSR_PKG_POWER_INFO:
                case MSR_DRAM_POWER_LIMIT:
                case MSR_DRAM_ENERGY_STATUS:
                case MSR_DRAM_PERF_STATUS:
                case MSR_DRAM_POWER_INFO:
                case MSR_PP0_POWER_LIMIT:
                case MSR_PP0_ENERGY_STATUS:
                case MSR_PP0_POLICY:
                case MSR_PP0_PERF_STATUS:
                case MSR_PP1_POWER_LIMIT:
                case MSR_PP1_ENERGY_STATUS:
                case MSR_PP1_POLICY:
                    err = 0;
                    break;

                default:
                    err = 1;
                    break;
            }
            break;

        case E_MACHINE_AMD:
            switch (msrRegister)
            {
                case MSR_AMD_RAPL_POWER_UNIT:
                case MSR_AMD_CORE_ENERGY_STATUS:
                case MSR_AMD_PKG_ENERGY_STATUS:
                    err = 0;
                    break;

                default:
                    err = 1;
                    break;
            }
            break;

        case E_MACHINE_UNK:
        default:
            err = 1;
            break;
    }

    return err;
}
