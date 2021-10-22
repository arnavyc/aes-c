<!--
SPDX-License-Identifier: 0BSD
-->

# aes-c

[![standard-readme compliant](https://img.shields.io/badge/readme%20style-standard-brightgreen.svg?style=flat-square)](https://github.com/RichardLitt/standard-readme)

C library for encrypting & decrypting data with AES-128/192/256. (WIP)

## Table of Contents

- [Install](#install)
- [Usage](#usage)
- [Contributing](#contributing)
- [License](#license)

## Install

WIP

## Usage

Suppose, we need to encrypt data in `inbuf` with AES-128-CTR & put the encrypted
data into `outbuf`:

```c
/* ... ... are to be replaced with appropriate values. */
const unsigned char inbuf[100] = ... data ...; /* Plain text. */
const unsigned char key[16] = ... key ...; /* Key to encrypt `inbuf` with. */
unsigned char outbuf[100]; /* Destination of encrypted data. */
```

First, we would initialize the AES context with `aes_init` function:

```c
AesContext ctx;
aes_init(&ctx, KEY_TYPE_AES128, key);
```

Then, we would use `aes_ctr_xcrypt()` to encrypt the data:

```c
/* Set initial IV. Replace with appropriate value. */
const unsigned char iv[16] = ... iv ...;
unsigned char new_iv[16]; /* New IV. */

aes_ctr_xcrypt(&ctx, sizeof outbuf, outbuf, inbuf, iv, new_iv);
```

## API

For more information, see [aes.h](include/ay/aes.h).

### Initialization

Initialize `AesContext` using `aes_init(ctx, key_size, key)` where
- `ctx` is pointer to `AesContext`
- `key_size` = 128, 192, 256 for AES-128, AES-192 & AES-256 respectively
- `key` is pointer to AES key

### ECB mode
- For encrypting data using ECB mode, use `aes_ecb_encrypt`.
- For decrypting data using ECB mode, use `aes_ecb_decrypt`.

### CBC mode
- For encrypting data using CBC mode, use `aes_cbc_encrypt`.
- For decrypting data using CBC mode, use `aes_cbc_decrypt`.

### CTR mode
For encrypting or decrypting data using CTR mode, use `aes_ctr_xcrypt`.

## Contributing

Unless you explicitly state otherwise, any contribution intentionally submitted
for inclusion in the work by you, shall be licensed as below, without any
additional terms or conditions.

## License

&copy; 2021 arnavyc \<arnavyc@outlook.com\>

Licensed under the BSD Zero Clause License. See [LICENSE file](LICENSE.md) in
the project root, or https://opensource.org/licenses/0BSD for full license
information.

```
SPDX-License-Identifier: 0BSD
```
