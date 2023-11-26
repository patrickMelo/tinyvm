/*
 * BlankVM/BlankVM.cxx
 *
 * This file is part of the tinyVM source code.
 * Copyright 2023 Patrick Melo <patrick@patrickmelo.com.br>
 */

#include "BlankVM.hxx"

namespace tinyVM {

// BlankVM

BlankVM::BlankVM(void) :
    VirtualMachineCore() {
    this->BuildOperationsList();
}

}    // namespace tinyVM
