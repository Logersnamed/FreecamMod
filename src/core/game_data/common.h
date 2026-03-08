#pragma once
#include <cstddef>
#include <type_traits>

#include "utils/types.h"

#define ASSERT_OFFSET(type, field, offset) \
    static_assert(offsetof(type, field) == offset, #type "::" #field " offset incorrect")

#define ASSERT_SIZE(type, size) \
    static_assert(sizeof(type) == size, #type " size incorrect")