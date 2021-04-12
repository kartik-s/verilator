// -*- mode: C++; c-file-style: "cc-mode" -*-
//*************************************************************************
// DESCRIPTION: Verilator: Emit LuaJIT bindings for top-level module
//
// Code available from: https://verilator.org
//
//*************************************************************************
//
// Copyright 2003-2021 by Wilson Snyder. This program is free software; you
// can redistribute it and/or modify it under the terms of either the GNU
// Lesser General Public License Version 3 or the Perl Artistic License
// Version 2.0.
// SPDX-License-Identifier: LGPL-3.0-only OR Artistic-2.0
//
//*************************************************************************

#include "V3Ast.h"
#include "V3File.h"
#include "V3Global.h"
#include "V3EmitC.h"
#include "V3EmitCBase.h"
#include "V3EmitLuajit.h"

class EmitLuajit final {
public:
    void emitModule(AstNodeModule* modp) {
        string prefix = v3Global.opt.prefix();
        string filenamePrefix = v3Global.opt.makeDir() + "/" + prefix;
        V3OutLuaFile* m_ofp = new V3OutLuaFile(filenamePrefix + "_pre.lua");

        m_ofp->putsHeader();
        m_ofp->puts("-- DESCR"
                    "IPTION: Verilator output: LuaJIT FFI module");
        m_ofp->puts("--\n");

        m_ofp->puts("local ffi = require(\"ffi\")\n");

        m_ofp->puts("ffi.cdef[[\n");
        V3OutCFile* embed = new V3OutCFile{m_ofp};
        embed->puts("#include <verilated_luajit.h>\n");
        embed->puts("struct " + prefix + " {\n");
        embed->puts("void *padding;\n");
        V3EmitC::emitModPorts(modp, embed);
        embed->puts("};\n");
        embed->puts("void *" + prefix + "_new(void);\n");
        embed->puts("void " + prefix + "_delete(void *p);\n");
        embed->puts("void " + prefix + "_eval(void *p);\n");
        embed->puts("void " + prefix + "_eval_step(void *p);\n");
        embed->puts("void " + prefix + "_eval_end_step(void *p);\n");
        embed->puts("void " + prefix + "_final(void *p);\n");
        embed->puts("bool " + prefix + "_got_finish(void *p);\n");
        if (v3Global.opt.trace()) {
            embed->puts("void " + prefix + "_trace(void *p, void *tfp, int levels);\n");
            embed->puts("#include \"verilated_vcd_luajit.h\"\n");
            // embed->puts("struct VerilatedVcdC {\n");
            // embed->puts("char unused[0];\n");
            // embed->puts("};\n");
        }
        m_ofp->puts("]]\n");

        m_ofp->puts("local lib = ffi.load(\"" + prefix + "_luajit_shim\")\n");

        m_ofp->puts("return {\n");
        m_ofp->puts("ffi.metatype(\"struct " + prefix + "\", {\n");
        m_ofp->puts("__index = {\n");
        m_ofp->puts("new = function()"
                    " return ffi.cast(\"struct "
                    + prefix + " *\", lib." + prefix
                    + "_new()) "
                      "end,\n");
        m_ofp->puts("__gc = function(p) p:final(); lib." + prefix + "_delete(p) end,\n");
        m_ofp->puts("eval = lib." + prefix + "_eval,\n");
        m_ofp->puts("eval_step = lib." + prefix + "_eval_step,\n");
        m_ofp->puts("eval_end_step = lib." + prefix + "_eval_end_step,\n");
        m_ofp->puts("final = lib." + prefix + "_final,\n");
        m_ofp->puts("got_finish = lib." + prefix + "_got_finish,\n");

        if (v3Global.opt.trace()) { m_ofp->puts("trace = lib." + prefix + "_trace,\n"); }
        m_ofp->puts("}\n");
        m_ofp->puts("})");

        m_ofp->puts(", ffi.metatype(\"struct VerilatedContext\", {\n");
        m_ofp->puts("__index = {\n");
        m_ofp->puts("new = function()"
                    " return ffi.cast(\"struct VerilatedContext *\", lib.VerilatedContext_new()) "
                    "end,\n");
        m_ofp->puts("__gc = lib.VerilatedContext_delete,\n");
        m_ofp->puts("command_args = lib.VerilatedContext_commandArgs,\n");
        m_ofp->puts("got_finish = lib.VerilatedContext_gotFinish,\n");
        m_ofp->puts("trace_ever_on = lib.VerilatedContext_traceEverOn,\n");
        m_ofp->puts("}\n");
        m_ofp->puts("})");

        if (v3Global.opt.trace()) {
            m_ofp->puts(", ffi.metatype(\"struct VerilatedVcdC\", {\n");
            m_ofp->puts("__index = {\n");
            m_ofp->puts("new = function()"
                        " return ffi.cast(\"struct VerilatedVcdC *\", lib.VerilatedVcdC_new()) "
                        "end,\n");
            m_ofp->puts("__gc = function(p) p:close(); lib.VerilatedVcdC_delete(p) end,\n");
            m_ofp->puts("open = lib.VerilatedVcdC_open,\n");
            m_ofp->puts("dump = lib.VerilatedVcdC_dump,\n");
            m_ofp->puts("flush = lib.VerilatedVcdC_flush,\n");
            m_ofp->puts("close = lib.VerilatedVcdC_close,\n");
            m_ofp->puts("}\n");
            m_ofp->puts("})");
        }

        m_ofp->puts("\n}\n");
        m_ofp->putsHeader();
        VL_DO_CLEAR(delete m_ofp, m_ofp = nullptr);
    }

