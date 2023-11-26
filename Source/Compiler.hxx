/*
 * Source/Compiler.hxx
 *
 * This file is part of the tinyVM source code.
 * Copyright 2023 Patrick Melo <patrick@patrickmelo.com.br>
 */

#ifndef VM_COMPILER_H
#define VM_COMPILER_H

#include "Core.hxx"
#include "Parser.hxx"
#include "Program.hxx"
#include "VirtualMachine.hxx"

namespace tinyVM {

// Compiler

class Compiler {
    public:
        Compiler(void);
        ~Compiler();

        // General

        bool Load(const string sourceFilePath);
        bool Compile(VirtualMachineCore* hostMachine);
        bool Save(const string binaryFilePath);

    private:
        // General

        Parser*             parser;
        Program*            program;
        VirtualMachineCore* hostMachine;
        Parser::Token       currentToken;
        int64               operationCounter;

        bool CompileOperation(void);

        // Labels

        std::map<string, int64> labels;

        bool CompileLabel(void);
};

};    // namespace tinyVM

#endif    // VM_COMPILER_H
