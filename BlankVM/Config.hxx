/*
 * BlankVM/Config.hxx
 *
 * This file is part of the tinyVM source code.
 * Copyright 2023 Patrick Melo <patrick@patrickmelo.com.br>
 */

#ifndef VM_CONFIG_H
#define VM_CONFIG_H

#include "BlankVM.hxx"

namespace tinyVM {

const charconst Name          = "BlankVM";
const uint      Version       = 0x00000001;
const charconst VersionString = "0.1";
const charconst CopyrightInfo = "Copyright 2023 Patrick Melo <patrick@patrickmelo.com.br>";

typedef BlankVM VirtualMachine;

};    // namespace tinyVM

#endif    // VM_CONFIG_H
