/*
 * Source/Compiler.cxx
 *
 * This file is part of the tinyVM source code.
 * Copyright 2023 Patrick Melo <patrick@patrickmelo.com.br>
 */

#include "Compiler.hxx"

#include "Core.hxx"
#include "Parser.hxx"
#include "Program.hxx"
#include "VirtualMachine.hxx"

namespace tinyVM {

// Compiler

Compiler::Compiler(void) :
    parser(new Parser()),
    program(new Program()),
    hostMachine(NULL),
    operationCounter(0) {
    // Empty
}

Compiler::~Compiler() {
    delete this->parser;
    delete this->program;
}

// General

bool Compiler::Load(const string sourceFilePath) {
    return this->parser->Load(sourceFilePath);
}

bool Compiler::Compile(VirtualMachineCore* hostMachine) {
    this->operationCounter = 0;
    this->hostMachine      = hostMachine;
    this->labels.clear();
    this->program->New();

    // First pass: create the labels map and count the total number of operations.

    Debug("");
    Debug("Doing first pass...");
    Debug("");

    this->parser->Reset();

    while (this->parser->GetNextToken(this->currentToken)) {
        // The first token we find must always be an identifier (the operation mnemonic) or a label.

        switch (currentToken.type) {
            case Parser::Token::Identifier: {
                this->operationCounter++;

                // Continue reading until we reach a new line or the end of the file.

                Parser::DeleteToken(this->currentToken);

                while (this->parser->GetNextToken(this->currentToken)) {
                    if (currentToken.type == Parser::Token::NewLine) {
                        Parser::DeleteToken(this->currentToken);
                        break;
                    }

                    Parser::DeleteToken(this->currentToken);
                }

                break;
            }

            case Parser::Token::Label: {
                if (!this->CompileLabel())
                    return false;

                break;
            }

            case Parser::Token::NewLine: {
                Parser::DeleteToken(this->currentToken);
                break;
            }

            default: {
                Error("Line %d: operation identifier or label expected, but \"%s\" was found.", currentToken.line, this->parser->TokenValueToString(currentToken).c_str());
                return false;
            }
        }
    }

    // Second pass: compile the operations.

    Debug("");
    Debug("Doing second pass...");
    Debug("");

    this->parser->Reset();

    while (this->parser->GetNextToken(this->currentToken)) {
        switch (this->currentToken.type) {
            case Parser::Token::Identifier: {
                if (!this->CompileOperation())
                    return false;

                break;
            }

            case Parser::Token::NewLine: {
                break;
            }

            default: {
                Parser::DeleteToken(this->currentToken);

                // Keep reading until we reach a new line or the end of the file.

                while (this->parser->GetNextToken(this->currentToken)) {
                    if (currentToken.type == Parser::Token::NewLine) {
                        Parser::DeleteToken(this->currentToken);
                        break;
                    }

                    Parser::DeleteToken(this->currentToken);
                }

                break;
            }
        }
    }

    Info("Program compiled successfully.");
    return true;
}

bool Compiler::Save(const string binaryFilePath) {
    return this->program->Save(binaryFilePath);
}

bool Compiler::CompileOperation(void) {
    Debug("Compiling operation \"%s\"...", this->currentToken.value->asString);

    int                                         parameterIndex = 0;
    VirtualMachineCore::OperationParameterTypes parameterTypes;
    Program::InstructionParameters              instructionParameters;

    memset(parameterTypes, 0, sizeof(parameterTypes));
    memset(instructionParameters, 0, sizeof(instructionParameters));

    int64  line     = this->currentToken.line;
    string mnemonic = this->currentToken.value->asString;
    Parser::DeleteToken(this->currentToken);

    // Get the operation parameters and try to find an operation that matches
    // the mnemonic and the parameter types.

    while (this->parser->GetNextToken(this->currentToken)) {
        if (this->currentToken.type == Parser::Token::NewLine) {
            Parser::DeleteToken(this->currentToken);
            Program::DeleteParameters(instructionParameters);
            break;
        }

        if (parameterIndex == 3) {
            Error("Line %ld: too many parameters.", line);
            Parser::DeleteToken(this->currentToken);
            Program::DeleteParameters(instructionParameters);
            return false;
        }

        switch (this->currentToken.type) {
            case Parser::Token::Identifier: {
                parameterTypes[parameterIndex++]      = VirtualMachineCore::Identifier;
                instructionParameters[parameterIndex] = NewStringValue(this->currentToken.value->asString);
                break;
            }

            case Parser::Token::Label: {
                // Find the label address and use it.

                auto foundLabel = this->labels.find(this->currentToken.value->asString);

                if (foundLabel == this->labels.end()) {
                    Error("Line %ld: label !%s not found.", line, this->currentToken.value->asString);
                    Parser::DeleteToken(this->currentToken);
                    Program::DeleteParameters(instructionParameters);
                    return false;
                }

                parameterTypes[parameterIndex++]      = VirtualMachineCore::Address;
                instructionParameters[parameterIndex] = NewIntValue(foundLabel->second);

                break;
            }

            case Parser::Token::Address: {
                if (this->currentToken.value->asInt >= this->operationCounter) {
                    Error("Line %ld: address @%ld out of range.", line, this->currentToken.value->asInt);
                    Parser::DeleteToken(this->currentToken);
                    Program::DeleteParameters(instructionParameters);
                    return false;
                }

                parameterTypes[parameterIndex++]      = VirtualMachineCore::Address;
                instructionParameters[parameterIndex] = NewIntValue(this->currentToken.value->asInt);

                break;
            }

            case Parser::Token::IntLiteral: {
                parameterTypes[parameterIndex++]      = VirtualMachineCore::IntLiteral;
                instructionParameters[parameterIndex] = NewIntValue(this->currentToken.value->asInt);
                break;
            }

            case Parser::Token::BoolLiteral: {
                parameterTypes[parameterIndex++]      = VirtualMachineCore::BoolLiteral;
                instructionParameters[parameterIndex] = NewBoolValue(this->currentToken.value->asBool);
                break;
            }

            case Parser::Token::FloatLiteral: {
                parameterTypes[parameterIndex++]      = VirtualMachineCore::FloatLiteral;
                instructionParameters[parameterIndex] = NewFloatValue(this->currentToken.value->asFloat);
                break;
            }

            case Parser::Token::StringLiteral: {
                parameterTypes[parameterIndex++]      = VirtualMachineCore::StringLiteral;
                instructionParameters[parameterIndex] = NewStringValue(this->currentToken.value->asString);
                break;
            }

            case Parser::Token::ArgumentSeparator: {
                Error("Line %ld: parameter expected, but parameter separator found.", line);
                Parser::DeleteToken(this->currentToken);
                Program::DeleteParameters(instructionParameters);
                return false;
            }

            default: {
                Error("Line %ld: unexpected token type.", line);
                Parser::DeleteToken(this->currentToken);
                Program::DeleteParameters(instructionParameters);
                return false;
            }
        }

        Parser::DeleteToken(this->currentToken);

        // After any parameter type we must have a parameter separator, a new line or the end of the file.

        if (this->parser->GetNextToken(this->currentToken)) {
            if ((this->currentToken.type != Parser::Token::ArgumentSeparator) && (this->currentToken.type != Parser::Token::NewLine)) {
                Error("Line %ld: parameter separator or new line expected, but \"%s\" was found.", line, this->parser->TokenValueToString(this->currentToken).c_str());
                Parser::DeleteToken(this->currentToken);
                Program::DeleteParameters(instructionParameters);
                return false;
            }

            Parser::DeleteToken(this->currentToken);
        }
    }

    // Try to find and operation the matches the specified mnemonic and the parameter types.

    int  numberOfParameters = parameterIndex;
    bool operationFound     = false;

    Debug("Parameters: %d.", numberOfParameters);

    const VirtualMachineCore::OperationList& vmOperations = this->hostMachine->GetOperations();

    for (auto currentOperation = vmOperations.begin(); currentOperation != vmOperations.end(); currentOperation++) {
        if (currentOperation->mnemonic != mnemonic)
            continue;

        operationFound = true;

        for (parameterIndex = 0; parameterIndex < numberOfParameters; ++parameterIndex)
            if (parameterTypes[parameterIndex] != currentOperation->parameterTypes[parameterIndex]) {
                operationFound = false;
                break;
            }

        if (operationFound) {
            Debug("Operation found with opcode %d.", currentOperation->opCode);
            this->program->Emit(currentOperation->opCode, instructionParameters);
            break;
        }
    }

    Program::DeleteParameters(instructionParameters);

    if (!operationFound)
        Error("Line %d: unkown operation (%s) or could not find one that matches the specified parameters.", line, mnemonic.c_str());

    return operationFound;
}

// Labels

bool Compiler::CompileLabel(void) {
    auto foundLabel = this->labels.find(this->currentToken.value->asString);

    if (foundLabel != this->labels.end()) {
        Error("Line %d: label !%s redeclared.", this->currentToken.line, this->currentToken.value->asString);
        Parser::DeleteToken(this->currentToken);
        return false;
    }

    Debug("Label !%s at operation %d.", this->currentToken.value->asString, this->operationCounter + 1);
    this->labels[this->currentToken.value->asString] = this->operationCounter + 1;
    Parser::DeleteToken(this->currentToken);

    // There must be a new line after the label declaration.

    if (this->parser->GetNextToken(this->currentToken)) {
        if (this->currentToken.type != Parser::Token::NewLine) {
            Error("Line %d: a label declaration must be followed by a new line.", this->currentToken.line);
            Parser::DeleteToken(this->currentToken);
            return false;
        }

        Parser::DeleteToken(this->currentToken);
    }

    return true;
}

};    // namespace tinyVM
