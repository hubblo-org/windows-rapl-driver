#ifndef _MSR_H
#define _MSR_H

/* Run Time Average Power Limiting (RAPL) Interface */

#define MSR_RAPL_POWER_UNIT     0x00000606

#define MSR_PKG_POWER_LIMIT     0x00000610
#define MSR_PKG_ENERGY_STATUS   0x00000611
#define MSR_PKG_PERF_STATUS     0x00000613
#define MSR_PKG_POWER_INFO      0x00000614

#define MSR_DRAM_POWER_LIMIT    0x00000618
#define MSR_DRAM_ENERGY_STATUS  0x00000619
#define MSR_DRAM_PERF_STATUS    0x0000061b
#define MSR_DRAM_POWER_INFO     0x0000061c

#define MSR_PP0_POWER_LIMIT     0x00000638
#define MSR_PP0_ENERGY_STATUS   0x00000639
#define MSR_PP0_POLICY          0x0000063a
#define MSR_PP0_PERF_STATUS     0x0000063b

#define MSR_PP1_POWER_LIMIT     0x00000640
#define MSR_PP1_ENERGY_STATUS   0x00000641
#define MSR_PP1_POLICY          0x00000642

#define MSR_AMD_RAPL_POWER_UNIT     0xc0010299
#define MSR_AMD_CORE_ENERGY_STATUS  0xc001029a
#define MSR_AMD_PKG_ENERGY_STATUS   0xc001029b

#define MSR_PLATFORM_ENERGY_STATUS 0x0000064d
#define MSR_PLATFORM_POWER_LIMIT 0x0000065c


typedef enum {
    E_MACHINE_INTEL,
    E_MACHINE_AMD,
    E_MACHINE_UNK
}e_machine_type;


static e_machine_type machine_type;
static unsigned long max_processors;

int validate_msr_lookup(unsigned __int64 msrRegister);

#endif /* _MSR_H */