    void emitShim(AstNodeModule* modp) {
        string prefix = v3Global.opt.prefix();
        string filenamePrefix = v3Global.opt.makeDir() + "/lib" + prefix;
        V3OutCFile* m_ofp = new V3OutCFile(filenamePrefix + "_luajit_shim.cpp");

        m_ofp->putsHeader();
        m_ofp->puts("// DESCR"
                    "IPTION: Verilator output: LuaJIT shim library\n");
        m_ofp->puts("//\n");

        m_ofp->puts("#include \"" + EmitCBaseVisitor::symClassName() + ".h\"\n");
        m_ofp->puts("#include \"" + EmitCBaseVisitor::prefixNameProtect(modp) + ".h\"\n");
        m_ofp->puts("#include \"" + prefix + ".h\"\n");

        m_ofp->puts("extern \"C\" {\n");

        // Emit constructor
        m_ofp->puts("void *" + prefix + "_new() {\n");
        m_ofp->puts("return reinterpret_cast<void *>((new " + prefix
                    + ")->rootp);\n");
        m_ofp->puts("}\n");

        // Emit destructor
        m_ofp->puts("void " + prefix + "_delete(void *p) {\n");
        m_ofp->puts("delete reinterpret_cast<" + EmitCBaseVisitor::prefixNameProtect(modp) + "*>(p)->vlSymsp->__Vm_modelp;\n");
        m_ofp->puts("}\n");

        // Emit eval
        m_ofp->puts("void " + prefix + "_eval(void *p) {\n");
        m_ofp->puts("return reinterpret_cast<" + EmitCBaseVisitor::prefixNameProtect(modp) + "*>(p)->vlSymsp->__Vm_modelp->eval();\n");
        m_ofp->puts("}\n");

        // Emit eval_step
        m_ofp->puts("void " + prefix + "_eval_step(void *p) {\n");
        m_ofp->puts("return reinterpret_cast<" + EmitCBaseVisitor::prefixNameProtect(modp) + "*>(p)->vlSymsp->__Vm_modelp->eval_step();\n");
        m_ofp->puts("}\n");

        // Emit eval_end_step
        m_ofp->puts("void " + prefix + "_eval_end_step(void *p) {\n");
        m_ofp->puts("return reinterpret_cast<" + EmitCBaseVisitor::prefixNameProtect(modp) + "*>(p)->vlSymsp->__Vm_modelp->eval_end_step();\n");
        m_ofp->puts("}\n");

        // Emit final
        m_ofp->puts("void " + prefix + "_final(void *p) {\n");
        m_ofp->puts("return reinterpret_cast<" + EmitCBaseVisitor::prefixNameProtect(modp) + "*>(p)->vlSymsp->__Vm_modelp->final();\n");
        m_ofp->puts("}\n");

        // Emit got_finish
        m_ofp->puts("bool " + prefix + "_got_finish(void *p) {\n");
        m_ofp->puts("return reinterpret_cast<" + EmitCBaseVisitor::prefixNameProtect(modp) + "*>(p)->vlSymsp->__Vm_modelp->contextp()->gotFinish();\n");
        m_ofp->puts("}\n");

        // Emit trace
        if (v3Global.opt.trace()) {
            m_ofp->puts("void " + prefix + "_trace(void *p, void *tfp, int levels) {\n");
            m_ofp->puts(prefix + "* top = reinterpret_cast<" + prefix + "*>(p);\n");
            m_ofp->puts("top->contextp()->traceEverOn(true);\n");
            m_ofp->puts("top->trace(reinterpret_cast<VerilatedVcdC*>(tfp), levels);\n");
            m_ofp->puts("}\n");
        }

        m_ofp->puts("}");

        m_ofp->puts("\n");
        m_ofp->putsHeader();
        VL_DO_CLEAR(delete m_ofp, m_ofp = nullptr);
    }

    explicit EmitLuajit() {
        for (AstNodeModule* nodep = v3Global.rootp()->modulesp(); nodep;
             nodep = VN_CAST(nodep->nextp(), NodeModule)) {
            if (nodep->isTop()) {
                emitShim(nodep);
                emitModule(nodep);
                break;
            }
        }
    }

    virtual ~EmitLuajit() = default;
};

void V3EmitLuajit::emitBindings() {
    UINFO(2, __FUNCTION__ << ": " << endl);
    EmitLuajit emitter;
}
