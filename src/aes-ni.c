#include <assert.h>
#include <emmintrin.h>
#include <inttypes.h>
#include <string.h>
#include <tmmintrin.h>
#include <wmmintrin.h>
#include <xmmintrin.h>

#include "aes-ni.h"
#include <ay/aes.h>

#include "inner.h"

/**
 * @name Common functions
 * Functions used in AES-128, AES-192, AES-256 implementation
 */
/** @{ */

/**
 * @brief Expands the given `key_lower` into the decryption key schedule (since
 * Intel uses Equivalent Inverse Cipher in section 5.3.5 of FIPS 197)
 *
 * @param dec_key_schedule Pointer to array having the decryption key schedule
 * @param enc_key_schedule Pointer to array having the encryption key schedule
 * @param key_lower lower 128 bits of the AES key
 * @param Nr number of rounds in algorithm (10 for AES-128, 12 for AES-192, 14
 * for AES-256)
 */
static inline void aes_decryption_key_expansion(__m128i *dec_key_schedule,
                                                const __m128i *enc_key_schedule,
                                                __m128i key_lower,
                                                unsigned char Nr) {
  dec_key_schedule[Nr] = key_lower;
  for (size_t i = 1; i < Nr; ++i) {
    dec_key_schedule[Nr - i] = _mm_aesimc_si128(enc_key_schedule[i]);
  }
  dec_key_schedule[0] = enc_key_schedule[Nr];
}

static inline __m128i aes_encrypt_block(unsigned char Nr, const __m128i *enc_ks,
                                        __m128i plain_block) {
  __m128i block = _mm_xor_si128(plain_block, enc_ks[0]);
  size_t i = 1;
  while (i < Nr) {
    block = _mm_aesenc_si128(block, enc_ks[i++]);
  }
  block = _mm_aesenclast_si128(block, enc_ks[i]);

  return block;
};

static inline __m128i aes_decrypt_block(unsigned char Nr, const __m128i *dec_ks,
                                        __m128i cipher_block) {
  __m128i block = _mm_xor_si128(cipher_block, dec_ks[0]);
  size_t i = 1;
  while (i < Nr) {
    block = _mm_aesdec_si128(block, dec_ks[i++]);
  }
  block = _mm_aesdeclast_si128(block, dec_ks[i]);

  return block;
}

/** @} */

/* AES-128 functions */
static inline __m128i aes128_keyexp_round_core(__m128i key,
                                               __m128i keygenassist_output) {
  keygenassist_output =
      _mm_shuffle_epi32(keygenassist_output, _MM_SHUFFLE(3, 3, 3, 3));
  key = _mm_xor_si128(key, _mm_slli_si128(key, 4));
  key = _mm_xor_si128(key, _mm_slli_si128(key, 4));
  key = _mm_xor_si128(key, _mm_slli_si128(key, 4));

  return _mm_xor_si128(key, keygenassist_output);
}

#define aes128_keyexp_round(key, rcon)                                         \
  aes128_keyexp_round_core((key), _mm_aeskeygenassist_si128((key), (rcon)))

static inline void aes128_expand_key(__m128i enc_key_schedule[11],
                                     __m128i key) {
  /*
   * Process of generation of encryption round keys
   *
   * Let a = W_3 + W_2 + W_1 + W_0
   * For any 2 numbers A & B, A ^ B = B ^ A & A ^ 0 = A
   * Then:
   *   a            = W_3            W_2            W_1            W_0
   *   a << (4 * 4) = W_2            W_1            W_0            00
   *   ---------------------------------------------------------------
   *   b            = (W_2 ^ W_3)    (W_1 ^ W_2)    (W_0 ^ W_1)    W_0
   *   b << (4 * 4) = (W_1 ^ W_2)    (W_0 ^ W_1)    W_0            00
   *   ---------------------------------------------------------------
   *   c            = (W_1 ^ W_3)    (W_0 ^ W_2)    W_1            W_0
   *   c << (4 * 4) = (W_0 ^ W_2)    W_1            W_0            00
   *   ---------------------------------------------------------------
   *                  (W_0 ^ W_1 ^   (W_0 ^ W_1 ^   (W_0 ^ W_1)    W_0
   *                   W_2 ^ W_3)     W_2)
   *                  ------------   ------------   -----------    ---
   *                  W_7            W_6            W_5            W_4
   *
   * Now, XOR it with a number with all 32-bit words set to
   * SubWord(RotWord(W_0)). Then, W_7, W_6, W_5, W_4 are the 32-bit words of
   * next 128-bit round key.
   */
  /* Generate encryption round keys */
  enc_key_schedule[0] = key;
  enc_key_schedule[1] = aes128_keyexp_round(enc_key_schedule[0], 0x01);
  enc_key_schedule[2] = aes128_keyexp_round(enc_key_schedule[1], 0x02);
  enc_key_schedule[3] = aes128_keyexp_round(enc_key_schedule[2], 0x04);
  enc_key_schedule[4] = aes128_keyexp_round(enc_key_schedule[3], 0x08);
  enc_key_schedule[5] = aes128_keyexp_round(enc_key_schedule[4], 0x10);
  enc_key_schedule[6] = aes128_keyexp_round(enc_key_schedule[5], 0x20);
  enc_key_schedule[7] = aes128_keyexp_round(enc_key_schedule[6], 0x40);
  enc_key_schedule[8] = aes128_keyexp_round(enc_key_schedule[7], 0x80);
  enc_key_schedule[9] = aes128_keyexp_round(enc_key_schedule[8], 0x1B);
  enc_key_schedule[10] = aes128_keyexp_round(enc_key_schedule[9], 0x36);
}

