#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aes-bs.h"
#include <ay/aes.h>

#if CHAR_BIT != 8
#error This code requires CHAR_BIT to be 8.
#endif

/** @brief Number of rounds in AES variants */
enum AesBsNr {
  NONE_BS_NR = 0,

  /** @brief Number of rounds in AES-128 */
  AES128_BS_NR = 10,

  /** @brief Number of rounds in AES-192 */
  AES192_BS_NR = 12,

  /** @brief Number of rounds in AES-256 */
  AES256_BS_NR = 14
};

static struct AesBsState aesbs_AddRoundKey(struct AesBsState state,
                                           struct AesBsState round_key) {
  struct AesBsState result;
  for (size_t i = 0; i < CHAR_BIT; ++i) {
    result.slice[i] = state.slice[i] ^ round_key.slice[i];
  }

  return result;
}

static void store_byte_to_bitslice(struct AesBsState *dest, unsigned char byte,
                                   size_t row, size_t column) {
  for (size_t i = 0; i < CHAR_BIT; ++i) {
    dest->slice[i] |= (uint16_t)(byte & 1) << (row * 4 + column);
    byte >>= 1;
  }
}

static struct AesBsState store_bytes_to_bitslice(const unsigned char src[16]) {
  struct AesBsState result;
  memset(&result, 0, sizeof result);
  for (size_t column = 0; column < 4; ++column) {
    for (size_t row = 0; row < 4; ++row) {
      store_byte_to_bitslice(&result, *(src++), row, column);
    }
  }

  return result;
}

static void save_bitslice_to_bytes(unsigned char dest[16],
                                   struct AesBsState src) {
  for (size_t column = 0; column < 4; ++column) {
    for (size_t row = 0; row < 4; ++row) {
      unsigned char byte = 0;
      for (size_t i = 0; i < CHAR_BIT; ++i) {
        byte |= ((src.slice[i] >> (row * 4 + column)) & 1) << i;
      }
      *(dest++) = byte;
    }
  }
}

