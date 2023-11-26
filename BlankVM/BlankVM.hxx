/*
 * BlankVM/BlankVM.hxx
 *
 * This file is part of the tinyVM source code.
 * Copyright 2023 Patrick Melo <patrick@patrickmelo.com.br>
 */

#ifndef VM_BLANK_VM_H
#define VM_BLANK_VM_H

#include "VirtualMachine.hxx"

namespace tinyVM {

// BlankVM

class BlankVM : public VirtualMachineCore {
    public:
        BlankVM(void);
};

}    // namespace tinyVM

#endif    // VM_BLANK_VM_H
