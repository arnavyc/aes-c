#ifndef AY_AES_H
#define AY_AES_H

#include <stdalign.h>
#include <stddef.h>

#define SIZE_OF_AES_ROUND_KEY 16
#define NUM_ROUND_KEYS_IN_ARRAY 15

enum AesKeyType {
  KEY_TYPE_AES128 = 128,
  KEY_TYPE_AES192 = 192,
  KEY_TYPE_AES256 = 256
};

typedef struct AesContext AesContext;

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

struct AesContext {
  unsigned short key_size;
  unsigned char Nr;
  struct aes_vtable vtable;
  alignas(16) unsigned char enc_round_keys[NUM_ROUND_KEYS_IN_ARRAY *
                                           SIZE_OF_AES_ROUND_KEY];
  alignas(16) unsigned char dec_round_keys[NUM_ROUND_KEYS_IN_ARRAY *
                                           SIZE_OF_AES_ROUND_KEY];
};

void aes_init(AesContext *ctx, enum AesKeyType key_type,
              const unsigned char *key);

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

void aes_cbc_encrypt(AesContext *ctx, size_t textsize,
                     unsigned char *cipher_text,
                     const unsigned char *plain_text,
                     const unsigned char iv[16]);

void aes_cbc_decrypt(AesContext *ctx, size_t textsize,
                     unsigned char *plain_text,
                     const unsigned char *cipher_text,
                     const unsigned char iv[16]);

#undef SIZE_OF_AES_ROUND_KEY
#undef NUM_ROUND_KEYS_IN_ARRAY

#endif /* AY_AES_H */
