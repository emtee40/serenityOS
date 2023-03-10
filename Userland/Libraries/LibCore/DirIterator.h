/*
 * Copyright (c) 2018-2020, Andreas Kling <kling@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/DeprecatedString.h>
#include <dirent.h>
#include <string.h>

namespace Core {

class DirIterator {
public:
    enum Flags {
        NoFlags = 0x0,
        SkipDots = 0x1,
        SkipParentAndBaseDir = 0x2,
    };

    explicit DirIterator(DeprecatedString path, Flags = Flags::NoFlags);
    ~DirIterator();

    DirIterator(DirIterator&&);
    DirIterator(DirIterator const&) = delete;

    bool has_error() const { return m_error != 0; }
    int error() const { return m_error; }
    char const* error_string() const { return strerror(m_error); }
    bool has_next();
    DeprecatedString next_path();
    DeprecatedString next_full_path();
    int fd() const;

private:
    DIR* m_dir = nullptr;
    int m_error = 0;
    DeprecatedString m_next;
    DeprecatedString m_path;
    int m_flags;

    bool advance_next();
};

}
