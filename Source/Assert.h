#pragma once

#include <ppk_assert.h>

#if defined(ASSERT)
#  error "ASSERT already defined"
#endif

#define ASSERT PPK_ASSERT
