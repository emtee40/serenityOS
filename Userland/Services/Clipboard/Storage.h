/*
 * Copyright (c) 2020, Andreas Kling <kling@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/DeprecatedString.h>
#include <AK/Function.h>
#include <AK/HashMap.h>
#include <LibCore/AnonymousBuffer.h>

namespace Clipboard {

class Storage {
public:
    static Storage& the();
    ~Storage() = default;

    bool has_data() const { return m_buffer.is_valid(); }

    DeprecatedString const& mime_type() const { return m_mime_type; }
    HashMap<DeprecatedString, DeprecatedString> const& metadata() const { return m_metadata; }

    u8 const* data() const
    {
        if (!has_data())
            return nullptr;
        return m_buffer.data<u8>();
    }

    size_t data_size() const
    {
        if (has_data())
            return m_data_size;
        return 0;
    }

    void set_data(Core::AnonymousBuffer, DeprecatedString const& mime_type, HashMap<DeprecatedString, DeprecatedString> const& metadata);

    Function<void()> on_content_change;

    Core::AnonymousBuffer const& buffer() const { return m_buffer; }

private:
    Storage() = default;

    DeprecatedString m_mime_type;
    Core::AnonymousBuffer m_buffer;
    size_t m_data_size { 0 };
    HashMap<DeprecatedString, DeprecatedString> m_metadata;
};

}