/*
 * AES-192 functions
 */

#define shufpd_to_m128i(a, b, imm8)                                            \
  _mm_castpd_si128(_mm_shuffle_pd((__m128d)(a), (__m128d)(b), (imm8)))

static inline void KEY_192_ASSIST(__m128i *temp1, __m128i temp2,
                                  __m128i *temp3) {
  __m128i temp4;
  temp2 = _mm_shuffle_epi32(temp2, 0x55);

  __m128i tmp1 = _mm_load_si128(temp1);
  temp4 = _mm_slli_si128(tmp1, 0x4);
  tmp1 = _mm_xor_si128(tmp1, temp4);
  temp4 = _mm_slli_si128(temp4, 0x4);
  tmp1 = _mm_xor_si128(tmp1, temp4);
  temp4 = _mm_slli_si128(temp4, 0x4);
  tmp1 = _mm_xor_si128(tmp1, temp4);
  tmp1 = _mm_xor_si128(tmp1, temp2);
  temp2 = _mm_shuffle_epi32(tmp1, 0xff);
  *temp1 = tmp1;

  __m128i tmp3 = _mm_load_si128(temp3);
  temp4 = _mm_slli_si128(tmp3, 0x4);
  tmp3 = _mm_xor_si128(tmp3, temp4);
  tmp3 = _mm_xor_si128(tmp3, temp2);
  *temp3 = tmp3;
}

#define aes192_keyexp_assist(temp1, temp3, rcon)                               \
  KEY_192_ASSIST((temp1), _mm_aeskeygenassist_si128(*(temp3), (rcon)), (temp3))

static inline void AES_192_Key_Expansion(const __m128i userkey[2],
                                         __m128i Key_Schedule[13]) {
  __m128i temp1, temp3;

  temp1 = userkey[0];
  temp3 = userkey[1];

  Key_Schedule[0] = temp1;
  Key_Schedule[1] = temp3;
  aes192_keyexp_assist(&temp1, &temp3, 1);
  Key_Schedule[1] = shufpd_to_m128i(Key_Schedule[1], temp1, 0);
  Key_Schedule[2] = shufpd_to_m128i(temp1, temp3, 1);
  aes192_keyexp_assist(&temp1, &temp3, 2);
  Key_Schedule[3] = temp1;
  Key_Schedule[4] = temp3;
  aes192_keyexp_assist(&temp1, &temp3, 4);
  Key_Schedule[4] = shufpd_to_m128i(Key_Schedule[4], temp1, 0);
  Key_Schedule[5] = shufpd_to_m128i(temp1, temp3, 1);
  aes192_keyexp_assist(&temp1, &temp3, 8);
  Key_Schedule[6] = temp1;
  Key_Schedule[7] = temp3;
  aes192_keyexp_assist(&temp1, &temp3, 0x10);
  Key_Schedule[7] = shufpd_to_m128i(Key_Schedule[7], temp1, 0);
  Key_Schedule[8] = shufpd_to_m128i(temp1, temp3, 1);

  aes192_keyexp_assist(&temp1, &temp3, 0x20);
  Key_Schedule[9] = temp1;
  Key_Schedule[10] = temp3;
  aes192_keyexp_assist(&temp1, &temp3, 0x40);
  Key_Schedule[10] = shufpd_to_m128i(Key_Schedule[10], temp1, 0);
  Key_Schedule[11] = shufpd_to_m128i(temp1, temp3, 1);
  aes192_keyexp_assist(&temp1, &temp3, 0x80);
  Key_Schedule[12] = temp1;
}

/*
 * AES-256 functions
 */

#define aes256_key_expansion_round(round_key0, round_key1, key0, key1, rcon)   \
  aes256_key_expansion_round_core((round_key0), (round_key1), (key0), (key1),  \
                                  _mm_aeskeygenassist_si128((key0), (rcon)))

