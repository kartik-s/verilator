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
/// \brief Verilated C++ tracing in VCD format implementation code
///
/// This file must be compiled and linked against all Verilated objects
/// that use --trace.
///
/// Use "verilator --trace" to add this to the Makefile for the linker.
///
//=============================================================================

#include "verilated.h"
#include "verilated_vcd_c.h"
#include "verilated_vcd_luajit.h"
#include "verilatedos.h"

// clang-format off

void* VerilatedVcdC_new() {
    return reinterpret_cast<void*>(new VerilatedVcdC);
}

void VerilatedVcdC_new(void* p) {
    delete reinterpret_cast<VerilatedVcdC*>(p);
}

void VerilatedVcdC_open(void* p, const char* filename) {
    reinterpret_cast<VerilatedVcdC*>(p)->open(filename);
}

void VerilatedVcdC_dump(void* p, vluint64_t timeui) {
    reinterpret_cast<VerilatedVcdC*>(p)->dump(timeui);
}

void VerilatedVcdC_flush(void* p) {
    reinterpret_cast<VerilatedVcdC*>(p)->flush();
}

void VerilatedVcdC_close(void* p) {
    reinterpret_cast<VerilatedVcdC*>(p)->close();
}

// clang-format on

//********************************************************************
// ;compile-command: "v4make test_regress/t/t_trace_c_api.pl"
//
// Local Variables:
// End:
