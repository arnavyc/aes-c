#ifndef AY_CPU_CAPABILITY_X86
#define AY_CPU_CAPABILITY_X86

#include <stdbool.h>
#include <stdint.h>

#if defined(__x86_64__) || defined(_M_AMD64) || defined(_M_X64)
#define AY_ARCH_X86_64
#endif

struct cpu_capability_x86 {
  /* Leaf = 01h */

  /* register = EDX */
  bool sse : 1;
  bool sse2 : 1;

  /* register = ECX */
  bool pclmulqdq : 1;
  bool ssse3 : 1;
  bool aes : 1;
};

void cpu_capability_x86_init(struct cpu_capability_x86 *ctx);
void cpuid_x86(uint32_t regs[4], uint32_t leaf);

#endif /* AY_CPU_CAPABILITY_X86 */