static void aesbs_SubBytes_core(struct AesBsState *dest_state,
                                const struct AesBsState *state,
                                bool needs_inverse) {
  uint16_t U0 = state->slice[7];
  uint16_t U1 = state->slice[6];
  uint16_t U2 = state->slice[5];
  uint16_t U3 = state->slice[4];
  uint16_t U4 = state->slice[3];
  uint16_t U5 = state->slice[2];
  uint16_t U6 = state->slice[1];
  uint16_t U7 = state->slice[0];

  uint16_t T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15,
      T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27;
  uint16_t D;

  if (!needs_inverse) {
    T1 = U0 ^ U3;
    T2 = U0 ^ U5;
    T3 = U0 ^ U6;
    T4 = U3 ^ U5;
    T5 = U4 ^ U6;
    T6 = T1 ^ T5;
    T7 = U1 ^ U2;

    T8 = U7 ^ T6;
    T9 = U7 ^ T7;
    T10 = T6 ^ T7;
    T11 = U1 ^ U5;
    T12 = U2 ^ U5;
    T13 = T3 ^ T4;
    T14 = T6 ^ T11;

    T15 = T5 ^ T11;
    T16 = T5 ^ T12;
    T17 = T9 ^ T16;
    T18 = U3 ^ U7;
    T19 = T7 ^ T18;
    T20 = T1 ^ T19;
    T21 = U6 ^ U7;

    T22 = T7 ^ T21;
    T23 = T2 ^ T22;
    T24 = T2 ^ T10;
    T25 = T20 ^ T17;
    T26 = T3 ^ T16;
    T27 = T1 ^ T12;

    D = U7;
  } else {
    T23 = U0 ^ U3;
    T22 = ~(U1 ^ U3);
    T2 = ~(U0 ^ U1);
    T1 = U3 ^ U4;
    T24 = ~(U4 ^ U7);
    uint16_t R5 = U6 ^ U7;
    T8 = ~(U1 ^ T23);

    T19 = T22 ^ R5;
    T9 = ~(U7 ^ T1);
    T10 = T2 ^ T24;
    T13 = T2 ^ R5;
    T3 = T1 ^ R5;
    T25 = ~(U2 ^ T1);
    uint16_t R13 = U1 ^ U6;

    T17 = ~(U2 ^ T19);
    T20 = T24 ^ R13;
    T4 = U4 ^ T8;
    uint16_t R17 = ~(U2 ^ U5);
    uint16_t R18 = ~(U5 ^ U6);
    uint16_t R19 = ~(U2 ^ U4);
    uint16_t Y5 = U0 ^ R17;

    T6 = T22 ^ R17;
    T16 = R13 ^ R19;
    T27 = T1 ^ R18;
    T15 = T10 ^ T27;
    T14 = T10 ^ R18;
    T26 = T3 ^ T16;

    D = Y5;
  }

  uint16_t M1 = T13 & T6;
  uint16_t M2 = T23 & T8;
  uint16_t M3 = T14 ^ M1;
  uint16_t M4 = T19 & D;
  uint16_t M5 = M4 ^ M1;
  uint16_t M6 = T3 & T16;
  uint16_t M7 = T22 & T9;
  uint16_t M8 = T26 ^ M6;
  uint16_t M9 = T20 & T17;
  uint16_t M10 = M9 ^ M6;
  uint16_t M11 = T1 & T15;
  uint16_t M12 = T4 & T27;
  uint16_t M13 = M12 ^ M11;
  uint16_t M14 = T2 & T10;
  uint16_t M15 = M14 ^ M11;
  uint16_t M16 = M3 ^ M2;

  uint16_t M17 = M5 ^ T24;
  uint16_t M18 = M8 ^ M7;
  uint16_t M19 = M10 ^ M15;
  uint16_t M20 = M16 ^ M13;
  uint16_t M21 = M17 ^ M15;
  uint16_t M22 = M18 ^ M13;
  uint16_t M23 = M19 ^ T25;
  uint16_t M24 = M22 ^ M23;
  uint16_t M25 = M22 & M20;
  uint16_t M26 = M21 ^ M25;
  uint16_t M27 = M20 ^ M21;
  uint16_t M28 = M23 ^ M25;
  uint16_t M29 = M28 & M27;
  uint16_t M30 = M26 & M24;
  uint16_t M31 = M20 & M23;
  uint16_t M32 = M27 & M31;

  uint16_t M33 = M27 ^ M25;
  uint16_t M34 = M21 & M22;
  uint16_t M35 = M24 & M34;
  uint16_t M36 = M24 ^ M25;
  uint16_t M37 = M21 ^ M29;
  uint16_t M38 = M32 ^ M33;
  uint16_t M39 = M23 ^ M30;
  uint16_t M40 = M35 ^ M36;
  uint16_t M41 = M38 ^ M40;
  uint16_t M42 = M37 ^ M39;
  uint16_t M43 = M37 ^ M38;
  uint16_t M44 = M39 ^ M40;
  uint16_t M45 = M42 ^ M41;
  uint16_t M46 = M44 & T6;
  uint16_t M47 = M40 & T8;
  uint16_t M48 = M39 & D;

  uint16_t M49 = M43 & T16;
  uint16_t M50 = M38 & T9;
  uint16_t M51 = M37 & T17;
  uint16_t M52 = M42 & T15;
  uint16_t M53 = M45 & T27;
  uint16_t M54 = M41 & T10;
  uint16_t M55 = M44 & T13;
  uint16_t M56 = M40 & T23;
  uint16_t M57 = M39 & T19;
  uint16_t M58 = M43 & T3;
  uint16_t M59 = M38 & T22;
  uint16_t M60 = M37 & T20;
  uint16_t M61 = M42 & T1;
  uint16_t M62 = M45 & T4;
  uint16_t M63 = M41 & T2;

  if (!needs_inverse) {
    uint16_t L0 = M61 ^ M62;
    uint16_t L1 = M50 ^ M56;
    uint16_t L2 = M46 ^ M48;
    uint16_t L3 = M47 ^ M55;
    uint16_t L4 = M54 ^ M58;
    uint16_t L5 = M49 ^ M61;
    uint16_t L6 = M62 ^ L5;
    uint16_t L7 = M46 ^ L3;
    uint16_t L8 = M51 ^ M59;
    uint16_t L9 = M52 ^ M53;

    uint16_t L10 = M53 ^ L4;
    uint16_t L11 = M60 ^ L2;
    uint16_t L12 = M48 ^ M51;
    uint16_t L13 = M50 ^ L0;
    uint16_t L14 = M52 ^ M61;
    uint16_t L15 = M55 ^ L1;
    uint16_t L16 = M56 ^ L0;
    uint16_t L17 = M57 ^ L1;
    uint16_t L18 = M58 ^ L8;
    uint16_t L19 = M63 ^ L4;

    uint16_t L20 = L0 ^ L1;
    uint16_t L21 = L1 ^ L7;
    uint16_t L22 = L3 ^ L12;
    uint16_t L23 = L18 ^ L2;
    uint16_t L24 = L15 ^ L9;
    uint16_t L25 = L6 ^ L10;
    uint16_t L26 = L7 ^ L9;
    uint16_t L27 = L8 ^ L10;
    uint16_t L28 = L11 ^ L14;
    uint16_t L29 = L11 ^ L17;

    uint16_t S0 = L6 ^ L24;
    uint16_t S1 = ~(L16 ^ L26);
    uint16_t S2 = ~(L19 ^ L28);
    uint16_t S3 = L6 ^ L21;
    uint16_t S4 = L20 ^ L22;
    uint16_t S5 = L25 ^ L29;
    uint16_t S6 = ~(L13 ^ L27);
    uint16_t S7 = ~(L6 ^ L23);

    dest_state->slice[7] = S0;
    dest_state->slice[6] = S1;
    dest_state->slice[5] = S2;
    dest_state->slice[4] = S3;
    dest_state->slice[3] = S4;
    dest_state->slice[2] = S5;
    dest_state->slice[1] = S6;
    dest_state->slice[0] = S7;
  } else {
    uint16_t P0 = M52 ^ M61;
    uint16_t P1 = M58 ^ M59;
    uint16_t P2 = M54 ^ M62;
    uint16_t P3 = M47 ^ M50;
    uint16_t P4 = M48 ^ M56;
    uint16_t P5 = M46 ^ M51;
    uint16_t P6 = M49 ^ M60;
    uint16_t P7 = P0 ^ P1;
    uint16_t P8 = M50 ^ M53;
    uint16_t P9 = M55 ^ M63;

    uint16_t P10 = M57 ^ P4;
    uint16_t P11 = P0 ^ P3;
    uint16_t P12 = M46 ^ M48;
    uint16_t P13 = M49 ^ M51;
    uint16_t P14 = M49 ^ M62;
    uint16_t P15 = M54 ^ M59;
    uint16_t P16 = M57 ^ M61;
    uint16_t P17 = M58 ^ P2;
    uint16_t P18 = M63 ^ P5;
    uint16_t P19 = P2 ^ P3;

    uint16_t P20 = P4 ^ P6;
    uint16_t P22 = P2 ^ P7;
    uint16_t P23 = P7 ^ P8;
    uint16_t P24 = P5 ^ P7;
    uint16_t P25 = P6 ^ P10;
    uint16_t P26 = P9 ^ P11;
    uint16_t P27 = P10 ^ P18;
    uint16_t P28 = P11 ^ P25;
    uint16_t P29 = P15 ^ P20;
    uint16_t W0 = P13 ^ P22;

    uint16_t W1 = P26 ^ P29;
    uint16_t W2 = P17 ^ P28;
    uint16_t W3 = P12 ^ P22;
    uint16_t W4 = P23 ^ P27;
    uint16_t W5 = P19 ^ P24;
    uint16_t W6 = P14 ^ P23;
    uint16_t W7 = P9 ^ P16;

    dest_state->slice[7] = W0;
    dest_state->slice[6] = W1;
    dest_state->slice[5] = W2;
    dest_state->slice[4] = W3;
    dest_state->slice[3] = W4;
    dest_state->slice[2] = W5;
    dest_state->slice[1] = W6;
    dest_state->slice[0] = W7;
  }
}

