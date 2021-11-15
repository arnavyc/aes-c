.. include:: <isonum.txt>

=================================
Welcome to aes-c's documentation!
=================================

.. image:: https://img.shields.io/github/license/arnavyc/aes-c?style=flat-square
   :alt: GitHub

C library for encrypting & decrypting data with AES-128/192/256. (WIP)

Installation
============

WIP

Usage
=====

Short Introduction
------------------

Suppose, we need to encrypt data in ``inbuf`` with AES-128-CTR & put the encrypted data into ``outbuf``:

.. code-block:: c

   /* ... ... are to be replaced with appropriate values. */
   const unsigned char inbuf[100] = ... data ...; /* Plain text. */
   const unsigned char key[16] = ... key ...; /* Key to encrypt `inbuf` with. */
   unsigned char outbuf[100]; /* Destination of encrypted data. */

First, we would initialize the AES context with :cpp:func:`aes_init` function:

.. code-block:: c

   AesContext ctx;
   aes_init(&ctx, KEY_TYPE_AES128, key);

Then, we would use :cpp:func:`aes_ctr_xcrypt()` to encrypt the data:

.. code-block:: c

   /* Set initial IV. Replace with appropriate value. */
   const unsigned char iv[16] = ... iv ...;
   unsigned char new_iv[16]; /* New IV. */

   aes_ctr_xcrypt(&ctx, sizeof outbuf, outbuf, inbuf, iv, new_iv);

Other modes
-----------

For other modes of operation in AES:

ECB mode
^^^^^^^^

- For encrypting data using ECB mode, use :cpp:func:`aes_ecb_encrypt`.
- For decrypting data using ECB mode, use :cpp:func:`aes_ecb_decrypt`.

CBC mode
^^^^^^^^

- For encrypting data using CBC mode, use :cpp:func:`aes_cbc_encrypt`.
- For decrypting data using CBC mode, use :cpp:func:`aes_cbc_decrypt`.

CTR mode
^^^^^^^^

For encrypting or decrypting data using CTR mode, use :cpp:func:`aes_ctr_xcrypt`.

Other information
-----------------

For more information, see :doc:`api`.

Contributing
============

Unless you explicitly state otherwise, any contribution intentionally submitted
for inclusion in the work by you, shall be licensed as below, without any
additional terms or conditions.

License
=======

|copy| 2021 arnavyc \<arnavyc@outlook.com\>

Licensed under the BSD Zero Clause License. See :doc:`LICENSE file <LICENSE>` in
the project root, or https://opensource.org/licenses/0BSD for full license
information.

   SPDX-License-Identifier: 0BSD
