// -*- mode: C++; c-file-style: "cc-mode" -*-
//=============================================================================
//
// Code available from: https://verilator.org
//
// Copyright 2001-2021 by Wilson Snyder. This program is free software; you
// can redistribute it and/or modify it under the terms of either the GNU
// Lesser General Public License Version 3 or the Perl Artistic License
// Version 2.0.
// SPDX-License-Identifier: LGPL-3.0-only OR Artistic-2.0
//
//=============================================================================
///
/// \file
/// \brief Verilated tracing in VCD format header
///
/// User wrapper code should use this header when creating VCD traces.
///
//=============================================================================

#ifndef VERILATOR_VERILATED_VCD_LUAJIT_H_
#define VERILATOR_VERILATED_VCD_LUAJIT_H_

#ifndef VL_IN_LUAJIT_FFI_MODULE
#include "verilatedos.h"

extern "C" {
#endif

#ifdef VL_IN_LUAJIT_FFI_MODULE
struct VerilatedVcdC {
    char unused[0];
};
#endif

void* VerilatedVcdC_new();
void VerilatedVcdC_delete(void* p);
void VerilatedVcdC_open(void* p, const char* filename);
void VerilatedVcdC_dump(void* p, vluint64_t timeui);
void VerilatedVcdC_flush(void* p);
void VerilatedVcdC_close(void* p);

#ifndef VL_IN_LUAJIT_FFI_MODULE
}
#endif

#endif  // guard
