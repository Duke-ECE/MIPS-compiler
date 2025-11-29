#include "compiler/codegen.hpp"
#include <sstream>
#include <stdexcept>

CodeGen::CodeGen() {
}

int CodeGen::allocateRegister(const std::string &var) {
    if (regMap.count(var)) return regMap[var];
    int id = regMap.size() + 1;  // r1, r2, r3...
    regMap[var] = id;
    return id;
}

std::string CodeGen::regName(int id) const {
    return "$r" + std::to_string(id);
}

std::string CodeGen::labelName(const std::string &name) const {
    return name;
}

std::vector<std::string>
CodeGen::generateAssembly(const IRProgram &program) {
    std::vector<std::string> out;
    for (auto &inst : program.instructions)
        translate(inst, out);
    return out;
}

void CodeGen::translate(const IRInstruction &I,
                        std::vector<std::string> &out)
{
    const std::string &op = I.op;

    // --------------------
    // Label
    // --------------------
    if (op == "label") {
        out.push_back(I.dst + ":");
        return;
    }

    // --------------------
    // jmp label
    // --------------------
    if (op == "jmp") {
        out.push_back("j " + labelName(I.dst));
        return;
    }

    // --------------------
    // ret value → jr $value
    // --------------------
    if (op == "ret") {
        if (!I.dst.empty()) {
            int r = allocateRegister(I.dst);
            out.push_back("jr " + regName(r));
        } else {
            out.push_back("jr $r31");
        }
        return;
    }

    // --------------------
    // mov dst, src
    // immediate or register
    // using addi
    // --------------------
    if (op == "mov") {
        int rd = allocateRegister(I.dst);

        // immediate?
        bool isImm = true;
        for (char c : I.src1) {
            if (!isdigit(c) && c != '-') { isImm = false; break; }
        }

        if (isImm) {
            out.push_back("addi " + regName(rd) + ", $r0, " + I.src1);
        } else {
            int rs = allocateRegister(I.src1);
            out.push_back("addi " + regName(rd) + ", " + regName(rs) + ", 0");
        }
        return;
    }

    // --------------------
    // add / sub / and / or
    // --------------------
    if (op == "add" || op == "sub" ||
        op == "and" || op == "or")
    {
        int rd = allocateRegister(I.dst);
        int rs = allocateRegister(I.src1);
        int rt = allocateRegister(I.src2);
        out.push_back(op + " " + regName(rd) + ", "
                           + regName(rs) + ", "
                           + regName(rt));
        return;
    }

    // --------------------
    // mul / div → software expansion
    // --------------------
    if (op == "mul" || op == "div") {
        // We expand "mul" as repeated addition
        // And "div" as repeated subtraction
        // (simple but valid on your ISA)

        int rd = allocateRegister(I.dst);
        int a  = allocateRegister(I.src1);
        int b  = allocateRegister(I.src2);

        static int lbl = 0;
        std::string L_loop = "LMD_LOOP_" + std::to_string(lbl);
        std::string L_end  = "LMD_END_"  + std::to_string(lbl);
        lbl++;

        // clear rd = 0
        out.push_back("addi " + regName(rd) + ", $r0, 0");

        if (op == "mul") {
            // multiply rd = a * b
            out.push_back(L_loop + ":");
            out.push_back("beq " + regName(b) + ", $r0, " + L_end);
            out.push_back("add " + regName(rd) + ", " + regName(rd) + ", " + regName(a));
            out.push_back("addi " + regName(b) + ", " + regName(b) + ", -1");
            out.push_back("j " + L_loop);
            out.push_back(L_end + ":");
        } else {
            // divide rd = a / b (we count how many times b fits into a)
            out.push_back(L_loop + ":");
            out.push_back("bgt " + regName(b) + ", " + regName(a) + ", " + L_end);
            out.push_back("sub " + regName(a) + ", " + regName(a) + ", " + regName(b));
            out.push_back("addi " + regName(rd) + ", " + regName(rd) + ", 1");
            out.push_back("j " + L_loop);
            out.push_back(L_end + ":");
        }
        return;
    }

    // --------------------
    // Unary neg: x = -y → sub x, r0, y
    // (ISA has sub)
    // --------------------
    if (op == "neg") {
        int rd = allocateRegister(I.dst);
        int rs = allocateRegister(I.src1);
        out.push_back("sub " + regName(rd) + ", $r0, " + regName(rs));
        return;
    }

    // --------------------
    // Unary not: x = !y
    // --------------------
    if (op == "not") {
        int rd = allocateRegister(I.dst);
        int rs = allocateRegister(I.src1);

        static int lbl = 0;
        std::string L_true = "LNOT_T_" + std::to_string(lbl);
        std::string L_end  = "LNOT_E_" + std::to_string(lbl);
        lbl++;

        out.push_back("beq " + regName(rs) + ", $r0, " + L_true);
        out.push_back("addi " + regName(rd) + ", $r0, 0");
        out.push_back("j " + L_end);
        out.push_back(L_true + ":");
        out.push_back("addi " + regName(rd) + ", $r0, 1");
        out.push_back(L_end + ":");
        return;
    }

    // --------------------
    // Comparisons
    // IR: dst = lt a b
    // --------------------
    if (op == "lt" || op == "le" ||
        op == "gt" || op == "ge" ||
        op == "eq" || op == "ne")
    {
        int rd = allocateRegister(I.dst);
        int a  = allocateRegister(I.src1);
        int b  = allocateRegister(I.src2);

        static int lbl = 0;
        std::string L_true = "LCMP_T_" + std::to_string(lbl);
        std::string L_end  = "LCMP_E_" + std::to_string(lbl);
        lbl++;

        if (op == "lt") {
            out.push_back("bgt " + regName(b) + ", " + regName(a) + ", " + L_true);
        }
        else if (op == "gt") {
            out.push_back("bgt " + regName(a) + ", " + regName(b) + ", " + L_true);
        }
        else if (op == "eq") {
            out.push_back("beq " + regName(a) + ", " + regName(b) + ", " + L_true);
        }
        else if (op == "ne") {
            out.push_back("beq " + regName(a) + ", " + regName(b) + ", " + L_end);
        }
        else if (op == "le") {
            out.push_back("bgt " + regName(a) + ", " + regName(b) + ", " + L_end);
            out.push_back("j " + L_true);
        }
        else if (op == "ge") {
            out.push_back("bgt " + regName(b) + ", " + regName(a) + ", " + L_end);
            out.push_back("j " + L_true);
        }

        out.push_back("addi " + regName(rd) + ", $r0, 0");
        out.push_back("j " + L_end);
        out.push_back(L_true + ":");
        out.push_back("addi " + regName(rd) + ", $r0, 1");
        out.push_back(L_end + ":");
        return;
    }

    // --------------------
    // Call: dst = call f, nArgs
    // --------------------
    if (op == "call") {
        int rd = allocateRegister(I.dst);
        out.push_back("jal " + I.src1);
        out.push_back("addi " + regName(rd) + ", $r31, 0");
        return;
    }

    // --------------------
    // input / output
    // --------------------
    if (op == "input") {
        int rd = allocateRegister(I.dst);
        out.push_back("input " + regName(rd));
        return;
    }
    if (op == "output") {
        int rd = allocateRegister(I.dst);
        out.push_back("output " + regName(rd));
        return;
    }

    throw std::runtime_error("Unknown IR op: " + op);
}
