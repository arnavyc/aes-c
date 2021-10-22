/**
 * @file ay/aes.h
 * @brief Public header for aes-c
 */
#ifndef AY_AES_H
#define AY_AES_H

#include <ay/aes/hedley.h>
#include <stddef.h>

HEDLEY_BEGIN_C_DECLS

/** @cond */
#if defined(__STDC_VERSION__) && !defined(AY_AES_ALIGNAS)
#if __STDC_VERSION__ >= 201112L
#include <stdalign.h>
#define AY_AES_ALIGNAS(x) alignas(x)
#endif
#endif

#ifndef AY_AES_ALIGNAS

#if defined(_MSC_VER)
#define AY_AES_ALIGNAS(x) __declspec(align(x))
#elif defined(__GNUC__)
#define AY_AES_ALIGNAS(x) __attribute__((aligned(x)))
#endif

#endif

#define SIZE_OF_AES_ROUND_KEY 16
#define NUM_ROUND_KEYS_IN_ARRAY 15
/** @endcond */

/**
 * @brief AES variants that can be used with this library
 */
enum AesKeyType {
  KEY_TYPE_AES128 = 128, /**< For AES-128 */
  KEY_TYPE_AES192 = 192, /**< For AES-192 */
  KEY_TYPE_AES256 = 256  /**< For AES-256 */
};

/**
 * @brief Structure for storing internal information needed by the library
 */
typedef struct AesContext AesContext;

/**
 * @cond
 * **PRIVATE**: Do not use as consumer of library.
 */
struct aes_vtable {
  void (*init)(AesContext *ctx, enum AesKeyType key_type,
               const unsigned char *key);

  void (*ctr_xcrypt)(AesContext *ctx, size_t textsize, unsigned char *out,
                     const unsigned char *in, unsigned char next_iv[16],
                     const unsigned char iv[16]);
  void (*ecb_encrypt)(AesContext *ctx, size_t textsize,
                      unsigned char *cipher_text,
                      const unsigned char *plain_text);
  void (*ecb_decrypt)(AesContext *ctx, size_t textsize,
                      unsigned char *plain_text,
                      const unsigned char *cipher_text);

  void (*cbc_encrypt)(AesContext *ctx, size_t textsize,
                      unsigned char *cipher_text,
                      const unsigned char *plain_text,
                      const unsigned char iv[16]);

  void (*cbc_decrypt)(AesContext *ctx, size_t textsize,
                      unsigned char *plain_text,
                      const unsigned char *cipher_text,
                      const unsigned char iv[16]);
};
/** @endcond */

/** @cond
 * **PRIVATE**: Do not use any private field of this structure
 */
struct AesContext {
  unsigned short key_size;
  unsigned char Nr;
  struct aes_vtable vtable;
  AY_AES_ALIGNAS(16)
  unsigned char enc_round_keys[NUM_ROUND_KEYS_IN_ARRAY * SIZE_OF_AES_ROUND_KEY];
  AY_AES_ALIGNAS(16)
  unsigned char dec_round_keys[NUM_ROUND_KEYS_IN_ARRAY * SIZE_OF_AES_ROUND_KEY];
};
/** @endcond */

/**
 * @brief Initialize the AES context.
 *
 * @param ctx Pointer to context
 * @param key_type Variant of AES to use
 * @param key Pointer to AES key
 */
void aes_init(AesContext *ctx, enum AesKeyType key_type,
              const unsigned char *key);

/**
 * @brief Encrypt or decrypt data in plain_text using AES CTR mode and store the
 * output to cipher_text
 *
 * @param ctx pointer to AES state
 * @param textsize size of data to be encrypted
 * @param out pointer to memory where encrypted/decrypted data must be written
 * to. Size of out must be >= textsize.
 * @param in pointer to data to be encrypted/decrypted
 * @param next_iv pointer to memory where the next counter be stored. Can be
 * NULL
 * @param iv pointer to counter to be used by function
 */
void aes_ctr_xcrypt(AesContext *ctx, size_t textsize, unsigned char *out,
                    const unsigned char *in, unsigned char next_iv[16],
                    const unsigned char iv[16]);

/**
 * @brief Encrypt data in plain_text using AES ECB mode and store the
 * encrypted data to cipher_text
 *
 * @param ctx pointer to AES state
 * @param textsize size of data to be encrypted. It must be divisible by 16.
 * @param cipher_text pointer to memory where encrypted data must be written to.
 * Size of cipher_text must be >= textsize.
 * @param plain_text pointer to data to be encrypted
 */
void aes_ecb_encrypt(AesContext *ctx, size_t textsize,
                     unsigned char *cipher_text,
                     const unsigned char *plain_text);

/**
 * @brief Decrypt data in cipher_text using AES ECB mode and store the
 * decrypted data to plain_text
 *
 * @param ctx pointer to AES state
 * @param textsize size of data to be decrypted. It must be divisible by 16.
 * @param plain_text pointer to memory where decrypted data is to be written.
 * Size of plain_text must be >= textsize.
 * @param cipher_text pointer to data to be decrypted
 */
void aes_ecb_decrypt(AesContext *ctx, size_t textsize,
                     unsigned char *plain_text,
                     const unsigned char *cipher_text);

/**
 * @brief Encrypt data in plain_text using AES CBC mode and store the
 * encrypted data to cipher_text
 *
 * @param ctx pointer to AES state
 * @param textsize size of data to be encrypted. It must be divisible by 16.
 * @param cipher_text pointer to memory where encrypted data must be written to.
 * Size of cipher_text must be >= textsize.
 * @param plain_text pointer to data to be encrypted
 * @param iv Initialization Vector to be used
 */
void aes_cbc_encrypt(AesContext *ctx, size_t textsize,
                     unsigned char *cipher_text,
                     const unsigned char *plain_text,
                     const unsigned char iv[16]);

/**
 * @brief Decrypt data in cipher_text using AES CBC mode and store the
 * decrypted data to plain_text
 *
 * @param ctx pointer to AES state
 * @param textsize size of data to be decrypted. It must be divisible by 16.
 * @param plain_text pointer to memory where decrypted data is to be written.
 * Size of plain_text must be >= textsize.
 * @param cipher_text pointer to data to be decrypted
 * @param iv Initialization Vector to be used
 */
void aes_cbc_decrypt(AesContext *ctx, size_t textsize,
                     unsigned char *plain_text,
                     const unsigned char *cipher_text,
                     const unsigned char iv[16]);

#undef SIZE_OF_AES_ROUND_KEY
#undef NUM_ROUND_KEYS_IN_ARRAY

HEDLEY_END_C_DECLS

#endif /* AY_AES_H */
