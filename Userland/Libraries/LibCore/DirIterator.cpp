/*
 * Copyright (c) 2018-2020, Andreas Kling <kling@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/Vector.h>
#include <LibCore/DirIterator.h>
#include <errno.h>
#include <unistd.h>

namespace Core {

DirIterator::DirIterator(DeprecatedString path, Flags flags)
    : m_path(move(path))
    , m_flags(flags)
{
    m_dir = opendir(m_path.characters());
    if (!m_dir) {
        m_error = errno;
    }
}

DirIterator::~DirIterator()
{
    if (m_dir) {
        closedir(m_dir);
        m_dir = nullptr;
    }
}

DirIterator::DirIterator(DirIterator&& other)
    : m_dir(other.m_dir)
    , m_error(other.m_error)
    , m_next(move(other.m_next))
    , m_path(move(other.m_path))
    , m_flags(other.m_flags)
{
    other.m_dir = nullptr;
}

bool DirIterator::advance_next()
{
    if (!m_dir)
        return false;

    while (true) {
        errno = 0;
        auto* de = readdir(m_dir);
        if (!de) {
            m_error = errno;
            m_next = DeprecatedString();
            return false;
        }

        m_next = de->d_name;
        if (m_next.is_null())
            return false;

        if (m_flags & Flags::SkipDots && m_next.starts_with('.'))
            continue;

        if (m_flags & Flags::SkipParentAndBaseDir && (m_next == "." || m_next == ".."))
            continue;

        return !m_next.is_empty();
    }
}

bool DirIterator::has_next()
{
    if (!m_next.is_null())
        return true;

    return advance_next();
}

DeprecatedString DirIterator::next_path()
{
    if (m_next.is_null())
        advance_next();

    auto tmp = m_next;
    m_next = DeprecatedString();
    return tmp;
}

DeprecatedString DirIterator::next_full_path()
{
    StringBuilder builder;
    builder.append(m_path);
    if (!m_path.ends_with('/'))
        builder.append('/');
    builder.append(next_path());
    return builder.to_deprecated_string();
}

int DirIterator::fd() const
{
    if (!m_dir)
        return -1;
    return dirfd(m_dir);
}

}
