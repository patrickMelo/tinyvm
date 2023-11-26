/*
 * Source/Parser.hxx
 *
 * This file is part of the tinyVM source code.
 * Copyright 2023 Patrick Melo <patrick@patrickmelo.com.br>
 */

#ifndef VM_PARSER_H
#define VM_PARSER_H

#include "Core.hxx"

namespace tinyVM {

// Parser

class Parser {
    public:
        Parser(void);
        ~Parser();

        // General

        bool Load(const string filePath);
        void Unload(void);
        void Reset(void);

        // Tokens

        struct Token {
                enum {
                    None,
                    Identifier,
                    Label,
                    Address,
                    IntLiteral,
                    BoolLiteral,
                    FloatLiteral,
                    StringLiteral,
                    ArgumentSeparator,
                    NewLine
                } type;

                val value;
                int line;
        };

        bool        GetNextToken(Token& token);
        string      TokenValueToString(Token& token);
        static void DeleteToken(Token& token);

    private:
        // Source

        memory sourceCode;
        int    lineNumber;

        // Tokens

        string GetNextTokenValue(void);
        bool   IsBoolean(const string value);
        bool   IsInt(const string value);
        bool   IsFloat(const string value);
};

};    // namespace tinyVM

#endif    // VM_PARSER_H
