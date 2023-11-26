/*
 * Source/Parser.cxx
 *
 * This file is part of the tinyVM source code.
 * Copyright 2023 Patrick Melo <patrick@patrickmelo.com.br>
 */

#include "Parser.hxx"

namespace tinyVM {

// Parser

Parser::Parser(void) :
    sourceCode(NULL),
    lineNumber(0) {
    // Empty
}

Parser::~Parser() {
    this->Unload();
}

void Parser::Reset(void) {
    if (!this->sourceCode)
        return;

    this->sourceCode->index = 0;
    this->lineNumber        = 1;
}

// General

bool Parser::Load(const string filePath) {
    // Make sure we do not leave any memory used.

    this->Unload();

    FILE* file = fopen(filePath.c_str(), "rb");

    if (!file) {
        Error("Could not open the file \"%s\"", filePath.c_str());
        return false;
    }

    fseek(file, 0, SEEK_END);
    int fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    this->sourceCode = AllocateMemory(fileSize);

    if (!this->sourceCode) {
        Error("Could not allocate %ld bytes to read from \"%s\"", this->sourceCode->size, filePath.c_str());
        fclose(file);
        return false;
    }

    if (fread(this->sourceCode->data, this->sourceCode->size, 1, file) != 1) {
        Error("Could not read from \"%s\" (error %d)", filePath.c_str(), errno);
        DeleteMemory(this->sourceCode);
        fclose(file);
        return false;
    }

    fclose(file);
    this->Reset();

    Info("File \"%s\" loaded.", filePath.c_str());
    return true;
}

void Parser::Unload(void) {
    if (!this->sourceCode)
        return;

    this->lineNumber = 0;

    DeleteMemory(this->sourceCode);
    Debug("Source code unloaded.");
}

// Tokens

bool Parser::GetNextToken(Token& token) {
    token.type  = Token::None;
    token.value = NULL;
    token.line  = this->lineNumber;

    string value = this->GetNextTokenValue();

    // Did we reach the end of the file?

    if (value.empty())
        return false;

    // Parse the token value.

    if (value[0] == '"') {
        token.type  = Token::StringLiteral;
        token.value = NewStringValue(value.substr(1));
        Debug("TOKEN: STRING = %s", value.c_str());
    } else if (value[0] == '@') {
        token.type  = Token::Address;
        token.value = NewStringValue(value.substr(1));
        Debug("TOKEN: ADDRESS = %s", value.c_str());
    } else if (value[0] == '!') {
        token.type  = Token::Label;
        token.value = NewStringValue(value.substr(1));
        Debug("TOKEN: LABEL = %s", value.c_str());
    } else if ((value[0] == '\r') || (value[0] == '\n')) {
        token.type = Token::NewLine;
        this->lineNumber++;
        Debug("TOKEN: NEW LINE");
    } else if (value[0] == ',') {
        token.type = Token::ArgumentSeparator;
        Debug("TOKEN: ARGUMENT SEPARATOR");
    } else if (this->IsBoolean(value)) {
        token.type  = Token::BoolLiteral;
        token.value = NewBoolValue(value == "true");
        Debug("TOKEN: BOOLEAN = %s", value.c_str());
    } else if (this->IsInt(value)) {
        token.type  = Token::IntLiteral;
        token.value = NewIntValue(ToInt(value));
        Debug("TOKEN: INT = %s", value.c_str());
    } else if (this->IsFloat(value)) {
        token.type  = Token::FloatLiteral;
        token.value = NewFloatValue(ToFloat(value));
        Debug("TOKEN: FLOAT = %s", value.c_str());
    } else {
        token.type  = Token::Identifier;
        token.value = NewStringValue(value);
        Debug("TOKEN: IDENTIFIER = %s", value.c_str());
    }

    return true;
}

string Parser::GetNextTokenValue(void) {
    if (this->sourceCode->index >= this->sourceCode->size)
        return "";

    char currentChar = this->sourceCode->data[this->sourceCode->index++];

    // Ignore any initial control character or space (except for line endings).

    while ((currentChar <= ' ') && (this->sourceCode->index < this->sourceCode->size)) {
        if ((currentChar == '\r') || (currentChar == '\n'))
            break;

        currentChar = this->sourceCode->data[this->sourceCode->index++];
    }

    // Rewind one character to start from the first one that is not a control character or a space.

    this->sourceCode->index--;

    bool   inString = false;
    string value    = "";

    while (this->sourceCode->index < this->sourceCode->size) {
        currentChar = this->sourceCode->data[this->sourceCode->index++];

        if (currentChar == '"') {
            if (inString)
                break;

            // We keep the string separator so the value can be recognized as a string.

            value += currentChar;
            inString = true;
            continue;
        }

        // If we hit an escape character, no matter where we are at, we just get the next character and continue.

        if ((currentChar == '\\') and (this->sourceCode->index < this->sourceCode->size)) {
            value += this->sourceCode->data[this->sourceCode->index++];
            continue;
        }

        if (!inString) {
            // Break on any separator character.

            if (currentChar == ' ')
                break;

            if (currentChar == ',') {
                // If we hit a one-character only separator we must keep it.
                // Otherwise we rewind one character so the one-character separator
                // will be processed next time this method is called.

                if (value.empty())
                    value += currentChar;
                else
                    this->sourceCode->index--;

                break;
            }

            // If we hit a line end character we must check if it's a "separator" line end or the line end itself.

            if ((currentChar == '\r') || (currentChar == '\n')) {
                // If it is a "separator" line end, we rewind one character (so it will be processed
                // next time this method is called) and break.

                if (!value.empty()) {
                    this->sourceCode->index--;
                    break;
                }

                value += currentChar;

                // If it is a line end token we must test the line end type.
                // It may be a "CR+LF" (Windows) one.

                if ((currentChar == '\r') && (this->sourceCode->index < this->sourceCode->size))
                    if (this->sourceCode->data[this->sourceCode->index] == '\n')
                        value += this->sourceCode->data[this->sourceCode->index++];

                break;
            }
        }

        value += currentChar;
    }

    return value;
}

string Parser::TokenValueToString(Token& token) {
    switch (token.type) {
        case Token::Identifier: return token.value->asString;
        case Token::Label: return "!" + string(token.value->asString);
        case Token::Address: return "@" + string(token.value->asString);
        case Token::StringLiteral: return "\"" + string(token.value->asString) + "\"";
        case Token::IntLiteral: return FromInt(token.value->asInt);
        case Token::FloatLiteral: return FromFloat(token.value->asFloat);
        case Token::BoolLiteral: return FromBool(token.value->asBool);
        case Token::ArgumentSeparator: return ",";
        case Token::NewLine: return "new line";
        default: return "";
    }
}

void Parser::DeleteToken(Token& token) {
    DeleteValue(token.value);
}

bool Parser::IsBoolean(const string value) {
    return (value == "true") || (value == "false");
}

bool Parser::IsInt(const string value) {
    for (int charIndex = 0; charIndex < value.size(); charIndex++) {
        if ((charIndex == 0) && (value[charIndex] == '-'))
            continue;

        if ((value[charIndex] < '0') || (value[charIndex] > '9'))
            return false;
    }

    return true;
}

bool Parser::IsFloat(const string value) {
    bool dotFound = false;

    for (int charIndex = 0; charIndex < value.size(); charIndex++) {
        if ((charIndex == 0) && (value[charIndex] == '-'))
            continue;

        if (value[charIndex] == '.') {
            if (dotFound)
                return false;

            dotFound = true;
            continue;
        }

        if ((value[charIndex] < '0') || (value[charIndex] > '9'))
            return false;
    }

    return true;
}

};    // namespace tinyVM