static inline void aesbs_SubBytes(struct AesBsState *dest_state,
                                  struct AesBsState *state) {
  aesbs_SubBytes_core(dest_state, state, false);
}

static inline void aesbs_InvSubBytes(struct AesBsState *dest_state,
                                     struct AesBsState *state) {
  aesbs_SubBytes_core(dest_state, state, true);
}

// [begin, end)
static inline uint16_t extract_bits_u16(uint16_t value, size_t begin,
                                        size_t end) {
  uint16_t mask = (1 << (end - begin)) - 1;
  return (value >> begin) & mask;
}

static inline uint16_t rotl_4bit_u16(uint16_t n, unsigned char c) {
  return (n >> c) | ((n << (4 - c)) & 0xf);
}

static inline uint16_t rotr_4bit_u16(uint16_t n, unsigned char c) {
  return ((n << c) & 0xf) | (n >> (4 - c));
}

static struct AesBsState aesbs_ShiftRows(struct AesBsState state) {
  struct AesBsState result;

  for (size_t i = 0; i < CHAR_BIT; ++i) {
    uint16_t b = state.slice[i];
    result.slice[i] = extract_bits_u16(b, 0, 4);
    result.slice[i] |= rotr_4bit_u16(extract_bits_u16(b, 4, 8), 3) << 4;
    result.slice[i] |= rotr_4bit_u16(extract_bits_u16(b, 8, 12), 2) << 8;
    result.slice[i] |= rotr_4bit_u16(extract_bits_u16(b, 12, 16), 1) << 12;
  }

