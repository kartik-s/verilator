#include "verilated.h"
#include "verilated_luajit.h"

// clang-format off

void* VerilatedContext_new() {
    return reinterpret_cast<VerilatedContext*>(new VerilatedContext);
}

void VerilatedContext_delete(void* p) {
    delete reinterpret_cast<VerilatedContext*>(p);
}

void VerilatedContext_commandArgs(void* p, int argc, const char** argv) {
    reinterpret_cast<VerilatedContext*>(p)->commandArgs(argc, argv);
}

bool VerilatedContext_gotFinish(void* p) {
    return reinterpret_cast<VerilatedContext*>(p)->gotFinish();
}

void VerilatedContext_traceEverOn(void* p, bool flag) {
    reinterpret_cast<VerilatedContext*>(p)->traceEverOn(flag);
}

// clang-format on
