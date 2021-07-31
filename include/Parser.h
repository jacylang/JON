#ifndef JON_PARSER_H
#define JON_PARSER_H

#include "Lexer.h"
#include "ast.h"

namespace jon {
    class Parser {
    public:
        Parser() {}
        ~Parser() = default;

        void parse() {

        }

    private:
        TokenStream tokens;
        size_t index;

        Token peek() const {
            return tokens.at(index);
        }

        Token advance() {
            return tokens.at(++index);
        }

        bool eof() const {
            return index >= tokens.size();
        }

        bool is(TokenKind kind) const {
            return peek().kind == kind;
        }

        value_ptr parseValue() {
            switch (peek().kind) {
                case TokenKind::LBrace: {

                    break;
                }
                case TokenKind::LBracket: {

                    break;
                }
                case TokenKind::String: {

                    break;
                }
            }
        }
    };
}

#endif // JON_PARSER_H