  return result;
}

static struct AesBsState aesbs_InvShiftRows(struct AesBsState state) {
  struct AesBsState result;

  for (size_t i = 0; i < CHAR_BIT; ++i) {
    uint16_t b = state.slice[i];
    result.slice[i] = extract_bits_u16(b, 0, 4);
    result.slice[i] |= rotl_4bit_u16(extract_bits_u16(b, 4, 8), 3) << 4;
    result.slice[i] |= rotl_4bit_u16(extract_bits_u16(b, 8, 12), 2) << 8;
    result.slice[i] |= rotl_4bit_u16(extract_bits_u16(b, 12, 16), 1) << 12;
  }

  return result;
}

#define generic_mask(n) (CHAR_BIT * sizeof(n) - 1)
#define generic_negate(n) (-(n))
#define generic_rotl(n, c)                                                     \
  ((n) << ((c)&generic_mask(n)) |                                              \
   ((n) >> (generic_negate(c) & generic_mask(n))))
#define generic_rotr(n, c)                                                     \
  ((n) >> ((c)&generic_mask(n)) |                                              \
   ((n) << (generic_negate(c) & generic_mask(n))))

static inline uint16_t rotr16(uint16_t n, unsigned char c) {
  return generic_rotr(n, c);
}
static inline uint16_t rotl16(uint16_t n, unsigned char c) {
  return generic_rotl(n, c);
}

static struct AesBsState aes__xor_state(struct AesBsState lhs,
                                        struct AesBsState rhs) {
  struct AesBsState result;
  for (size_t i = 0; i < CHAR_BIT; ++i) {
    result.slice[i] = lhs.slice[i] ^ rhs.slice[i];
  }
  return result;
}

static struct AesBsState aesbs_RotWord(struct AesBsState state) {
  struct AesBsState result;
  for (size_t i = 0; i < CHAR_BIT; ++i) {
    result.slice[i] = rotl16(state.slice[i], 12);
  }
  return result;
}

