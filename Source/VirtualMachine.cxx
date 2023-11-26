/*
 * Source/VirtualMachine.cxx
 *
 * This file is part of the tinyVM source code.
 * Copyright 2023 Patrick Melo <patrick@patrickmelo.com.br>
 */

#include "VirtualMachine.hxx"

#include "Program.hxx"

namespace tinyVM {

// Virtual Machine

VirtualMachineCore::VirtualMachineCore(void) :
    isRunning(false),
    isPaused(false),
    currentProgram(NULL),
    currentProgramInstruction(0) {
    OperationParameterTypes noParameters = {None, None, None, None};

    this->RegisterOperation(0, "NOP", &VirtualMachineCore::OpNoOp, noParameters);
    this->RegisterOperation(1, "EXIT", &VirtualMachineCore::OpExit, noParameters);
    this->RegisterOperation(2, "PAUSE", &VirtualMachineCore::OpPause, noParameters);
    this->RegisterOperation(3, "STOP", &VirtualMachineCore::OpStop, noParameters);
}

VirtualMachineCore::~VirtualMachineCore() {
    this->Stop();
}

// Operations

bool VirtualMachineCore::RegisterOperation(const int64 opCode, const OperationMnemonic mnemonic, const OperationMethod method, const OperationParameterTypes parameterTypes) {
    auto foundOperation = this->operationsMap.find(opCode);

    if (foundOperation != this->operationsMap.end()) {
        Warning("Operation code %ld already in use by %s.", opCode, foundOperation->second.mnemonic);
        return false;
    }

    Operation newOperation = {
        opCode,
        "",
        method,
        {None, None, None, None}
    };

    memcpy(newOperation.mnemonic, mnemonic, sizeof(mnemonic));
    memcpy(newOperation.parameterTypes, parameterTypes, sizeof(parameterTypes));

    this->operationsMap[opCode] = newOperation;
    Debug("Operation %ld registered (%s).", opCode, mnemonic);

    return true;
}

void VirtualMachineCore::BuildOperationsList(void) {
    Debug("Building operations list...");

    this->operations.clear();

    // Find the maximum operation code used.

    int64 maxOpCode = 0;

    for (auto currentOperation = this->operationsMap.begin(); currentOperation != this->operationsMap.end(); ++currentOperation)
        if (currentOperation->first > maxOpCode)
            maxOpCode = currentOperation->first;

    Debug("Maximum operation code used: %ld", maxOpCode);

    // Fill the list with the registered operations.
    // If no operation is found for some operation code, use NOP.

    for (int64 currentOpCode = 0; currentOpCode <= maxOpCode; ++currentOpCode) {
        auto foundOperation = this->operationsMap.find(currentOpCode);
        this->operations.push_back(foundOperation != this->operationsMap.end() ? foundOperation->second : this->operationsMap[0]);
    }

    Debug("Operations list built. Operations supported: %ld.", this->operations.size());
}

const VirtualMachineCore::OperationList& VirtualMachineCore::GetOperations(void) const {
    return this->operations;
}

// Execution

bool VirtualMachineCore::Start(Program* program) {
    if (this->isRunning) {
        Warning("Cannot start a program while another one is running.");
        return false;
    }

    if (!program) {
        Error("The program is null.");
        return false;
    }

    this->currentProgramInstruction = 0;
    this->currentProgram            = program;

    this->isRunning = true;
    this->isPaused  = false;

    Info("Starting program execution...");
    return this->Resume();
}

void VirtualMachineCore::Pause(void) {
    this->isPaused = true;
}

bool VirtualMachineCore::Resume(void) {
    if (!this->currentProgram) {
        Error("No program to resume execution from.");
        return false;
    }

    if (!this->isRunning) {
        Warning("The program is not running. Cannot resume execution.");
        return false;
    }

    if (this->isPaused) {
        Info("Resuming program execution...");
        this->isPaused = false;
    }

    while (this->isRunning && (!this->isPaused)) {
        if (!this->Step())
            break;
    }

    Info("Program execution %s.", this->isPaused ? "paused" : "stopped");
    return true;
}

bool VirtualMachineCore::Step(void) {
    if (!this->isRunning)
        return false;

    // TODO: implement.
    Stub();

    return false;
}

void VirtualMachineCore::Stop(void) {
    this->isRunning = false;
}

bool VirtualMachineCore::IsRunning(void) const {
    return this->isRunning;
}

bool VirtualMachineCore::IsPaused(void) const {
    return this->isPaused;
}

// Bult-in Instructions

bool VirtualMachineCore::OpNoOp(const Program::InstructionParameters parameters) {
    return true;
}

bool VirtualMachineCore::OpExit(const Program::InstructionParameters parameters) {
    return false;
}

bool VirtualMachineCore::OpPause(const Program::InstructionParameters parameters) {
    this->Pause();
    return true;
}

bool VirtualMachineCore::OpStop(const Program::InstructionParameters parameters) {
    this->Stop();
    return true;
}

}    // namespace tinyVM