static inline void
aes256_key_expansion_round_core(__m128i *round_key0, __m128i *round_key1,
                                __m128i key0, __m128i key1,
                                __m128i keygenassist_lower_output) {
  __m128i key_lower = key1;

  __m128i tmp1 =
      _mm_shuffle_epi32(keygenassist_lower_output, _MM_SHUFFLE(3, 3, 3, 3));
  for (size_t i = 0; i < 3; ++i) {
    key_lower = _mm_xor_si128(key_lower, _mm_slli_si128(key_lower, 4));
  }
  key_lower = _mm_xor_si128(key_lower, tmp1);

  if (round_key1 != NULL) {
    __m128i key_upper = key0;
    for (size_t i = 0; i < 3; ++i) {
      key_upper = _mm_xor_si128(key_upper, _mm_slli_si128(key_upper, 4));
    }

    __m128i tmp2 = _mm_aeskeygenassist_si128(key_lower, 0);
    tmp2 = _mm_shuffle_epi32(tmp2, _MM_SHUFFLE(2, 2, 2, 2));
    key_upper = _mm_xor_si128(key_upper, tmp2);

    *round_key1 = key_upper;
  }
  *round_key0 = key_lower;
}

static inline void aes256_expand_key(__m128i *enc_key_schedule,
                                     __m128i key[2]) {
  enc_key_schedule[0] = key[0];
  enc_key_schedule[1] = key[1];
  aes256_key_expansion_round(&enc_key_schedule[2], &enc_key_schedule[3],
                             enc_key_schedule[1], enc_key_schedule[0], 1);
  aes256_key_expansion_round(&enc_key_schedule[4], &enc_key_schedule[5],
                             enc_key_schedule[3], enc_key_schedule[2], 2);
  aes256_key_expansion_round(&enc_key_schedule[6], &enc_key_schedule[7],
                             enc_key_schedule[5], enc_key_schedule[4], 4);
  aes256_key_expansion_round(&enc_key_schedule[8], &enc_key_schedule[9],
                             enc_key_schedule[7], enc_key_schedule[6], 8);
  aes256_key_expansion_round(&enc_key_schedule[10], &enc_key_schedule[11],
                             enc_key_schedule[9], enc_key_schedule[8], 0x10);
  aes256_key_expansion_round(&enc_key_schedule[12], &enc_key_schedule[13],
                             enc_key_schedule[11], enc_key_schedule[10], 0x20);
  aes256_key_expansion_round(&enc_key_schedule[14], NULL, enc_key_schedule[13],
                             enc_key_schedule[12], 0x40);
}

static unsigned char key_size_to_nr(unsigned short key_size) {
  switch (key_size) {
  case 128:
    return 10;
  case 192:
    return 12;
  case 256:
    return 14;
  }

  return 0;
}

void aesni_init(AesContext *ctx, enum AesKeyType key_size,
                const unsigned char *key) {
  ctx->key_size = key_size;
  ctx->Nr = key_size_to_nr(key_size);

  __m128i key_v[2];

  switch (key_size) {
  case 128:
    /* Load AES-128 key. */
    key_v[0] = _mm_loadu_si128((const __m128i *)key);

    aes128_expand_key((__m128i *)ctx->enc_round_keys, key_v[0]);
    break;
  case 192:
    /* Load AES-192 key. */
    key_v[0] = _mm_loadu_si128((__m128i *)key);
    key_v[1] = _mm_set_epi32(0, 0, 0, 0);
    memcpy(&key_v[1], key + 16, 8);

    AES_192_Key_Expansion(key_v, (__m128i *)ctx->enc_round_keys);
    break;
  case 256:
    /* Load AES-256 key. */
    key_v[0] = _mm_loadu_si128((const __m128i *)key);
    key_v[1] = _mm_loadu_si128((const __m128i *)key + 1);

    aes256_expand_key((__m128i *)ctx->enc_round_keys, key_v);
    break;
  default:
    break;
  }

  aes_decryption_key_expansion((__m128i *)ctx->dec_round_keys,
                               (__m128i *)ctx->enc_round_keys, key_v[0],
                               ctx->Nr);
}

void aesni_ecb_decrypt(AesContext *ctx, size_t textsize,
                       unsigned char *plain_text,
                       const unsigned char *cipher_text) {
  for (size_t i = 0; i < textsize / 16; ++i) {
    __m128i cipher_block = _mm_loadu_si128(&((const __m128i *)cipher_text)[i]);
    __m128i block = aes_decrypt_block(ctx->Nr, (__m128i *)ctx->dec_round_keys,
                                      cipher_block);
    _mm_storeu_si128(&((__m128i *)plain_text)[i], block);
  }
}