static struct AesBsState aes__get_column(struct AesBsState src, size_t column) {
  struct AesBsState result;
  for (size_t i = 0; i < CHAR_BIT; ++i) {
    result.slice[i] = (src.slice[i] >> column) & 0x1111;
  }

  return result;
}

static struct AesBsState
aes__key_setup_round_core(struct AesBsState *round_key,
                          size_t num_resultant_col, struct AesBsState first_col,
                          struct AesBsState last_expanded_key,
                          size_t num_last_expanded_key) {
  struct AesBsState resultant_col;
  for (size_t i = 0; i < CHAR_BIT; ++i) {
    resultant_col.slice[i] =
        first_col.slice[i] ^
        ((last_expanded_key.slice[i] >> num_last_expanded_key) & 0x1111);
    round_key->slice[i] |= (resultant_col.slice[i] & 0x1111)
                           << num_resultant_col;
  }

  return resultant_col;
}

static struct AesBsState aes__multiply_by_x(struct AesBsState state) {
  struct AesBsState result;
  uint16_t orig_a_7 = state.slice[7];
  result.slice[7] = state.slice[6];
  result.slice[6] = state.slice[5];
  result.slice[5] = state.slice[4];
  result.slice[4] = state.slice[3] ^ orig_a_7;
  result.slice[3] = state.slice[2] ^ orig_a_7;
  result.slice[2] = state.slice[1];
  result.slice[1] = state.slice[0] ^ orig_a_7;
  result.slice[0] = orig_a_7;

  return result;
}

static void aes__key_schedule(struct AesBsState *round_keys,
                              const unsigned char *key, size_t Nk,
                              enum AesBsNr Nr) {
  /* Clear garbage values from round_keys */
  for (size_t i = 0; i < (size_t)Nr + 1; ++i) {
    for (size_t j = 0; j < CHAR_BIT; ++j) {
      round_keys[i].slice[j] = 0;
    }
  }

  /* Put contents of key into round_keys first. */
  for (size_t i = 0; i < (size_t)Nk; ++i) {
    for (size_t j = 0; j < 4; ++j) {
      store_byte_to_bitslice(&round_keys[i / 4], *(key++), j, i % 4);
    }
  }

  struct AesBsState rcon = {{1, 0, 0, 0, 0, 0, 0, 0}};
  struct AesBsState first_col =
      aes__get_column(round_keys[(Nk - 1) / 4], (Nk - 1) % 4);

  for (size_t i = Nk, pos = 0; i < 4 * ((size_t)Nr + 1); ++i, ++pos) {
    if (pos % Nk == 0) {
      first_col = aesbs_RotWord(first_col);
      aesbs_SubBytes(&first_col, &first_col);
      first_col = aes__xor_state(first_col, rcon);

      rcon = aes__multiply_by_x(rcon);
    } else if (Nk > 6 && pos % Nk == 4) {
      aesbs_SubBytes(&first_col, &first_col);
    }
    first_col =
        aes__key_setup_round_core(&round_keys[i / 4], i % 4, first_col,
                                  round_keys[(i - Nk) / 4], (i - Nk) % 4);
  }
}

static struct AesBsState aesbs_MixColumns(struct AesBsState src) {
  struct AesBsState result;

  uint16_t a0 = src.slice[0], a1 = src.slice[1], a2 = src.slice[2],
           a3 = src.slice[3], a4 = src.slice[4], a5 = src.slice[5],
           a6 = src.slice[6], a7 = src.slice[7];

