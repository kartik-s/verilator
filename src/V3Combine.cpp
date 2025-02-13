// -*- mode: C++; c-file-style: "cc-mode" -*-
//*************************************************************************
// DESCRIPTION: Verilator: Combine common code into functions
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
// V3Combine's Transformations:
//
//      Combine identical CFuncs by retaining only a single copy
//      Also drop empty CFuncs
//*************************************************************************

#include "config_build.h"
#include "verilatedos.h"

#include "V3Global.h"
#include "V3Combine.h"
#include "V3Hashed.h"
#include "V3Stats.h"
#include "V3Ast.h"

#include <algorithm>
#include <map>
#include <vector>

//######################################################################

class CombBaseVisitor VL_NOT_FINAL : public AstNVisitor {
protected:
    // STATE

    // METHODS
    virtual ~CombBaseVisitor() override = default;
    VL_DEBUG_FUNC;  // Declare debug()
};

//######################################################################
// Combine replacement function

class CombCallVisitor final : CombBaseVisitor {
    // Find all CCALLS of each CFUNC, so that we can later rename them
private:
    // NODE STATE
    std::multimap<AstCFunc*, AstCCall*> m_callMmap;  // Associative array of {function}{call}
    // METHODS
public:
    void replaceFunc(AstCFunc* oldfuncp, AstCFunc* newfuncp) {
        if (oldfuncp == newfuncp) return;
        if (newfuncp) {
            UINFO(4, "   Replace " << oldfuncp << " -WITH-> " << newfuncp << endl);
        } else {
            UINFO(4, "   Remove " << oldfuncp << endl);
        }
        // Note: m_callMmap modified in loop, so not using equal_range.
        for (auto it = m_callMmap.find(oldfuncp); it != m_callMmap.end();
             it = m_callMmap.find(oldfuncp)) {
            AstCCall* const oldp = it->second;
            UINFO(4, "     Called " << oldp << endl);
            UASSERT_OBJ(oldp->funcp() == oldfuncp, oldp,
                        "Call list broken, points to call w/different func");
            if (newfuncp) {
                // Replace call to oldfuncp with call to newfuncp
                AstNode* const argsp
                    = oldp->argsp() ? oldp->argsp()->unlinkFrBackWithNext() : nullptr;
                AstCCall* const newp = new AstCCall(oldp->fileline(), newfuncp, argsp);
                newp->hiernameToProt(oldp->hiernameToProt());
                newp->hiernameToUnprot(oldp->hiernameToUnprot());
                newp->argTypes(oldp->argTypes());
                addCall(newp);  // Fix the table, in case the newfuncp itself gets replaced
                oldp->replaceWith(newp);
            } else {
                // Just deleting empty function
                oldp->unlinkFrBack();
            }
            VL_DO_DANGLING(pushDeletep(oldp), oldp);
            m_callMmap.erase(it);  // Fix the table, This call has been replaced
        }
    }
    // METHODS
    void addCall(AstCCall* nodep) {
        if (nodep->funcp()->dontCombine()) return;
        m_callMmap.emplace(nodep->funcp(), nodep);
    }

private:
    // VISITORS
    virtual void visit(AstCCall* nodep) override { addCall(nodep); }
    // Speed things up
    virtual void visit(AstNodeAssign*) override {}
    virtual void visit(AstNodeMath*) override {}
    virtual void visit(AstNode* nodep) override { iterateChildren(nodep); }

public:
    // CONSTRUCTORS
    CombCallVisitor() = default;
    virtual ~CombCallVisitor() override = default;
    void main(AstNetlist* nodep) { iterate(nodep); }
};

//######################################################################
// Combine state, as a visitor of each AstNode

class CombineVisitor final : CombBaseVisitor {
private:
    // NODE STATE
    // Entire netlist:
    AstUser3InUse m_user3InUse;  // Marks replaced AstCFuncs
    //  AstUser4InUse     part of V3Hashed

    // STATE
    VDouble0 m_cfuncsCombined;  // Statistic tracking
    CombCallVisitor m_call;  // Tracking of function call users
    V3Hashed m_hashed;  // Hash for every CFunc in module

