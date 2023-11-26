/*
 * Source/Program.hxx
 *
 * This file is part of the tinyVM source code.
 * Copyright 2023 Patrick Melo <patrick@patrickmelo.com.br>
 */

#ifndef VM_PROGRAM_H
#define VM_PROGRAM_H

#include "Core.hxx"

namespace tinyVM {

// Program

class Program {
    public:
        Program(void);
        ~Program();

        // Constants

        static constexpr int32     Version         = 1;
        static constexpr charconst Signature       = "TVMP";
        static constexpr int       MemoryBlockSize = 8192;

        // General

        bool New(void);
        bool Save(const string filePath);
        bool Load(const string filePath);
        void Delete(void);

        // Instructions

        typedef val InstructionParameters[4];

        bool        Emit(const int64 opCode, const InstructionParameters parameters);
        static void DeleteParameters(InstructionParameters& parameters);

    private:
        // General

        bool canEmit;

        // Program Data

        memory code;       // [ OpCode, Param1, Param2, Param3, Param4 ]
        memory data;       // [ * ]
        memory strings;    // [ Start, Size ]

        // Strings

        std::map<string, int64> stringIndex;

        int64 GetStringIndex(const string stringValue);
};

};    // namespace tinyVM

#endif    // VM_PROGRAM_H
