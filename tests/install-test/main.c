#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <ay/aes.h>

int main(int argc, char *argv[]) {
  const unsigned char key[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
                                 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
                                 0x0c, 0x0d, 0x0e, 0x0f};
  const unsigned char plain_text[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
                                        0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
                                        0xcc, 0xdd, 0xee, 0xff};
  unsigned char actual_cipher_text[16];

  AesContext ctx;
  aes_init(&ctx, 128, key);
  aes_ecb_encrypt(&ctx, 16, actual_cipher_text, plain_text);

  for (size_t i = 0; i < sizeof actual_cipher_text; ++i) {
    printf("%02x ", actual_cipher_text[i]);
  }
  puts("");

  unsigned char new_plain_text[16];

  aes_init(&ctx, KEY_TYPE_AES128, key);
  aes_ecb_decrypt(&ctx, 16, new_plain_text, actual_cipher_text);

  for (size_t i = 0; i < sizeof new_plain_text; ++i) {
    printf("%02x ", new_plain_text[i]);
  }
  puts("");

  return EXIT_SUCCESS;
}
