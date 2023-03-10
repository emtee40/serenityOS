/*
 * Copyright (c) 2020-2021, the SerenityOS developers.
 * Copyright (c) 2022, Sam Atkins <atkinssj@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/DeprecatedString.h>
#include <LibWeb/CSS/Parser/Token.h>
#include <LibWeb/CSS/Serialize.h>

namespace Web::CSS::Parser {

DeprecatedString Token::to_deprecated_string() const
{
    StringBuilder builder;

    switch (m_type) {
    case Type::EndOfFile:
        return "";
    case Type::Ident:
        return serialize_an_identifier(ident());
    case Type::Function:
        return DeprecatedString::formatted("{}(", serialize_an_identifier(function()));
    case Type::AtKeyword:
        return DeprecatedString::formatted("@{}", serialize_an_identifier(at_keyword()));
    case Type::Hash: {
        switch (m_hash_type) {
        case HashType::Id:
            return DeprecatedString::formatted("#{}", serialize_an_identifier(hash_value()));
        case HashType::Unrestricted:
            return DeprecatedString::formatted("#{}", hash_value());
        }
        VERIFY_NOT_REACHED();
    }
    case Type::String:
        return serialize_a_string(string());
    case Type::BadString:
        return "";
    case Type::Url:
        return serialize_a_url(url());
    case Type::BadUrl:
        return "url()";
    case Type::Delim:
        return m_value;
    case Type::Number:
        return DeprecatedString::number(m_number_value.value());
    case Type::Percentage:
        return DeprecatedString::formatted("{}%", m_number_value.value());
    case Type::Dimension:
        return DeprecatedString::formatted("{}{}", m_number_value.value(), dimension_unit());
    case Type::Whitespace:
        return " ";
    case Type::CDO:
        return "<!--";
    case Type::CDC:
        return "-->";
    case Type::Colon:
        return ":";
    case Type::Semicolon:
        return ";";
    case Type::Comma:
        return ",";
    case Type::OpenSquare:
        return "[";
    case Type::CloseSquare:
        return "]";
    case Type::OpenParen:
        return "(";
    case Type::CloseParen:
        return ")";
    case Type::OpenCurly:
        return "{";
    case Type::CloseCurly:
        return "}";
    case Type::Invalid:
    default:
        VERIFY_NOT_REACHED();
    }
}

DeprecatedString Token::to_debug_string() const
{
    switch (m_type) {
    case Type::Invalid:
        VERIFY_NOT_REACHED();

    case Type::EndOfFile:
        return "__EOF__";
    case Type::Ident:
        return DeprecatedString::formatted("Ident: {}", ident());
    case Type::Function:
        return DeprecatedString::formatted("Function: {}", function());
    case Type::AtKeyword:
        return DeprecatedString::formatted("AtKeyword: {}", at_keyword());
    case Type::Hash:
        return DeprecatedString::formatted("Hash: {} (hash_type: {})", hash_value(), m_hash_type == HashType::Unrestricted ? "Unrestricted" : "Id");
    case Type::String:
        return DeprecatedString::formatted("String: {}", string());
    case Type::BadString:
        return "BadString";
    case Type::Url:
        return DeprecatedString::formatted("Url: {}", url());
    case Type::BadUrl:
        return "BadUrl";
    case Type::Delim:
        return DeprecatedString::formatted("Delim: {}", m_value);
    case Type::Number:
        return DeprecatedString::formatted("Number: {}{} (number_type: {})", m_number_value.value() > 0 && m_number_value.is_integer_with_explicit_sign() ? "+" : "", m_number_value.value(), m_number_value.is_integer() ? "Integer" : "Number");
    case Type::Percentage:
        return DeprecatedString::formatted("Percentage: {}% (number_type: {})", percentage(), m_number_value.is_integer() ? "Integer" : "Number");
    case Type::Dimension:
        return DeprecatedString::formatted("Dimension: {}{} (number_type: {})", dimension_value(), dimension_unit(), m_number_value.is_integer() ? "Integer" : "Number");
    case Type::Whitespace:
        return "Whitespace";
    case Type::CDO:
        return "CDO";
    case Type::CDC:
        return "CDC";
    case Type::Colon:
        return "Colon";
    case Type::Semicolon:
        return "Semicolon";
    case Type::Comma:
        return "Comma";
    case Type::OpenSquare:
        return "OpenSquare";
    case Type::CloseSquare:
        return "CloseSquare";
    case Type::OpenParen:
        return "OpenParen";
    case Type::CloseParen:
        return "CloseParen";
    case Type::OpenCurly:
        return "OpenCurly";
    case Type::CloseCurly:
        return "CloseCurly";
    }
    VERIFY_NOT_REACHED();
}

Token::Type Token::mirror_variant() const
{
    if (is(Token::Type::OpenCurly)) {
        return Type::CloseCurly;
    }

    if (is(Token::Type::OpenSquare)) {
        return Type::CloseSquare;
    }

    if (is(Token::Type::OpenParen)) {
        return Type::CloseParen;
    }

    return Type::Invalid;
}

DeprecatedString Token::bracket_string() const
{
    if (is(Token::Type::OpenCurly)) {
        return "{";
    }

    if (is(Token::Type::CloseCurly)) {
        return "}";
    }

    if (is(Token::Type::OpenSquare)) {
        return "[";
    }

    if (is(Token::Type::CloseSquare)) {
        return "]";
    }

    if (is(Token::Type::OpenParen)) {
        return "(";
    }

    if (is(Token::Type::CloseParen)) {
        return ")";
    }

    return "";
}

DeprecatedString Token::bracket_mirror_string() const
{
    if (is(Token::Type::OpenCurly)) {
        return "}";
    }

    if (is(Token::Type::CloseCurly)) {
        return "{";
    }

    if (is(Token::Type::OpenSquare)) {
        return "]";
    }

    if (is(Token::Type::CloseSquare)) {
        return "[";
    }

    if (is(Token::Type::OpenParen)) {
        return ")";
    }

    if (is(Token::Type::CloseParen)) {
        return "(";
    }

    return "";
}

}
