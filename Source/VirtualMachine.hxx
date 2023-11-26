/*
 * Source/VirtualMachine.hxx
 *
 * This file is part of the tinyVM source code.
 * Copyright 2023 Patrick Melo <patrick@patrickmelo.com.br>
 */

#ifndef VM_VIRTUAL_MACHINE_H
#define VM_VIRTUAL_MACHINE_H

#include "Program.hxx"

namespace tinyVM {

// Virtual Machine Core

class VirtualMachineCore {
    public:
        VirtualMachineCore(void);
        virtual ~VirtualMachineCore();

        // Operations

        typedef char OperationMnemonic[8];

        enum OperationParameterType {
            None,
            Address,
            Identifier,
            IntLiteral,
            BoolLiteral,
            FloatLiteral,
            StringLiteral
        };

        typedef OperationParameterType OperationParameterTypes[4];
        typedef bool (VirtualMachineCore::*OperationMethod)(const Program::InstructionParameters parameters);

        struct Operation {
                int64                   opCode;
                OperationMnemonic       mnemonic;
                OperationMethod         method;
                OperationParameterTypes parameterTypes;
        };

        typedef std::vector<Operation> OperationList;

        bool                 RegisterOperation(const int64 opCode, const OperationMnemonic mnemonic, const OperationMethod method, const OperationParameterTypes parameterTypes);
        void                 BuildOperationsList(void);
        const OperationList& GetOperations(void) const;

        // Execution

        bool Start(Program* program);
        void Pause(void);
        bool Resume(void);
        bool Step(void);
        void Stop(void);
        bool IsRunning(void) const;
        bool IsPaused(void) const;

        // Bult-in Instructions

        bool OpNoOp(const Program::InstructionParameters parameters);
        bool OpExit(const Program::InstructionParameters parameters);
        bool OpPause(const Program::InstructionParameters parameters);
        bool OpStop(const Program::InstructionParameters parameters);

    private:
        // Operations

        std::map<int64, Operation> operationsMap;
        OperationList              operations;

        // Execution

        bool isRunning;
        bool isPaused;

        // Programs

        Program* currentProgram;
        int64    currentProgramInstruction;
};

}    // namespace tinyVM

#endif    // VM_VIRTUAL_MACHINE_H
