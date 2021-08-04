#ifndef JON_LEXER_H
#define JON_LEXER_H

#include <string>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <assert.h>

#include "utils.h"
#include "error.h"

namespace jon {
    struct Token;
    using TokenStream = std::vector<Token>;

    enum class TokenKind {
        Eof,

        NL,

        // Punctuations
        Comma,
        Colon,
        LBrace,
        RBrace,
        LBracket,
        RBracket,

        Null,
        False,
        True,
        NaN,
        Inf,
        NegInf,

        BinInt,
        HexInt,
        OctoInt,
        DecInt,

        Float,

        /// Either string enclosed into quotes (maybe triple if multi-line) or identifier
        // Note: Separate with identifier if would be needed
        String,
    };

    struct Span {
        using pos_t = size_t;
        using len_t = size_t;

        Span(pos_t pos, len_t len) : pos(pos), len(len) {}

        const pos_t pos;
        const len_t len;
    };

    struct Token {
        Token(TokenKind kind, const std::string & val, const Span & span) : kind(kind), val(val), span(span) {}

        const TokenKind kind;
        const std::string val;
        const Span span;

        std::string toString() const {
            switch (kind) {
                case TokenKind::Eof: {
                    return "[EOF]";
                }
                case TokenKind::NL: {
                    return "new line";
                }
                case TokenKind::Comma: {
                    return "`,`";
                }
                case TokenKind::Colon: {
                    return "`:`";
                }
                case TokenKind::LBrace: {
                    return "`{`";
                }
                case TokenKind::RBrace: {
                    return "`}`";
                }
                case TokenKind::LBracket: {
                    return "`[`";
                }
                case TokenKind::RBracket: {
                    return "`]`";
                }
                case TokenKind::Null: {
                    return "`null`";
                }
                case TokenKind::False: {
                    return "`false`";
                }
                case TokenKind::True: {
                    return "`true`";
                }
                case TokenKind::String: {
                    return "string '" + escstr(val) + "'";
                }
                case TokenKind::BinInt: {
                    return "number `0b" + val + "`";
                }
                case TokenKind::OctoInt: {
                    return "number `0o" + val + "`";
                }
                case TokenKind::HexInt: {
                    return "number `0x" + val + "`";
                }
                case TokenKind::DecInt: {
                    return "number `" + val + "`";
                }
                case TokenKind::Float: {
                    return "number `" + val + "`";
                }
            }
        }

        uint8_t intBase() const {
            switch (kind) {
                case TokenKind::DecInt: {
                    return 0;
                }
                case TokenKind::HexInt: {
                    return 16;
                }
                case TokenKind::OctoInt: {
                    return 8;
                }
                case TokenKind::BinInt: {
                    return 2;
                }
                default: {
                    throw std::logic_error("Called `Token::intBase` with non-int token");
                }
            }
        }
    };

    class Lexer {
    public:
        Lexer() = default;
        ~Lexer() = default;

        TokenStream lex(const std::string & source) {
            this->source = source;
            index = 0;
            lastNl = 0;
            col = 0;
            tokens.clear();

            while (not eof()) {
                tokenPos = index;
                lexCurrent();
            }

            tokenPos = index;
            addToken(TokenKind::Eof, 0);

            return std::move(tokens);
        }

    private:
        std::string source;

        size_t index{0};
        size_t lastNl{0};
        uint16_t col{0};
        Span::pos_t tokenPos;

        char peek() {
            return source.at(index);
        }

        char advance(uint8_t dist = 1) {
            auto cur = peek();
            for (uint8_t i = 0; i < dist; i++) {
                if (isNL()) {
                    lastNl = index;
                    col = 0;
                } else {
                    col++;
                }
                index++;
            }
            if (eof()) {
                return '\0';
            }
            return cur;
        }

        char lookup(uint8_t dist = 1) {
            return index < source.size() - dist ? source.at(index + dist) : '\0';
        }

        bool eof() {
            return index >= source.size();
        }

        bool isNL() {
            // TODO: Handle '\r' and '\r\n'
            return peek() == '\n';
        }

        template<class ...Args>
        bool isSeq(Args && ...chars) {
            uint8_t offset{0};
            return (... and (lookup(offset++) == chars));
        }

        bool is(char c) {
            return peek() == c;
        }

