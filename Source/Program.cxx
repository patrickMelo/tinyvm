/*
 * Source/Program.cxx
 *
 * This file is part of the tinyVM source code.
 * Copyright 2023 Patrick Melo <patrick@patrickmelo.com.br>
 */

#include "Program.hxx"

namespace tinyVM {

// Program

Program::Program(void) :
    canEmit(false),
    code(NULL),
    data(NULL),
    strings(NULL) {
    // Empty
}

Program::~Program() {
    this->Delete();
}

// General

bool Program::New(void) {
    // Make sure we do not leave any memory used.

    this->Delete();

    this->code    = AllocateMemory(Program::MemoryBlockSize);
    this->data    = AllocateMemory(Program::MemoryBlockSize);
    this->strings = AllocateMemory(Program::MemoryBlockSize);

    if ((!this->code) || (!this->data) || (!this->strings)) {
        Error("Could not allocate memory to hold the new program.");

        DeleteMemory(this->code);
        DeleteMemory(this->data);
        DeleteMemory(this->strings);

        return false;
    }

    this->canEmit = true;

    Debug("New program created.");
    return true;
}

bool Program::Save(const string filePath) {
    if (!this->code)
        return false;

    FILE* file = fopen(filePath.c_str(), "wb");

    if (!file) {
        Error("Could not create the program file in \"%s\"", filePath.c_str());
        return false;
    }

    // Write the program header.

    // ID (4)
    // Version (4)
    // Code Size (8)
    // Data Size (8)
    // String Index Size (8)

    buffer programHeader = NewBuffer(32);

    if (!programHeader) {
        Error("Could not allocate memory to hold the file header.");
        fclose(file);
        return false;
    }

    memcpy(programHeader, Program::Signature, 4);
    memcpy(&programHeader[4], &Program::Version, 4);
    memcpy(&programHeader[8], &this->code->index, 8);
    memcpy(&programHeader[16], &this->data->index, 8);
    memcpy(&programHeader[24], &this->strings->index, 8);

    if (fwrite(programHeader, 32, 1, file) != 1) {
        Error("Could not write the program header to \"%s\"", filePath.c_str());
        DeleteBuffer(programHeader);
        fclose(file);
        return false;
    }

    DeleteBuffer(programHeader);

    // Write the program code.

    if (this->code->index > 0)
        if (fwrite(this->code->data, this->code->index, 1, file) != 1) {
            Error("Could not write the program code to \"%s\"", filePath.c_str());
            fclose(file);
            return false;
        }

    // Write the program data.

    if (this->data->index > 0)
        if (fwrite(this->data->data, this->data->index, 1, file) != 1) {
            Error("Could not write the program data to \"%s\"", filePath.c_str());
            fclose(file);
            return false;
        }

    // Write the program string index.

    if (this->strings->index > 0)
        if (fwrite(this->strings->data, this->strings->index, 1, file) != 1) {
            Error("Could not write the program string index to \"%s\"", filePath.c_str());
            fclose(file);
            return false;
        }

    fclose(file);
    Info("Program saved to \"%s\"", filePath.c_str());
    return true;
}

bool Program::Load(const string filePath) {
    // Make sure we do not leave any memory in use.

    this->Delete();

    // Try to open the program file.

    FILE* file = fopen(filePath.c_str(), "rb");

    if (!file) {
        Error("Could not open the program file \"%s\"", filePath.c_str());
        return false;
    }

    // Read the program header.

    // ID (4)
    // Version (4)
    // Code Size (8)
    // Data Size (8)
    // String Index Size (8)

    buffer programHeader = NewBuffer(32);

    if (fread(programHeader, 32, 1, file) != 1) {
        Error("Could not read the program header from \"%s\"", filePath.c_str());
        DeleteBuffer(programHeader);
        fclose(file);
        return false;
    }

    // Check the program signature and version.

    if (memcmp(Program::Signature, programHeader, 4) != 0) {
        Error("The program signature is invalid.");
        DeleteBuffer(programHeader);
        fclose(file);
        return false;
    }

    int32 version = Program::Version;

    if (memcmp(&version, &programHeader[4], 4) != 0) {
        Error("Unsupported program version.");
        DeleteBuffer(programHeader);
        fclose(file);
        return false;
    }

    // Read the program blocks sizes.

    int64 codeSize, dataSize, stringsSize;

    memcpy(&codeSize, &programHeader[8], 8);
    memcpy(&dataSize, &programHeader[16], 8);
    memcpy(&stringsSize, &programHeader[24], 8);

    DeleteBuffer(programHeader);

    Debug("Program blocks sizes: %ld, %ld, %ld", codeSize, dataSize, stringsSize);

    // Allocate the needed memory.

    this->code    = AllocateMemory((ceil(codeSize / Program::MemoryBlockSize) + 1) * Program::MemoryBlockSize);
    this->data    = AllocateMemory((ceil(dataSize / Program::MemoryBlockSize) + 1) * Program::MemoryBlockSize);
    this->strings = AllocateMemory((ceil(stringsSize / Program::MemoryBlockSize) + 1) * Program::MemoryBlockSize);

    if ((!this->code) || (!this->data) || (!this->strings)) {
        Error("Could not allocate memory to hold the program.");
        fclose(file);
        this->Delete();
        return false;
    }

    memset(this->code->data, 0, this->code->size);
    memset(this->data->data, 0, this->data->size);
    memset(this->strings->data, 0, this->strings->size);

    // Read the program code, data and strings.

    this->code->index    = codeSize;
    this->data->index    = dataSize;
    this->strings->index = stringsSize;

    if (this->code->index > 0)
        if (fread(this->code->data, this->code->index, 1, file) != 1) {
            Error("Could not read the program code from \"%s\"", filePath.c_str());
            fclose(file);
            this->Delete();
            return false;
        }

    if (this->data->index > 0)
        if (fread(this->data->data, this->data->index, 1, file) != 1) {
            Error("Could not read the program data from \"%s\"", filePath.c_str());
            fclose(file);
            return false;
        }

    if (this->strings->index > 0)
        if (fread(this->strings->data, this->strings->index, 1, file) != 1) {
            Error("Could not read the program string index from \"%s\"", filePath.c_str());
            fclose(file);
            return false;
        }

    fclose(file);

    Info("Program loaded from \"%s\"", filePath.c_str());
    return true;
}

void Program::Delete(void) {
    if (!this->code)
        return;

    DeleteMemory(this->code);
    DeleteMemory(this->data);
    DeleteMemory(this->strings);

    this->stringIndex.clear();
    this->canEmit = false;

    Debug("Program deleted.");
}

// Instructions

bool Program::Emit(const int64 opCode, const InstructionParameters parameters) {
    if ((!this->code) || (!this->canEmit))
        return false;

    // Expand the program memory if needed.

    if (this->code->size < this->code->index + 32)
        if (!ExpandMemory(this->code, this->code->size + Program::MemoryBlockSize)) {
            Error("Could not expand the program memory to hold the new code.");
            return false;
        }

    Debug("Emit %ld:", opCode);

    memcpy(&this->code->data[this->code->index], &opCode, 8);
    int64 parameterValue = 0;

    for (int parameterIndex = 0; parameterIndex < 4; ++parameterIndex) {
        parameterValue = 0;

        if (parameters[parameterIndex])
            switch (parameters[parameterIndex]->type) {
                case Value::Int: parameterValue = parameters[parameterIndex]->asInt; break;
                case Value::Float: memcpy(&parameterValue, &parameters[parameterIndex]->asFloat, 8); break;
                case Value::Bool: parameterValue = static_cast<int64>(parameters[parameterIndex]->asBool); break;
                case Value::String: parameterValue = this->GetStringIndex(parameters[parameterIndex]->asString); break;
            }

        memcpy(&this->code->data[this->code->index += 8], &parameterValue, 8);
        Debug("  P%d = %ld", parameterIndex, parameterValue);
    }

    this->code->index += 8;
    return true;
}

void Program::DeleteParameters(InstructionParameters& theParameters) {
    for (int parameterIndex = 0; parameterIndex < 4; ++parameterIndex)
        DeleteValue(theParameters[parameterIndex]);
}

// Strings

int64 Program::GetStringIndex(const string stringValue) {
    auto foundString = this->stringIndex.find(stringValue);

    // If the string is not in the index yet we have to save it and return its index.

    if (foundString == this->stringIndex.end()) {
        int64 stringStart = this->data->index;
        int64 stringSize  = stringValue.size();

        // Save the string data.

        int neededBlocks = 0;

        while (this->data->size + (neededBlocks * Program::MemoryBlockSize) < this->data->index + stringSize)
            neededBlocks++;

        if (neededBlocks > 0)
            if (!ExpandMemory(this->data, this->data->size + (Program::MemoryBlockSize * neededBlocks))) {
                Error("Could not expand the program memory to hold the new string data.");
                return false;
            }

        memcpy(&this->data->data[stringStart], stringValue.c_str(), stringSize);
        this->data->index += stringSize;

        // Save the string in the index.

        if (this->strings->size < this->strings->index + 16)
            if (!ExpandMemory(this->strings, this->strings->size + Program::MemoryBlockSize)) {
                Error("Could not expand the program memory to hold the new string index.");
                return false;
            }

        memcpy(&this->strings->data[this->strings->index], &stringStart, 8);
        memcpy(&this->strings->data[this->strings->index += 8], &stringSize, 8);
        this->strings->index += 8;

        return this->strings->index / 16;
    } else
        return foundString->second;
}

};    // namespace tinyVM
