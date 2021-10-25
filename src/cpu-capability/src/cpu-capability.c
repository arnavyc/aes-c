#include <stdbool.h>
#include <stdint.h>

#include <ay/cpu-capability.h>

#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__)
#include <cpuid.h>
#endif

void cpuid_x86(uint32_t regs[4], uint32_t leaf) {
#if defined(_MSC_VER)
  __cpuid((uint32_t *)regs, leaf);
#elif defined(__GNUC__)
  __get_cpuid(leaf, &regs[0], &regs[1], &regs[2], &regs[3]);
#endif
}

static inline bool is_bit_set(uint32_t reg, unsigned char bit_location) {
  return (reg >> bit_location) & 0x01;
}

void cpu_capability_x86_init(struct cpu_capability_x86 *ctx) {
  unsigned int cpuid_regs_01h[4] = {0};
  cpuid_x86(cpuid_regs_01h, 0x01);

#if defined(AY_ARCH_X86_64)
  ctx->sse = true;
  ctx->sse2 = true;
#else
  ctx->sse = is_bit_set(cpuid_regs_01h[3], 25);
  ctx->sse2 = is_bit_set(cpuid_regs_01h[3], 26);
#endif

  ctx->pclmulqdq = is_bit_set(cpuid_regs_01h[2], 1);
  ctx->ssse3 = is_bit_set(cpuid_regs_01h[2], 9);
  ctx->aes = is_bit_set(cpuid_regs_01h[2], 25);
}