        template<class ...Args>
        bool isCharAnyOf(char c, Args && ...chars) {
            return (... or (c == chars));
        }

        bool isNextNonContinue(uint8_t offset) {
            return eof() or isCharAnyOf(lookup(offset), ',', ':', '{', '}', '[', ']', '\'', '"', ' ', '\n');
        }

        template<class ...Args>
        bool isAnyOf(Args && ...chars) {
            return isCharAnyOf(peek(), std::forward<Args>(chars)...);
        }

        bool isHidden() {
            return isAnyOf(' ', '\t', '\r');
        }

        bool isDigit() {
            return isDigit(peek());
        }

        bool isDigit(char c) {
            return c >= '0' and c <= '9';
        }

        bool isHexDigit() {
            return isDigit()
                or peek() >= 'a' and peek() <= 'f'
                or peek() >= 'A' and peek() <= 'F';
        }

        bool isOctDigit() {
            return peek() >= '0' and peek() <= '7';
        }

        void skip(char c) {
            if (peek() != c) {
                expectedError("`" + mstr(c) + "`");
            }
            advance();
        }

        bool skipOpt(char c) {
            if (is(c)) {
                advance();
                return true;
            }
            return false;
        }

        void lexCurrent() {
            switch (peek()) {
                case '/': {
                    return lexComment();
                }
                case '\'':
                case '"': {
                    return lexString();
                }
                case ',': {
                    addTokenAdvance(TokenKind::Comma, 1);
                    break;
                }
                case ':': {
                    addTokenAdvance(TokenKind::Colon, 1);
                    break;
                }
                case '{': {
                    addTokenAdvance(TokenKind::LBrace, 1);
                    break;
                }
                case '}': {
                    addTokenAdvance(TokenKind::RBrace, 1);
                    break;
                }
                case '[': {
                    addTokenAdvance(TokenKind::LBracket, 1);
                    break;
                }
                case ']': {
                    addTokenAdvance(TokenKind::RBracket, 1);
                    break;
                }
                default: {
                    return lexMisc();
                }
            }
        }

        void lexComment() {
            if (peek() != '/') {
                throw std::logic_error("Called `Lexer::lexComment` with not the '/' char");
            }
            if (lookup() == '*') {
                advance(2);

                // Parse block comment handling nested
                uint8_t depth{1};
                while (not eof()) {
                    if (peek() == '/' and lookup() == '*') {
                        depth++;
                    }

                    if (peek() == '*' and lookup() == '/') {
                        depth--;
                    }

                    if (depth == 0) {
                        break;
                    }
                }
            } else if (lookup() == '/') {
                while (not eof()) {
                    advance();
                    if (isNL()) {
                        break;
                    }
                }
            }
        }

        void lexString() {
            const auto quote = peek();
            if (isSeq(quote, quote, quote)) {
                return lexMLString();
            }

            return lexNormalString();
        }

        void lexMLString() {
            const auto quote = peek();
            // Note: Skip triple quote
            advance(3);

            bool closed = false;
            std::string val;
            while (not eof()) {
                if (isSeq(quote, quote, quote)) {
                    closed = true;
                    break;
                }
                val += advance();
            }

            if (!closed) {
                expectedError(mstr(quote, quote, quote));
            }

            advance(3);

            addToken(TokenKind::String, val);
        }

        void lexNormalString() {
            const auto quote = advance();

            std::string val;
            while (not eof()) {
                if (isNL() or is(quote)) {
                    break;
                }
                val += advance();
            }

            skip(quote);

            addToken(TokenKind::String, val);
        }

