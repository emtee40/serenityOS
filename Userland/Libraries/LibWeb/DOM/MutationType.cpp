/*
 * Copyright (c) 2022, Luke Wilde <lukew@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/DOM/MutationType.h>

namespace Web::DOM::MutationType {

#define __ENUMERATE_MUTATION_TYPE(name) DeprecatedFlyString name;
ENUMERATE_MUTATION_TYPES
#undef __ENUMERATE_MUTATION_TYPE

[[gnu::constructor]] static void initialize()
{
    static bool s_initialized = false;
    if (s_initialized)
        return;

#define __ENUMERATE_MUTATION_TYPE(name) name = #name;
    ENUMERATE_MUTATION_TYPES
#undef __ENUMERATE_MUTATION_TYPE

    s_initialized = true;
}

}