  result.slice[0] =
      (a7 ^ rotr16(a7, 4)) ^ rotr16(a0, 4) ^ rotr16(a0 ^ rotr16(a0, 4), 8);
  result.slice[1] = (a0 ^ rotr16(a0, 4)) ^ (a7 ^ rotr16(a7, 4)) ^
                    rotr16(a1, 4) ^ rotr16(a1 ^ rotr16(a1, 4), 8);
  result.slice[2] =
      (a1 ^ rotr16(a1, 4)) ^ rotr16(a2, 4) ^ rotr16(a2 ^ rotr16(a2, 4), 8);
  result.slice[3] = (a2 ^ rotr16(a2, 4)) ^ (a7 ^ rotr16(a7, 4)) ^
                    rotr16(a3, 4) ^ rotr16(a3 ^ rotr16(a3, 4), 8);
  result.slice[4] = (a3 ^ rotr16(a3, 4)) ^ (a7 ^ rotr16(a7, 4)) ^
                    rotr16(a4, 4) ^ rotr16(a4 ^ rotr16(a4, 4), 8);
  result.slice[5] =
      (a4 ^ rotr16(a4, 4)) ^ rotr16(a5, 4) ^ rotr16(a5 ^ rotr16(a5, 4), 8);
  result.slice[6] =
      (a5 ^ rotr16(a5, 4)) ^ rotr16(a6, 4) ^ rotr16(a6 ^ rotr16(a6, 4), 8);
  result.slice[7] =
      (a6 ^ rotr16(a6, 4)) ^ rotr16(a7, 4) ^ rotr16(a7 ^ rotr16(a7, 4), 8);

  return result;
}

static struct AesBsState aesbs_InvMixColumns(struct AesBsState s) {
  struct AesBsState result = aesbs_MixColumns(s);
  uint16_t r0_rotr_r0 = result.slice[0] ^ rotr16(result.slice[0], 8);
  uint16_t r1_rotr_r1 = result.slice[1] ^ rotr16(result.slice[1], 8);
  uint16_t r2_rotr_r2 = result.slice[2] ^ rotr16(result.slice[2], 8);
  uint16_t r3_rotr_r3 = result.slice[3] ^ rotr16(result.slice[3], 8);
  uint16_t r4_rotr_r4 = result.slice[4] ^ rotr16(result.slice[4], 8);
  uint16_t r5_rotr_r5 = result.slice[5] ^ rotr16(result.slice[5], 8);
  uint16_t r6_rotr_r6 = result.slice[6] ^ rotr16(result.slice[6], 8);
  uint16_t r7_rotr_r7 = result.slice[7] ^ rotr16(result.slice[7], 8);
  /* And then update s += {04} * t?_02 */
  result.slice[0] ^= r6_rotr_r6;
  result.slice[1] ^= r6_rotr_r6 ^ r7_rotr_r7;
  result.slice[2] ^= r0_rotr_r0 ^ r7_rotr_r7;
  result.slice[3] ^= r1_rotr_r1 ^ r6_rotr_r6;
  result.slice[4] ^= r2_rotr_r2 ^ r6_rotr_r6 ^ r7_rotr_r7;
  result.slice[5] ^= r3_rotr_r3 ^ r7_rotr_r7;
  result.slice[6] ^= r4_rotr_r4;
  result.slice[7] ^= r5_rotr_r5;
  return result;
}

static struct AesBsState aesbs_enc_block(enum AesBsNr Nr,
                                         struct AesBsState *round_keys,
                                         struct AesBsState plain_text) {
  struct AesBsState block = aesbs_AddRoundKey(plain_text, round_keys[0]);

  size_t round = 1;
  while (round < Nr) {
    aesbs_SubBytes(&block, &block);
    block = aesbs_ShiftRows(block);
    block = aesbs_MixColumns(block);
    block = aesbs_AddRoundKey(block, round_keys[round++]);
  }

  aesbs_SubBytes(&block, &block);
  block = aesbs_ShiftRows(block);
  block = aesbs_AddRoundKey(block, round_keys[round++]);

  return block;
}