        void lexNum() {
            TokenKind kind;
            std::string val;

            bool baseSpecific = false;

            bool sign = false;
            if (is('+')) {
                advance();
            } else if (is('-')) {
                val += '-';
                advance();
                sign = true;
            }

            // Binary //
            if (peek() == '0' and (lookup() == 'b' or lookup() == 'B')) {
                if (sign) {
                    error("Signed binary numbers are not allowed");
                }

                baseSpecific = true;

                advance(2);
                if (not isAnyOf('0', '1')) {
                    expectedError("binary digit");
                }
                while (not eof()) {
                    skipOpt('_');
                    if (not isAnyOf('0', '1')) {
                        break;
                    }
                    val += advance();
                }

                kind = TokenKind::OctoInt;
            }

            // Hexadecimal //
            if (peek() == '0' and (lookup() == 'x' or lookup() == 'X')) {
                if (sign) {
                    error("Signed hexadecimal numbers are not allowed");
                }

                baseSpecific = true;

                advance(2);
                if (not isHexDigit()) {
                    expectedError("hexadecimal digit");
                }
                while (not eof()) {
                    skipOpt('_');
                    if (not isHexDigit()) {
                        break;
                    }
                    val += advance();
                }

                kind = TokenKind::HexInt;
            }

            // Octal //
            if (peek() == '0' and (lookup() == 'o' or lookup() == 'O')) {
                if (sign) {
                    error("Signed octal numbers are not allowed");
                }

                baseSpecific = true;

                advance(2);
                if (not isOctDigit()) {
                    expectedError("octal digit");
                }
                while (not eof()) {
                    skipOpt('_');
                    if (not isOctDigit()) {
                        break;
                    }
                    val += advance();
                }

                kind = TokenKind::OctoInt;
            }

            if (not baseSpecific) {
                while (not eof()) {
                    skipOpt('_');
                    if (not isDigit()) {
                        break;
                    }
                    val += advance();
                }

                kind = TokenKind::DecInt;

                if (is('.')) {
                    val += advance();
                    if (not isDigit()) {
                        expectedError("fractional part of number");
                    }
                    while (not eof()) {
                        skipOpt('_');
                        if (not isDigit()) {
                            break;
                        }
                        val += advance();
                    }
                    kind = TokenKind::Float;
                }
            }

            addToken(kind, val);
        }

        void lexMisc() {
            if (isNL()) {
                addTokenAdvance(TokenKind::NL, 1);
                return;
            }

            if (isHidden()) {
                advance();
                return;
            }

            if (isDigit() or (is('-') or is('+')) and isDigit(lookup())) {
                return lexNum();
            }

            // Identifier is anything not containing specific tokens
            std::string val;
            while (not eof()) {
                if (isAnyOf(',', ':', '{', '}', '[', ']', '\'', '"') or isNL()) {
                    break;
                }
                val += advance();
            }

            // Left spaces are already skipped by `isHidden`, thus trim right side of string to check for constant
            const auto & trimmed = rtrim(val);
            if (trimmed == "null") {
                addToken(TokenKind::Null, 4);
            } else if (trimmed == "false") {
                addToken(TokenKind::False, 5);
            } else if (trimmed == "true") {
                addToken(TokenKind::False, 4);
            } else if (trimmed == "nan") {
                addToken(TokenKind::NaN, 3);
            } else if (trimmed == "inf") {
                addToken(TokenKind::Inf, 3);
            } else if (trimmed == "-inf") {
                addToken(TokenKind::NegInf, 3);
            } else {
                // Add identifier as string
                addToken(TokenKind::String, std::move(val));
            }
       }

        // Tokens //
        TokenStream tokens;

        void addToken(TokenKind kind, const std::string & val) {
            tokens.emplace_back(kind, val, Span {tokenPos, val.size()});
        }

        void addToken(TokenKind kind, Span::len_t len) {
            tokens.emplace_back(kind, "", Span {tokenPos, len});
        }

        void addTokenAdvance(TokenKind kind, uint8_t len) {
            advance(len);
            tokens.emplace_back(kind, "", Span {tokenPos, len});
        }

        // Errors //
        void unexpectedToken() {
            error(mstr("Unexpected token '", peek(), "'"));
        }

        void expectedError(const std::string & expected) {
            std::string got;
            if (isNL()) {
                got = "new line";
            } else {
                got = mstr("`", peek(), "`");
            }

            error(mstr("Expected ", expected, ", got ", got));
        }

        void error(const std::string & msg) {
            size_t sliceTo = index;
            while (not eof()) {
                sliceTo = index;
                if (isNL()) {
                    break;
                }
                advance();
            }

            const auto & line = source.substr(lastNl, sliceTo - lastNl);
            std::string pointLine;
            if (msg.size() + 2 < col) {
                pointLine = std::string(col - msg.size() - 1, ' ') + msg + " ^";
            } else {
                pointLine = std::string(col, ' ') + "^ " + msg;
            }

            throw parse_error(
                mstr("\n", line, "\n", pointLine)
            );
        }
    };
}

#endif // JON_LEXER_H
