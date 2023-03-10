/*
 * Copyright (c) 2020, Sergey Bugaev <bugaevc@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/StringView.h>
#include <AK/Vector.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int opterr = 1;
int optopt = 0;
int optind = 1;
int optreset = 0;
char* optarg = nullptr;

// POSIX says, "When an element of argv[] contains multiple option characters,
// it is unspecified how getopt() determines which options have already been
// processed". Well, this is how we do it.
static size_t s_index_into_multioption_argument = 0;

[[gnu::format(printf, 1, 2)]] static inline void report_error(char const* format, ...)
{
    if (!opterr)
        return;

    fputs("\033[31m", stderr);

    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);

    fputs("\033[0m\n", stderr);
}

namespace {

class OptionParser {
public:
    OptionParser(int argc, char* const* argv, StringView short_options, option const* long_options, int* out_long_option_index = nullptr);
    int getopt();

private:
    bool lookup_short_option(char option, int& needs_value) const;
    int handle_short_option();

    option const* lookup_long_option(char* raw) const;
    int handle_long_option();

    void shift_argv();
    bool find_next_option();

    StringView current_arg() const
    {
        auto const* arg_ptr = m_argv[m_arg_index];
        if (arg_ptr == NULL)
            return {};
        return { arg_ptr, strlen(arg_ptr) };
    }

    size_t m_argc { 0 };
    char* const* m_argv { nullptr };
    StringView m_short_options;
    option const* m_long_options { nullptr };
    int* m_out_long_option_index { nullptr };
    bool m_stop_on_first_non_option { false };

    size_t m_arg_index { 0 };
    size_t m_consumed_args { 0 };
};

OptionParser::OptionParser(int argc, char* const* argv, StringView short_options, option const* long_options, int* out_long_option_index)
    : m_argc(argc)
    , m_argv(argv)
    , m_short_options(short_options)
    , m_long_options(long_options)
    , m_out_long_option_index(out_long_option_index)
{
    // In the following case:
    // $ foo bar -o baz
    // we want to parse the option (-o baz) first, and leave the argument (bar)
    // in argv after we return -1 when invoked the second time. So we reorder
    // argv to put options first and positional arguments next. To turn this
    // behavior off, start the short options spec with a "+". This is a GNU
    // extension that we support.
    m_stop_on_first_non_option = short_options.starts_with('+');

    // See if we should reset the internal state.
    if (optreset || optind == 0) {
        optreset = 0;
        optind = 1;
        s_index_into_multioption_argument = 0;
    }

    optopt = 0;
    optarg = nullptr;
}

int OptionParser::getopt()
{
    bool should_reorder_argv = !m_stop_on_first_non_option;
    int res = -1;

    bool found_an_option = find_next_option();
    auto arg = current_arg();

    if (!found_an_option) {
        res = -1;
        if (arg == "--")
            m_consumed_args = 1;
        else
            m_consumed_args = 0;
    } else {
        // Alright, so we have an option on our hands!
        bool is_long_option = arg.starts_with("--"sv);
        if (is_long_option)
            res = handle_long_option();
        else
            res = handle_short_option();

        // If we encountered an error, return immediately.
        if (res == '?')
            return '?';
    }

    if (should_reorder_argv)
        shift_argv();
    else
        VERIFY(optind == static_cast<int>(m_arg_index));
    optind += m_consumed_args;

    return res;
}

bool OptionParser::lookup_short_option(char option, int& needs_value) const
{
    Vector<StringView> parts = m_short_options.split_view(option, SplitBehavior::KeepEmpty);

    VERIFY(parts.size() <= 2);
    if (parts.size() < 2) {
        // Haven't found the option in the spec.
        return false;
    }

    if (parts[1].starts_with("::"sv)) {
        // If an option is followed by two colons, it optionally accepts an
        // argument.
        needs_value = optional_argument;
    } else if (parts[1].starts_with(':')) {
        // If it's followed by one colon, it requires an argument.
        needs_value = required_argument;
    } else {
        // Otherwise, it doesn't accept arguments.
        needs_value = no_argument;
    }
    return true;
}

int OptionParser::handle_short_option()
{
    StringView arg = current_arg();
    VERIFY(arg.starts_with('-'));

    if (s_index_into_multioption_argument == 0) {
        // Just starting to parse this argument, skip the "-".
        s_index_into_multioption_argument = 1;
    }
    char option = arg[s_index_into_multioption_argument];
    s_index_into_multioption_argument++;

    int needs_value = no_argument;
    bool ok = lookup_short_option(option, needs_value);
    if (!ok) {
        optopt = option;
        report_error("Unrecognized option \033[1m-%c\033[22m", option);
        return '?';
    }

    // Let's see if we're at the end of this argument already.
    if (s_index_into_multioption_argument < arg.length()) {
        // This not yet the end.
        if (needs_value == no_argument) {
            optarg = nullptr;
            m_consumed_args = 0;
        } else {
            // Treat the rest of the argument as the value, the "-ovalue"
            // syntax.
            optarg = m_argv[m_arg_index] + s_index_into_multioption_argument;
            // Next time, process the next argument.
            s_index_into_multioption_argument = 0;
            m_consumed_args = 1;
        }
    } else {
        s_index_into_multioption_argument = 0;
        if (needs_value != required_argument) {
            optarg = nullptr;
            m_consumed_args = 1;
        } else if (m_arg_index + 1 < m_argc) {
            // Treat the next argument as a value, the "-o value" syntax.
            optarg = m_argv[m_arg_index + 1];
            m_consumed_args = 2;
        } else {
            report_error("Missing value for option \033[1m-%c\033[22m", option);
            return '?';
        }
    }

    return option;
}

option const* OptionParser::lookup_long_option(char* raw) const
{
    StringView arg { raw, strlen(raw) };

    for (size_t index = 0; m_long_options[index].name; index++) {
        auto& option = m_long_options[index];
        StringView name { option.name, strlen(option.name) };

        if (!arg.starts_with(name))
            continue;

        // It would be better to not write out the index at all unless we're
        // sure we've found the right option, but whatever.
        if (m_out_long_option_index)
            *m_out_long_option_index = index;

        // Can either be "--option" or "--option=value".
        if (arg.length() == name.length()) {
            optarg = nullptr;
            return &option;
        }
        VERIFY(arg.length() > name.length());
        if (arg[name.length()] == '=') {
            optarg = raw + name.length() + 1;
            return &option;
        }
    }

    return nullptr;
}

int OptionParser::handle_long_option()
{
    VERIFY(current_arg().starts_with("--"sv));

    // We cannot set optopt to anything sensible for long options, so set it to 0.
    optopt = 0;

    auto* option = lookup_long_option(m_argv[m_arg_index] + 2);
    if (!option) {
        report_error("Unrecognized option \033[1m%s\033[22m", m_argv[m_arg_index]);
        return '?';
    }
    // lookup_long_option() will also set optarg if the value of the option is
    // specified using "--option=value" syntax.

    // Figure out whether this option needs and/or has a value (also called "an
    // argument", but let's not call it that to distinguish it from argv
    // elements).
    switch (option->has_arg) {
    case no_argument:
        if (optarg) {
            report_error("Option \033[1m--%s\033[22m doesn't accept an argument", option->name);
            return '?';
        }
        m_consumed_args = 1;
        break;
    case optional_argument:
        m_consumed_args = 1;
        break;
    case required_argument:
        if (optarg) {
            // Value specified using "--option=value" syntax.
            m_consumed_args = 1;
        } else if (m_arg_index + 1 < m_argc) {
            // Treat the next argument as a value in "--option value" syntax.
            optarg = m_argv[m_arg_index + 1];
            m_consumed_args = 2;
        } else {
            report_error("Missing value for option \033[1m--%s\033[22m", option->name);
            return '?';
        }
        break;
    default:
        VERIFY_NOT_REACHED();
    }

    // Now that we've figured the value out, see about reporting this option to
    // our caller.
    if (option->flag) {
        *option->flag = option->val;
        return 0;
    }
    return option->val;
}

void OptionParser::shift_argv()
{
    // We've just parsed an option (which perhaps has a value).
    // Put the option (along with it value, if any) in front of other arguments.
    VERIFY(optind <= static_cast<int>(m_arg_index));

    if (optind == static_cast<int>(m_arg_index) || m_consumed_args == 0) {
        // Nothing to do!
        return;
    }

    auto new_argv = const_cast<char**>(m_argv);
    char* buffer[m_consumed_args];
    memcpy(buffer, &new_argv[m_arg_index], sizeof(char*) * m_consumed_args);
    memmove(&new_argv[optind + m_consumed_args], &new_argv[optind], sizeof(char*) * (m_arg_index - optind));
    memcpy(&new_argv[optind], buffer, sizeof(char*) * m_consumed_args);
}

bool OptionParser::find_next_option()
{
    for (m_arg_index = optind; m_arg_index < m_argc && m_argv[m_arg_index]; m_arg_index++) {
        StringView arg = current_arg();
        // Anything that doesn't start with a "-" is not an option.
        // As a special case, a single "-" is not an option either.
        // (It's typically used by programs to refer to stdin).
        if (!arg.starts_with('-') || arg == "-") {
            if (m_stop_on_first_non_option)
                return false;
            continue;
        }

        // As another special case, a "--" is not an option either, and we stop
        // looking for further options if we encounter it.
        if (arg == "--")
            return false;
        // Otherwise, we have found an option!
        return true;
    }

    // Reached the end and still found no options.
    return false;
}

}

int getopt(int argc, char* const* argv, char const* short_options)
{
    option dummy { nullptr, 0, nullptr, 0 };
    OptionParser parser { argc, argv, { short_options, strlen(short_options) }, &dummy };
    return parser.getopt();
}

int getopt_long(int argc, char* const* argv, char const* short_options, const struct option* long_options, int* out_long_option_index)
{
    OptionParser parser { argc, argv, { short_options, strlen(short_options) }, long_options, out_long_option_index };
    return parser.getopt();
}