void aesni_ecb_encrypt(AesContext *ctx, size_t textsize,
                       unsigned char *cipher_text,
                       const unsigned char *plain_text) {
  for (size_t i = 0; i < textsize / 16; ++i) {
    __m128i plain_block = _mm_loadu_si128(&((const __m128i *)plain_text)[i]);
    __m128i block =
        aes_encrypt_block(ctx->Nr, (__m128i *)ctx->enc_round_keys, plain_block);
    _mm_storeu_si128(&((__m128i *)cipher_text)[i], block);
  }
}

void aesni_cbc_encrypt(AesContext *ctx, size_t textsize,
                       unsigned char *cipher_text,
                       const unsigned char *plain_text,
                       const unsigned char iv[16]) {
  __m128i previous_block = _mm_loadu_si128((const __m128i *)iv);

  for (size_t i = 0; i < textsize / 16; ++i) {
    __m128i plain_block = _mm_loadu_si128(&((const __m128i *)plain_text)[i]);
    __m128i block = _mm_xor_si128(plain_block, previous_block);
    block = aes_encrypt_block(ctx->Nr, (__m128i *)ctx->enc_round_keys, block);

    _mm_storeu_si128(&((__m128i *)cipher_text)[i], block);
    previous_block = block;
  }
}

void aesni_cbc_decrypt(AesContext *ctx, size_t textsize,
                       unsigned char *plain_text,
                       const unsigned char *cipher_text,
                       const unsigned char iv[16]) {
  __m128i previous_block = _mm_loadu_si128((const __m128i *)iv);

  for (size_t i = 0; i < textsize / 16; ++i) {
    __m128i cipher_block = _mm_loadu_si128(&((const __m128i *)cipher_text)[i]);
    __m128i block = aes_decrypt_block(ctx->Nr, (__m128i *)ctx->dec_round_keys,
                                      cipher_block);
    block = _mm_xor_si128(block, previous_block);

    _mm_storeu_si128(&((__m128i *)plain_text)[i], block);
    previous_block = cipher_block;
  }
}

/* Behaves exactly like _mm_cmplt_epi8 except doing unsigned comparisons.
 * From https://stackoverflow.com/a/56346628/15519945
 */
static inline __m128i cmplt_epi8_unsigned(__m128i a, __m128i b) {
  __m128i signbits = _mm_set1_epi8(0x80);
  a = _mm_xor_si128(a, signbits);
  b = _mm_xor_si128(b, signbits);

  return _mm_cmplt_epi8(a, b);
}

static __m128i m128i_add(__m128i a, __m128i b) {
  __m128i sum_packed8 = _mm_add_epi8(a, b);
  __m128i mask = cmplt_epi8_unsigned(sum_packed8, a);
  __m128i ones = _mm_set1_epi8(1);

  /* Get carry of each of 8-bit packed addition */
  __m128i carry = _mm_and_si128(mask, ones);

  carry = _mm_slli_si128(carry, 1);

  __m128i final_sum = _mm_add_epi8(sum_packed8, carry);

  return final_sum;
}

static inline __m128i m128i_increment(__m128i a) {
  __m128i one = _mm_set_epi32(0, 0, 0, 1);
  return m128i_add(a, one);
}

static inline __m128i m128i_bswap(__m128i x) {
  const __m128i reverse_order =
      _mm_set_epi32(0x00010203, 0x04050607, 0x08090a0b, 0x0c0d0e0f);
  return _mm_shuffle_epi8(x, reverse_order);
}

void aesni_ctr_xcrypt(AesContext *ctx, size_t textsize, unsigned char *out,
                      const unsigned char *in, unsigned char next_iv[16],
                      const unsigned char iv[16]) {
  __m128i iv_reg = m128i_bswap(_mm_loadu_si128((const __m128i *)iv));
  size_t i = 0;

  for (; i < textsize / 16; ++i) {
    __m128i stream_block = aes_encrypt_block(
        ctx->Nr, (__m128i *)ctx->enc_round_keys, m128i_bswap(iv_reg));
    __m128i in_block = _mm_loadu_si128((const __m128i *)&in[i * 16]);
    __m128i out_block = _mm_xor_si128(stream_block, in_block);

    iv_reg = m128i_increment(iv_reg);

    _mm_storeu_si128((__m128i *)&out[i * 16], out_block);
  }

  if (textsize % 16) {
    __m128i stream_block = aes_encrypt_block(
        ctx->Nr, (__m128i *)ctx->enc_round_keys, m128i_bswap(iv_reg));
    __m128i in_block = _mm_setzero_si128();
    memcpy(&in_block, &in[i * 16], textsize % 16);

    __m128i out_block = _mm_xor_si128(stream_block, in_block);
    memcpy(&out[i * 16], &out_block, textsize % 16);
  }

  if (next_iv) {
    _mm_storeu_si128((__m128i *)next_iv, m128i_bswap(iv_reg));
  }
}
