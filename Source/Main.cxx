/*
 * Source/Main.cxx
 *
 * This file is part of the tinyVM source code.
 * Copyright 2023 Patrick Melo <patrick@patrickmelo.com.br>
 */

#include "Compiler.hxx"
#include "Config.hxx"

int Run(const tinyVM::string programPath) {
    int returnCode = 1;

    tinyVM::VirtualMachine* tinyVM      = new tinyVM::VirtualMachine();
    tinyVM::Program*        tinyProgram = new tinyVM::Program();

    if (tinyProgram->Load(programPath))
        if (tinyVM->Start(tinyProgram))
            returnCode = 0;

    delete tinyProgram;
    delete tinyVM;

    return 1;
}

int Compile(const tinyVM::string sourcePath, const tinyVM::string binaryPath) {
    int returnCode = 1;

    tinyVM::Compiler*       tinyCompiler = new tinyVM::Compiler();
    tinyVM::VirtualMachine* tinyVM       = new tinyVM::VirtualMachine();

    if (tinyCompiler->Load(sourcePath))
        if (tinyCompiler->Compile(tinyVM))
            if (tinyCompiler->Save(binaryPath))
                returnCode = 0;

    delete tinyCompiler;
    delete tinyVM;

    return returnCode;
}

void PrintUsage(const tinyVM::string programPath) {
    Info("To run a program:");
    Info("  %s <program file path>", programPath.c_str());
    Info("");
    Info("To compile a program:");
    Info("  %s <source file path> <binary file path>", programPath.c_str());
    Info("");
}

int main(int numberOfArguments, char** argumentsValues) {
    setbuf(stdout, NULL);

    Info("");
    Info("%s - Version %s (%s %s)", tinyVM::Name, tinyVM::VersionString, OSName, ArchName);
    Info("");

    switch (numberOfArguments) {
        case 2: return Run(argumentsValues[1]);
        case 3: return Compile(argumentsValues[1], argumentsValues[2]);
        default: PrintUsage(argumentsValues[0]); break;
    }

    return 0;
}
