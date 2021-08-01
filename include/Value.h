#ifndef JON_VALUE_H
#define JON_VALUE_H

#include <variant>
#include <memory>
#include <string>
#include <assert.h>

#include "Lexer.h"

namespace jon::val {
    struct Value;
    using value_ptr = std::unique_ptr<Value>;
    using int_t = int64_t;

    enum class Type {
        Null,
        Bool,
        Int,
        Float,
        String,
        Object,
        Array,
    };

    struct Value {
        Value(Type type) : type(type) {}

        Type type;
    };

    const auto null = Value(Type::Null);

    struct Bool : Value {
        Bool(const Token & token) : Value(Type::Bool) {
            assert(token.kind == TokenKind::True or token.kind == TokenKind::False);
            val = token.kind == TokenKind::True;
        }

        bool val;
    };

    struct Int : Value {
        Int(const Token & token) : Value(Type::Int) {
            assert(token.kind == TokenKind::DecInt or token.kind == TokenKind::HexInt or
                   token.kind == TokenKind::OctoInt or token.kind == TokenKind::BinInt);
            val = std::stoull(token.val, nullptr);
        }

        int_t val;
    };

    struct Float : Value {
        Float(double val) : Value(Type::Float), val(val) {}

        double val;
    };

    struct String : Value {
        String(const std::string & val) : Value(Type::String), val(val) {}

        std::string val;
    };

    struct Object : Value {
        String() : Value(Type::String), val(val) {}

        std::string val;
    };
}

#endif // JON_VALUE_H
