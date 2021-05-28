/** compile with -std=gnu99 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <unistd.h>

#include "armpmu_lib.h"

#define WRITE_REG(src, reg)            \
        {                              \
                asm("mov " #reg ", %0" \
                    :                  \
                    : "r"(src));       \
        }

/* Simple loop body to keep things interested. Make sure it gets inlined. */
static inline int
loop(int *__restrict__ a, int *__restrict__ b, int n)
{
        unsigned sum = 0;
        for (int i = 0; i < n; ++i)
                if (a[i] > b[i])
                        sum += a[i] + 5;
        return sum;
}

void gao(int pid)
{
}

int main(int ac, char **av)
{
        uint32_t time_start = 0;
        uint32_t time_end = 0;
        uint32_t cnt_start = 0;
        uint32_t cnt_end = 0;

        int *a = NULL;
        int *b = NULL;
        int len = 0;
        int sum = 0;

        if (ac != 2)
                return -1;
        len = atoi(av[1]);
        printf("%s: len = %d\n", av[0], len);

        a = malloc(len * sizeof(*a));
        b = malloc(len * sizeof(*b));

        for (int i = 0; i < len; ++i)
        {
                a[i] = i + 128;
                b[i] = i + 64;
        }

        printf("%s: beginning loop\n", av[0]);
        // ARM_PMU_INST_RETIRED   0x0008

        enable_pmu(0x008);
        time_start = rdtsc32();

        // 获取当前 PID
        int pid = getpid();
        printf("Current PID=%d\n", pid);

        // 将 PMU 计数器设成最大
        uint32_t r = 0xffffffff;
        asm volatile("msr pmevcntr0_el0, %0"
                     :
                     : "r"(r));

        // 将所有寄存器设成目前的 pid
        gao(pid);

        cnt_start = read_pmu();
        printf("%s: PMU start at %8x \n",av[0], cnt_start);
        sum = loop(a, b, len);
        // usleep(200);
        // __asm__("nop");
        cnt_end = read_pmu();
        printf("%s: PMU end at %8x \n",av[0], cnt_end);
        time_end = rdtsc32();
        disable_pmu(0x008);
        printf("%s: done. sum = %d; time delta = %u; event 0x%03x delta = %u\n", av[0], sum, time_end - time_start, 0x008, cnt_end - cnt_start);

        free(a);
        free(b);
        return 0;
}