static struct AesBsState aesbs_dec_block(enum AesBsNr Nr,
                                         struct AesBsState *round_keys,
                                         struct AesBsState cipher_text) {
  size_t nr = Nr;
  struct AesBsState block = aesbs_AddRoundKey(cipher_text, round_keys[nr--]);

  for (size_t round = 1; round < Nr; ++round) {
    block = aesbs_InvShiftRows(block);
    aesbs_InvSubBytes(&block, &block);
    block = aesbs_AddRoundKey(block, round_keys[nr--]);
    block = aesbs_InvMixColumns(block);
  }

  block = aesbs_InvShiftRows(block);
  aesbs_InvSubBytes(&block, &block);
  block = aesbs_AddRoundKey(block, round_keys[nr]);

  return block;
}

#if 0
void printf_bitslice(struct AesBsState state, const char *fmt_str, ...) {
  unsigned char dest[16];
  va_list args;
  va_start(args, fmt_str);

  save_bitslice_to_bytes(dest, state);

  if (fmt_str) {
    vprintf(fmt_str, args);
    printf(" = ");
  }
  printf("%02x", dest[0]);
  for (size_t i = 1; i < 16; ++i) {
    printf(" %02x", dest[i]);
  }
  printf("\n");

  va_end(args);
}
#endif

static enum AesBsNr aesbs_key_size_to_nr(size_t key_size) {
  switch (key_size) {
  case 128:
    return AES128_BS_NR;
  case 192:
    return AES192_BS_NR;
  case 256:
    return AES256_BS_NR;
  default:
    return NONE_BS_NR;
  }
}

void aesbs_init(AesContext *ctx, enum AesKeyType key_size,
                const unsigned char *key) {
  ctx->key_size = key_size;
  struct AesBsState round_keys[15];
  aes__key_schedule(round_keys, key, key_size / 32,
                    aesbs_key_size_to_nr(key_size));

  static_assert(sizeof ctx->enc_round_keys == sizeof round_keys, "");
  memcpy(ctx->enc_round_keys, round_keys, sizeof ctx->enc_round_keys);
}

void aesbs_ecb_encrypt(AesContext *ctx, size_t textsize,
                       unsigned char *cipher_text,
                       const unsigned char *plain_text) {
  enum AesBsNr Nr = aesbs_key_size_to_nr(ctx->key_size);
  struct AesBsState round_keys[15];
  memcpy(round_keys, ctx->enc_round_keys, sizeof round_keys);

  for (size_t i = 0; i < textsize / 16; ++i) {
    struct AesBsState block = store_bytes_to_bitslice(&plain_text[i * 16]);
    block = aesbs_enc_block(Nr, round_keys, block);
    save_bitslice_to_bytes(&cipher_text[i * 16], block);
  }
}

void aesbs_ecb_decrypt(AesContext *ctx, size_t textsize,
                       unsigned char *plain_text,
                       const unsigned char *cipher_text) {
  enum AesBsNr Nr = aesbs_key_size_to_nr(ctx->key_size);
  struct AesBsState round_keys[15];
  memcpy(round_keys, ctx->enc_round_keys, sizeof round_keys);

  for (size_t i = 0; i < textsize / 16; ++i) {
    struct AesBsState block = store_bytes_to_bitslice(&cipher_text[i * 16]);
    block = aesbs_dec_block(Nr, round_keys, block);
    save_bitslice_to_bytes(&plain_text[i * 16], block);
  }
}

void aesbs_cbc_encrypt(AesContext *ctx, size_t textsize,
                       unsigned char *cipher_text,
                       const unsigned char *plain_text,
                       const unsigned char iv[16]) {
  enum AesBsNr Nr = aesbs_key_size_to_nr(ctx->key_size);
  struct AesBsState previous_block = store_bytes_to_bitslice(iv);

  struct AesBsState round_keys[15];
  memcpy(round_keys, ctx->enc_round_keys, sizeof round_keys);

  for (size_t i = 0; i < textsize / 16; ++i) {
    struct AesBsState plain_block =
        store_bytes_to_bitslice(&plain_text[i * 16]);
    struct AesBsState block = aes__xor_state(plain_block, previous_block);
    block = aesbs_enc_block(Nr, round_keys, block);

    save_bitslice_to_bytes(&cipher_text[i * 16], block);
    previous_block = block;
  }
}

