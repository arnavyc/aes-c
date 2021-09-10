#ifndef AY_AES_INNER_H
#define AY_AES_INNER_H

#include <ay/aes.h>

#if defined(__GNUC__)
#define AY_TARGET(x) __attribute__((target(x)))
#else
#define AY_TARGET(x)
#endif /* defined (__GNUC__) */

#if defined(__GNUC__)
#define AY_FLATTEN __attribute__((flatten))
#else
#define AY_FLATTEN
#endif /* defined (__GNUC__) */

#endif
