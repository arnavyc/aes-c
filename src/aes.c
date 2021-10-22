#include <stdbool.h>
#include <stdio.h>

#include "aes-bs.h"
#include "aes-ni.h"
#include "inner.h"
#include <ay/aes.h>
#include <ay/cpu-capability.h>

#define BOOL_TO_STR(b) ((b) ? "true" : "false")

void aes_init(AesContext *ctx, enum AesKeyType key_type,
              const unsigned char *key) {
  struct cpu_capability_x86 cpufeat;
  cpu_capability_x86_init(&cpufeat);

  if (cpufeat.sse && cpufeat.sse2 && cpufeat.ssse3 && cpufeat.aes) {
    ctx->vtable = (struct aes_vtable){.init = aesni_init,
                                      .ctr_xcrypt = aesni_ctr_xcrypt,
                                      .ecb_encrypt = aesni_ecb_encrypt,
                                      .ecb_decrypt = aesni_ecb_decrypt,
                                      .cbc_encrypt = aesni_cbc_encrypt,
                                      .cbc_decrypt = aesni_cbc_decrypt};
    aesni_init(ctx, key_type, key);
  } else {
    ctx->vtable = (struct aes_vtable){.init = aesbs_init,
                                      .ctr_xcrypt = aesbs_ctr_xcrypt,
                                      .ecb_encrypt = aesbs_ecb_encrypt,
                                      .ecb_decrypt = aesbs_ecb_decrypt,
                                      .cbc_encrypt = aesbs_cbc_encrypt,
                                      .cbc_decrypt = aesbs_cbc_decrypt};
    aesbs_init(ctx, key_type, key);
  }
}

void aes_ctr_xcrypt(AesContext *ctx, size_t textsize, unsigned char *out,
                    const unsigned char *in, unsigned char next_iv[16],
                    const unsigned char iv[16]) {
  ctx->vtable.ctr_xcrypt(ctx, textsize, out, in, next_iv, iv);
}

void aes_ecb_encrypt(AesContext *ctx, size_t textsize,
                     unsigned char *cipher_text,
                     const unsigned char *plain_text) {
  ctx->vtable.ecb_encrypt(ctx, textsize, cipher_text, plain_text);
}

void aes_ecb_decrypt(AesContext *ctx, size_t textsize,
                     unsigned char *plain_text,
                     const unsigned char *cipher_text) {
  ctx->vtable.ecb_decrypt(ctx, textsize, plain_text, cipher_text);
}

void aes_cbc_encrypt(AesContext *ctx, size_t textsize,
                     unsigned char *cipher_text,
                     const unsigned char *plain_text, const unsigned char *iv) {
  ctx->vtable.cbc_encrypt(ctx, textsize, cipher_text, plain_text, iv);
}

void aes_cbc_decrypt(AesContext *ctx, size_t textsize,
                     unsigned char *plain_text,
                     const unsigned char *cipher_text,
                     const unsigned char *iv) {
  ctx->vtable.cbc_decrypt(ctx, textsize, plain_text, cipher_text, iv);
}
