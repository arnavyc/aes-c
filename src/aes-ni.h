#ifndef AY_AES_NI_H
#define AY_AES_NI_H

#include <stddef.h>
#include <wmmintrin.h>

#include <ay/aes.h>

void aesni_init(AesContext *ctx, enum AesKeyType key_size,
                const unsigned char *key);
void aesni_ctr_xcrypt(AesContext *ctx, size_t textsize, unsigned char *out,
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
void aesni_ecb_encrypt(AesContext *ctx, size_t textsize,
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
void aesni_ecb_decrypt(AesContext *ctx, size_t textsize,
                       unsigned char *plain_text,
                       const unsigned char *cipher_text);

void aesni_cbc_encrypt(AesContext *ctx, size_t textsize,
                       unsigned char *cipher_text,
                       const unsigned char *plain_text,
                       const unsigned char iv[16]);

void aesni_cbc_decrypt(AesContext *ctx, size_t textsize,
                       unsigned char *plain_text,
                       const unsigned char *cipher_text,
                       const unsigned char iv[16]);

#endif /* AY_AES_NI_H */
