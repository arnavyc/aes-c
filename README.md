<!--
SPDX-License-Identifier: 0BSD
-->

# aes-c

[![GitHub](https://img.shields.io/github/license/arnavyc/aes-c?logo=github&style=flat-square)](LICENSE.md)
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

- Initialize `AesContext` using `aes_init(ctx, key_size, key)` where
  - `ctx` is pointer to `AesContext`
  - `key_size` = 128, 192, 256 for AES-128, AES-192 & AES-256 respectively
  - `key` is pointer to AES key
- ECB mode
  - For encrypting data using ECB mode, use `aes_ecb_encrypt`.
  - For decrypting data using ECB mode, use `aes_ecb_decrypt`.
- CBC mode
  - For encrypting data using CBC mode, use `aes_cbc_encrypt`.
  - For decrypting data using CBC mode, use `aes_cbc_decrypt`.
- CTR mode
  - For encrypting or decrypting data using CTR mode, use `aes_ctr_xcrypt`.

For more information, see [aes.h](include/ay/aes.h).

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