void aesbs_cbc_decrypt(AesContext *ctx, size_t textsize,
                       unsigned char *plain_text,
                       const unsigned char *cipher_text,
                       const unsigned char iv[16]) {
  enum AesBsNr Nr = aesbs_key_size_to_nr(ctx->key_size);
  struct AesBsState previous_block = store_bytes_to_bitslice(iv);

  struct AesBsState round_keys[15];
  memcpy(round_keys, ctx->enc_round_keys, sizeof round_keys);

  for (size_t i = 0; i < textsize / 16; ++i) {
    struct AesBsState cipher_block =
        store_bytes_to_bitslice(&cipher_text[i * 16]);
    struct AesBsState block = aesbs_dec_block(Nr, round_keys, cipher_block);
    block = aes__xor_state(block, previous_block);

    save_bitslice_to_bytes(&plain_text[i * 16], block);

    previous_block = cipher_block;
  }
}

#if 0
void printf_16b(FILE *f, const unsigned char data[16], const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  vfprintf(f, fmt, ap);

  va_end(ap);

  fprintf(f, "%02x", data[0]);
  for (size_t i = 1; i < 16; ++i) {
    fprintf(f, " %02x", data[i]);
  }
  fprintf(f, "\n");
}
#endif

static struct AesBsState aesbs_add_bitslice(struct AesBsState lhs,
                                            struct AesBsState rhs) {
  struct AesBsState result;
  memset(&result, 0, sizeof result);

  uint16_t carry = 0;
  for (size_t i = 0; i < CHAR_BIT; ++i) {
    result.slice[i] = lhs.slice[i] ^ rhs.slice[i] ^ carry;
    carry = (lhs.slice[i] & rhs.slice[i]) | (lhs.slice[i] & carry) |
            (rhs.slice[i] & carry);
  }

  return result;
}

static struct AesBsState aesbs_increment_bitslice(struct AesBsState a) {
  unsigned char bytes_one[16];
  memset(bytes_one, 0, sizeof bytes_one);
  bytes_one[15] = 1;

  struct AesBsState bitslice_one = store_bytes_to_bitslice(bytes_one);

  return aesbs_add_bitslice(a, bitslice_one);
}

void aesbs_ctr_xcrypt(AesContext *ctx, size_t textsize, unsigned char *out,
                      const unsigned char *in, unsigned char next_iv[16],
                      const unsigned char iv[16]) {
  struct AesBsState iv_bitslice = store_bytes_to_bitslice(iv);
  enum AesBsNr Nr = aesbs_key_size_to_nr(ctx->key_size);

  struct AesBsState round_keys[15];
  memcpy(round_keys, ctx->enc_round_keys, sizeof round_keys);
  size_t i = 0;

  for (; i < textsize / 16; ++i) {
    struct AesBsState stream_block =
        aesbs_enc_block(Nr, round_keys, iv_bitslice);
    struct AesBsState in_block = store_bytes_to_bitslice(&in[i * 16]);
    struct AesBsState out_block = aes__xor_state(stream_block, in_block);

    iv_bitslice = aesbs_increment_bitslice(iv_bitslice);

    save_bitslice_to_bytes(&out[i * 16], out_block);
  }

  if (textsize % 16) {
    struct AesBsState stream_block =
        aesbs_enc_block(Nr, round_keys, iv_bitslice);
    struct AesBsState in_block;
    memset(&in_block, 0, sizeof in_block);
    for (size_t j = 0; j < textsize % 16; ++j) {
      store_byte_to_bitslice(&in_block, in[i * 16 + j], j % 4, j / 4);
    }

    struct AesBsState out_block = aes__xor_state(stream_block, in_block);

    unsigned char out_bytes[16];
    save_bitslice_to_bytes(out_bytes, out_block);
    memcpy(&out[i * 16], out_bytes, textsize % 16);
  }

  save_bitslice_to_bytes(next_iv, iv_bitslice);
}
