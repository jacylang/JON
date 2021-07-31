#ifndef JON_AST_H
#define JON_AST_H

#include <string>
#include <vector>
#include <memory>

namespace jon {
    struct Null;
    struct Bool;
    struct Int;
    struct Float;
    struct String;
    struct Object;
    struct Array;

    struct ValueVisitor {
    public:
        virtual void accept(const Null&) = 0;
        virtual void accept(const Bool&) = 0;
        virtual void accept(const Int&) = 0;
        virtual void accept(const Float&) = 0;
        virtual void accept(const String&) = 0;
        virtual void accept(const Object&) = 0;
        virtual void accept(const Array&) = 0;
    };
}

namespace jon {
    struct Value;
    using value_ptr = std::unique_ptr<Value>;
    using value_list = std::vector<value_ptr>;

    struct Node {};

    struct Ident : Node {
        Ident(const std::string & val) : val(val) {}

        std::string val;
    };

    enum class ValueKind {
        Null,
        Bool,
        Int,
        Float,
        String,
        Object,
        Array,
    };

    struct Value {
        Value(ValueKind kind) : kind(kind) {}

        ValueKind kind;
    };

    struct Null : Value {
        Null() : Value(ValueKind::Null) {}
    };

    struct Bool : Value {
        Bool(bool val) : Value(ValueKind::Bool), val(val) {}

        bool val;
    };

    struct Int : Value {
        Int(const std::string & val) : Value(ValueKind::Int), val(val) {}

        std::string val;
    };

    struct Float : Value {
        Float(const std::string & val) : Value(ValueKind::Float), val(val) {}

        std::string val;
    };

    struct String : Value {
        String(const std::string & val) : Value(ValueKind::String), val(val) {}

        std::string val;
    };

    struct KeyValue {
        KeyValue(Ident && key, value_ptr && val) : key(std::move(key)), val(std::move(val)) {}

        Ident key;
        value_ptr val;
    };

    struct Object : Value {
        using Entries = std::vector<KeyValue>;

        Object(Entries && entries) : Value(ValueKind::Object), entries(std::move(entries)) {}

        Entries entries;
    };

    struct Array : Value {
        Array(value_list && values) : Value(ValueKind::Array), values(std::move(values)) {}

        value_list values;
    };
}

#endif // JON_AST_H