    // METHODS
    void walkEmptyFuncs() {
        for (const auto& itr : m_hashed) {
            AstCFunc* const oldfuncp = VN_CAST(itr.second, CFunc);
            UASSERT_OBJ(oldfuncp, itr.second, "Not a CFunc in hash");
            if (!oldfuncp->emptyBody()) continue;
            UASSERT_OBJ(!oldfuncp->dontCombine(), oldfuncp,
                        "dontCombine function should not be in hash");

            // Remove calls to empty function
            UASSERT_OBJ(!oldfuncp->user3(), oldfuncp, "Should not be processed yet");
            UINFO(5, "     Drop empty CFunc " << std::hex << V3Hash(oldfuncp->user4p()) << " "
                                              << oldfuncp << endl);
            oldfuncp->user3SetOnce();  // Mark replaced
            m_call.replaceFunc(oldfuncp, nullptr);
            oldfuncp->unlinkFrBack();
            VL_DO_DANGLING(pushDeletep(oldfuncp), oldfuncp);
        }
    }

    void walkDupFuncs() {
        // Do non-slow first as then favors naming functions based on fast name
        for (const bool slow : {false, true}) {
            for (auto newIt = m_hashed.begin(); newIt != m_hashed.end(); ++newIt) {
                AstCFunc* const newfuncp = VN_CAST(newIt->second, CFunc);
                UASSERT_OBJ(newfuncp, newIt->second, "Not a CFunc in hash");
                if (newfuncp->user3()) continue;  // Already replaced
                if (newfuncp->slow() != slow) continue;
                auto oldIt = newIt;
                ++oldIt;  // Skip over current position
                for (; oldIt != m_hashed.end(); ++oldIt) {
                    AstCFunc* const oldfuncp = VN_CAST(oldIt->second, CFunc);
                    UASSERT_OBJ(oldfuncp, oldIt->second, "Not a CFunc in hash");
                    UASSERT_OBJ(newfuncp != oldfuncp, newfuncp,
                                "Same function hashed multiple times");
                    if (newIt->first != oldIt->first) break;  // Iterate over same hashes only
                    if (oldfuncp->user3()) continue;  // Already replaced
                    if (!newfuncp->sameTree(oldfuncp)) continue;  // Different functions

                    // Replace calls to oldfuncp with calls to newfuncp
                    UINFO(5, "     Replace CFunc " << std::hex << V3Hash(newfuncp->user4p()) << " "
                                                   << newfuncp << endl);
                    UINFO(5, "              with " << std::hex << V3Hash(oldfuncp->user4p()) << " "
                                                   << oldfuncp << endl);
                    ++m_cfuncsCombined;
                    oldfuncp->user3SetOnce();  // Mark replaced
                    m_call.replaceFunc(oldfuncp, newfuncp);
                    oldfuncp->unlinkFrBack();
                    // Replacement may promote a slow routine to fast path
                    if (!oldfuncp->slow()) newfuncp->slow(false);
                    VL_DO_DANGLING(pushDeletep(oldfuncp), oldfuncp);
                }
            }
        }
    }

    // VISITORS
    virtual void visit(AstNetlist* nodep) override {
        m_call.main(nodep);  // Track all call sites of each function
        iterateChildren(nodep);
    }
    virtual void visit(AstNodeModule* nodep) override {
        UINFO(4, " MOD   " << nodep << endl);
        m_hashed.clear();
        // Compute hash of all CFuncs in the module
        iterateChildren(nodep);
        if (debug() >= 9) m_hashed.dumpFilePrefixed("combine");
        // Walk the hashes removing empty functions
        walkEmptyFuncs();
        // Walk the hashes looking for duplicate functions
        walkDupFuncs();
    }
    virtual void visit(AstCFunc* nodep) override {
        if (nodep->dontCombine()) return;
        // Hash the entire function
        m_hashed.hashAndInsert(nodep);
    }

    //--------------------
    // Default: Just iterate
    virtual void visit(AstVar*) override {}  // Accelerate
    virtual void visit(AstNodeStmt* nodep) override {}  // Accelerate
    virtual void visit(AstNode* nodep) override { iterateChildren(nodep); }

public:
    // CONSTRUCTORS
    explicit CombineVisitor(AstNetlist* nodep) { iterate(nodep); }
    virtual ~CombineVisitor() override {
        V3Stats::addStat("Optimizations, Combined CFuncs", m_cfuncsCombined);
    }
};

//######################################################################
// Combine class functions

void V3Combine::combineAll(AstNetlist* nodep) {
    UINFO(2, __FUNCTION__ << ": " << endl);
    { CombineVisitor visitor(nodep); }  // Destruct before checking
    V3Global::dumpCheckGlobalTree("combine", 0, v3Global.opt.dumpTreeLevel(__FILE__) >= 3);
}
