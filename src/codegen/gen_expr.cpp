#include <algorithm>
#include <string>

#include "../typeChecker/type.hpp"
#include "gen.hpp"
#include "optimizer/compiler.hpp"
#include "optimizer/instr.hpp"

void codegen::visitExpr(Node::Expr *expr) {
  // Optimize the expression before we handle it!
  Node::Expr *realExpr = CompileOptimizer::optimizeExpr(expr);
  if (int(realExpr->kind) > (int)NodeKind::ND_NULL) {  // im dumb ignore me i put
                                                       // the wrong sign :sob: ya
    std::cout << "stinky node D:"
              << std::endl;  // bp here if the nodekind is something weird
  }
  ExprHandler handler = lookup(exprHandlers, realExpr->kind);
  if (handler) {
    handler(realExpr);
  }
}

// A basic "fallback" type of expression that covers all 1-D, basic expr's
void codegen::primary(Node::Expr *expr) {
  // TODO: Implement the primary expression
  switch (expr->kind) {
    case NodeKind::ND_INT: {
      IntExpr *e = static_cast<IntExpr *>(expr);
      pushDebug(e->line, expr->file_id, e->pos);
      push(Instr{.var = PushInstr{.what = "$" + std::to_string(e->value),
                                  .whatSize = DataSize::None},  // THe only time None is used, because this literal could be literally anything
                 .type = InstrType::Push},
           Section::Main);
      break;
    }  // It happened here in the ident
    case NodeKind::ND_IDENT: {
      IdentExpr *e = static_cast<IdentExpr *>(expr);
      std::string res = variableTable[e->name];

      push(Instr{.var = Comment{.comment = "Retrieve identifier: '" + e->name +
                                           "' located at " + res},
                 .type = InstrType::Comment},
           Section::Main);

      pushDebug(e->line, expr->file_id, e->pos);

      // Push the result
      // Check for struct
      if (e->type->kind == ND_SYMBOL_TYPE) {
        if (structByteSizes.find(static_cast<SymbolType *>(e->asmType)->name) !=
            structByteSizes.end()) {
          push(Instr{.var = LeaInstr{.size = DataSize::Qword,
                                     .dest = "%rcx",
                                     .src = res},
                     .type = InstrType::Lea},
               Section::Main);
          pushRegister("%rcx");
          break;
        }
      } else if (e->type->kind == ND_ARRAY_TYPE) {
        push(Instr{.var = LeaInstr{.size = DataSize::Qword,
                                   .dest = "%rcx",
                                   .src = res},
                   .type = InstrType::Lea},
             Section::Main);
        pushRegister("%rcx");
        break;
      }
      // Get the byte size of the ident (this will determine the type of the push)
      DataSize finalSize = intDataToSize(getByteSizeOfType(e->asmType));
      push(Instr{.var = PushInstr{.what = res, .whatSize = finalSize},
                 .type = InstrType::Push},
           Section::Main);
      break;
    }
    case NodeKind::ND_STRING: {
      StringExpr *string = static_cast<StringExpr *>(expr);
      std::string label = "string" + std::to_string(stringCount++);

      pushDebug(string->line, expr->file_id, string->pos);
      // Push the label onto the stack
      push(Instr{.var = LeaInstr{.size = DataSize::Qword,
                                 .dest = "%r13",
                                 .src = label + "(%rip)"},
                 .type = InstrType::Lea},
           Section::Main);
      pushRegister("%r13");

      // define the string in the data section
      push(Instr{.var = Label{.name = label}, .type = InstrType::Label},
           Section::ReadonlyData);

      // Push the string onto the data section
      push(Instr{.var = AscizInstr{.what = string->value},
                 .type = InstrType::Asciz},
           Section::ReadonlyData);
      break;
    }
    case NodeKind::ND_CHAR: {
      CharExpr *charExpr = static_cast<CharExpr *>(expr);
      pushDebug(charExpr->line, expr->file_id, charExpr->pos);
      pushRegister("$" + std::to_string((int)charExpr->value));
      break;
    }
    case NodeKind::ND_FLOAT: {
      FloatExpr *floating = static_cast<FloatExpr *>(expr);
      std::string label = "float" + std::to_string(floatCount++);
      pushDebug(floating->line, expr->file_id, floating->pos);

      // Push the label onto the stack
      moveRegister("%xmm0", label + "(%rip)", DataSize::SS, DataSize::SS);
      push(Instr{.var = PushInstr{.what = "%xmm0", .whatSize = DataSize::SS},
                 .type = InstrType::Push},
           Section::Main);

      // define the string in the data section
      push(Instr{.var = Label{.name = label}, .type = InstrType::Label},
           Section::ReadonlyData);

      // Push the string onto the data section
      push(Instr{.var = Comment{.comment = floating->value},
                 .type = InstrType::Comment},
           Section::ReadonlyData);
      push(Instr{.var =
                     DataSectionInstr{
                         .bytesToDefine = DataSize::SS /* long, aka 32-bits */,
                         .what = floating->value},
                 .type = InstrType::DB},
           Section::ReadonlyData);
      break;
    }
    case NodeKind::ND_BOOL: {
      BoolExpr *boolExpr = static_cast<BoolExpr *>(expr);
      // Technically just an int but SHHHHHHHH.............................
      push(Instr{.var = PushInstr{.what = "$" + std::to_string(boolExpr->value),
                                  .whatSize = DataSize::Byte},  // A byte is so much more different than an int...
                 .type = InstrType::Push},
           Section::Main);
      break;
    }
    default: {
      std::cerr
          << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)"
          << std::endl;
      std::cerr << "Primary expression type not implemented! (Type: NodeKind["
                << (int)expr->kind << "])" << std::endl;
      exit(-1);
    }
  }
}

void codegen::binary(Node::Expr *expr) {
  BinaryExpr *e = static_cast<BinaryExpr *>(expr);
  pushDebug(e->line, expr->file_id, e->pos);
  SymbolType *returnType = static_cast<SymbolType *>(e->asmType);
  DataSize size = DataSize::Qword;
  std::string lhsReg = "";
  std::string rhsReg = "";
  switch (getByteSizeOfType(returnType)) {
    case 1:
      size = DataSize::Byte;
      lhsReg = "%al";
      rhsReg = "%bl";
      break;
    case 2:
      size = DataSize::Word;
      lhsReg = "%ax";
      rhsReg = "%bx";
      break;
    case 4:
      size = DataSize::Dword;
      lhsReg = "%eax";
      rhsReg = "%ebx";
      break;
    case 8:
    default:  // Not like i'll be dealing with 3-byte integers anyways
      size = DataSize::Qword;
      lhsReg = "%rax";
      rhsReg = "%rbx";
      break;
  }

  // TODO: Replace with "isIntBasedType" Function
  if (TypeChecker::isIntBasedType(returnType)) {
    int lhsDepth = getExpressionDepth(e->lhs);
    int rhsDepth = getExpressionDepth(e->rhs);
    if (lhsDepth > rhsDepth) {
      visitExpr(e->lhs);
      visitExpr(e->rhs);
      push(Instr{.var = PopInstr{.where = rhsReg, .whereSize = size}, .type = InstrType::Pop}, Section::Main);
      push(Instr{.var = PopInstr{.where = lhsReg, .whereSize = size}, .type = InstrType::Pop}, Section::Main);
    } else {
      visitExpr(e->rhs);
      visitExpr(e->lhs);
      push(Instr{.var = PopInstr{.where = lhsReg, .whereSize = size}, .type = InstrType::Pop}, Section::Main);
      push(Instr{.var = PopInstr{.where = rhsReg, .whereSize = size}, .type = InstrType::Pop}, Section::Main);
    }

    // Perform the operation
    std::string op = lookup(opMap, e->op);
    if (op == "idiv" || op == "div" || op == "mod") {
      // Division requires special handling because of RDX:RAX input (more precision or something)
      // We cannot check the result of the division, as a (neg / neg = pos) and that would ruin this
      bool isSignedOp = (static_cast<SymbolType *>(e->lhs->asmType))->signedness == SymbolType::Signedness::SIGNED ||
                        (static_cast<SymbolType *>(e->rhs->asmType))->signedness == SymbolType::Signedness::SIGNED;
      if (isSignedOp) {
        // Although C likes making really stupid optimizations, technically, it works without them.
        switch (size) {
          default:
          case DataSize::Qword:
            pushLinker("cqto\n\t", Section::Main);
            break;
          case DataSize::Dword:
            pushLinker("cltd\n\t", Section::Main);
            break;
          case DataSize::Word:
            pushLinker("movswl " + lhsReg + ", %eax", Section::Main);
            break;
          case DataSize::Byte:
            pushLinker("movsbw " + lhsReg + ", %ax", Section::Main);
            break;
        }
        push(Instr{.var = DivInstr{.from = rhsReg, .isSigned = true, .size = size}, .type = InstrType::Div}, Section::Main);
      } else {
        // it was unsigned so we have way less shit to worry about
        DataSize size2 = intDataToSize(getByteSizeOfType(returnType));
        push(Instr{.var = XorInstr{.lhs = "%rdi", .rhs = "%rdi"}, .type = InstrType::Xor}, Section::Main);
        push(Instr{.var = DivInstr{.from = rhsReg, .isSigned = false, .size = size2}, .type = InstrType::Div}, Section::Main);
      }

      if (op == "mod") {
        switch (getByteSizeOfType(returnType)) {
          case 1:
            push(Instr{.var = PushInstr{.what = "%dl", .whatSize = DataSize::Byte}, .type = InstrType::Push}, Section::Main);
            break;
          case 2:
            push(Instr{.var = PushInstr{.what = "%dx", .whatSize = DataSize::Word}, .type = InstrType::Push}, Section::Main);
            break;
          case 4:
            push(Instr{.var = PushInstr{.what = "%ebx", .whatSize = DataSize::Dword}, .type = InstrType::Push}, Section::Main);
            break;
          case 8:
          default:
            push(Instr{.var = PushInstr{.what = "%rbx", .whatSize = DataSize::Qword}, .type = InstrType::Push}, Section::Main);
            break;
        }
      } else {
        // It doesnt matter, push the real thing and get out of here
        push(Instr{.var = PushInstr{.what = lhsReg, .whatSize = size}, .type = InstrType::Push}, Section::Main);
      }
    } else if (op == "shl" || op == "shr" || op == "sar" || op == "sal") {
      // Shift operations require either the CL register or an immediate
      // Check if there is an immediate
      if (e->rhs->kind == ND_INT) {
        long long shiftAmount = static_cast<IntExpr *>(e->rhs)->value;
        if (shiftAmount == 1) {
          // This requires NO number 1
          pushLinker(op + " " + lhsReg + "\n\t", Section::Main);
          push(Instr{.var = PushInstr{.what = lhsReg, .whatSize = size}, .type = InstrType::Push}, Section::Main);
          return;
        }
        push(Instr{.var = BinaryInstr{.op = op, .src = "$" + std::to_string(shiftAmount), .dst = rhsReg},
                   .type = InstrType::Binary},
             Section::Main);
        // push the result
        push(Instr{.var = PushInstr{.what = rhsReg, .whatSize = size}, .type = InstrType::Push}, Section::Main);
      } else {
        // If there is no immediate, we need to pop the value into CL
        push(Instr{.var = MovInstr{.dest = "%cl", .src = rhsReg, .destSize = DataSize::Byte, .srcSize = size},
                   .type = InstrType::Mov},
             Section::Main);
        push(Instr{.var = BinaryInstr{.op = op, .src = "%cl", .dst = lhsReg},
                   .type = InstrType::Binary},
             Section::Main);
        // push the result
        push(Instr{.var = PushInstr{.what = lhsReg, .whatSize = size}, .type = InstrType::Push}, Section::Main);
      }
    } else {
      // Every other operation ...
      push(Instr{.var = BinaryInstr{.op = op, .src = rhsReg, .dst = lhsReg},
                 .type = InstrType::Binary},
           Section::Main);
      push(Instr{.var = PushInstr{.what = lhsReg, .whatSize = size}, .type = InstrType::Push}, Section::Main);
    }
    return;  // Done
  } else if (returnType->name == "float" || returnType->name == "double") {
    // Similar logic for floats
    size = returnType->name == "float" ? DataSize::SS : DataSize::SD;
    std::string suffix = size == DataSize::SS ? "ss" : "sd";
    int lhsDepth = getExpressionDepth(static_cast<BinaryExpr *>(e->lhs));
    int rhsDepth = getExpressionDepth(static_cast<BinaryExpr *>(e->rhs));
    if (lhsDepth > rhsDepth) {
      visitExpr(e->lhs);
      visitExpr(e->rhs);
      popToRegister("%xmm1");  // Pop RHS into XMM1
      popToRegister("%xmm0");  // Pop LHS into XMM0
    } else {
      visitExpr(e->rhs);
      visitExpr(e->lhs);
      popToRegister("%xmm0");  // Pop LHS into XMM0
      popToRegister("%xmm1");  // Pop RHS into XMM1
    }

    std::string op;
    if (e->op == "+")
      op = "add" + suffix;
    if (e->op == "-")
      op = "sub" + suffix;
    if (e->op == "*")
      op = "mul" + suffix;
    if (e->op == "/")
      op = "div" + suffix;

    // Doubles also work on the XMM registers so its ok

    push(Instr{.var = BinaryInstr{.op = op, .src = "%xmm1", .dst = "%xmm0"},
               .type = InstrType::Binary},
         Section::Main);
    push(Instr{.var = PushInstr{.what = "%xmm0", .whatSize = size}, .type = InstrType::Push}, Section::Main);
  } else if (returnType->name == "bool") {
    // We need to compare the two values
    // Check depth
    bool isFloating = e->lhs->asmType->kind == ND_SYMBOL_TYPE && (getUnderlying(e->lhs->asmType) == "float" || getUnderlying(e->lhs->asmType) == "double");
    // Right now, the lhsReg and rhsReg are %al and %bl because the return type is 1 byte. although technically correct, thats not what we want
    switch (std::max(getByteSizeOfType(e->rhs->asmType), getByteSizeOfType(e->lhs->asmType))) {  // use max because we may be comparing an integer literal to something else
      case 1:
        break;  // Its already like this, isn't it?
      case 2:
        lhsReg = "%ax";
        rhsReg = "%bx";
        size = DataSize::Word;
        break;
      case 4:
        lhsReg = "%eax";
        rhsReg = "%ebx";
        size = DataSize::Dword;
        break;
      case 8:
      default:
        lhsReg = "%rax";
        rhsReg = "%rbx";
        size = DataSize::Qword;
        break;
    }
    if (isFloating) {
      lhsReg = "%xmm0";
      rhsReg = "%xmm1";
      if (getByteSizeOfType(e->lhs->asmType) == 4) {
        size = DataSize::SS;
      } else {
        size = DataSize::SD;
      }
    }
    int lhsDepth = getExpressionDepth(e->lhs);
    int rhsDepth = getExpressionDepth(e->rhs);
    if (lhsDepth > rhsDepth) {
      visitExpr(e->lhs);
      visitExpr(e->rhs);
      push(Instr{.var = PopInstr{.where = rhsReg, .whereSize = size}, .type = InstrType::Pop}, Section::Main);
      push(Instr{.var = PopInstr{.where = lhsReg, .whereSize = size}, .type = InstrType::Pop}, Section::Main);
    } else {
      visitExpr(e->rhs);
      visitExpr(e->lhs);
      push(Instr{.var = PopInstr{.where = lhsReg, .whereSize = size}, .type = InstrType::Pop}, Section::Main);
      push(Instr{.var = PopInstr{.where = rhsReg, .whereSize = size}, .type = InstrType::Pop}, Section::Main);
    }
    // Get the operation
    std::string op = lookup(opMap, e->op);
    // Perform the operation
    // by running a comparison
    if (isFloating) {
      std::string letter = size == DataSize::SS ? "s" : "d";
      pushLinker("ucomis" + letter + " %xmm1, %xmm0\n\t", Section::Main);
    } else {
      push(Instr{.var = CmpInstr{.lhs = lhsReg, .rhs = rhsReg, .size = size},
                 .type = InstrType::Cmp},
           Section::Main);
    }
    // We must use a set instruction because %al doesnt have anything in it yet
    pushLinker(op + " %al\n\t", Section::Main);
    // Move the bytes up to 64-bits
    push(Instr{.var = PushInstr{.what = "%al", .whatSize = DataSize::Byte}, .type = InstrType::Push}, Section::Main);
  }
}

void codegen::unary(Node::Expr *expr) {
  UnaryExpr *e = static_cast<UnaryExpr *>(expr);
  pushDebug(e->line, expr->file_id, e->pos);
  visitExpr(e->expr);
  // Its gonna be a pop guys
  
  if (e->op == "-") {
    // Perform the operation
    DataSize size = intDataToSize(getByteSizeOfType(e->asmType));
    std::string reg = "%rax";
    switch (size) {
      case DataSize::Byte:
        reg = "%al";
        break;
      case DataSize::Word:
        reg = "%ax";
        break;
      case DataSize::Dword:
        reg = "%eax";
        break;
      case DataSize::Qword:
      default:
        reg = "%rax";
        break;
    }
    push(Instr{.var = PopInstr{.where = reg, .whereSize = size}, .type = InstrType::Pop}, Section::Main);
    push(Instr{.var = NegInstr{.what = reg, .size = size}, .type = InstrType::Neg}, Section::Main);
    push(Instr{.var = PushInstr{.what = reg, .whatSize = size}, .type = InstrType::Push}, Section::Main);
  } else if (e->op == "++" || e->op == "--") {
    // Perform the operation
    PushInstr instr = std::get<PushInstr>(text_section.at(text_section.size() - 1).var);
    std::string whatWasPushed = instr.what;
  
    text_section.pop_back();
    std::string res = (e->op == "++") ? "inc" : "dec";
    DataSize size = intDataToSize(getByteSizeOfType(e->asmType));
    std::string character = "";
    switch (size) {
      case DataSize::Byte:
        character = "b";
        break;
      case DataSize::Word:
        character = "w";
        break;
      case DataSize::Dword:
        character = "l";
        break;
      case DataSize::Qword:
      default:
        character = "q";
        break;
    }
    // Dear code reader, i apologize
    push(Instr{.var = LinkerDirective{.value = res + character + " " + whatWasPushed + "\n\t"}, .type = InstrType::Linker}, Section::Main);
    // Push the result
    pushRegister(whatWasPushed);
  }
}

void codegen::grouping(Node::Expr *expr) {
  GroupExpr *e = static_cast<GroupExpr *>(expr);
  // Visit the expression inside the grouping
  pushDebug(e->line, expr->file_id, e->pos);
  visitExpr(e->expr);
}

void codegen::call(Node::Expr *expr) {
  CallExpr *e = static_cast<CallExpr *>(expr);
  if (e->callee->kind == ND_IDENT) {
    IdentExpr *n = static_cast<IdentExpr *>(e->callee);
    pushDebug(e->line, expr->file_id, e->pos);
    // Push each argument one by one.
    if (e->args.size() > intArgOrder.size()) {
      std::cerr << "Too many arguments in call - consider reducing them or "
                   "moving them to a globally defined space."
                << std::endl;
      exit(-1);
    }
    long long offsetAmount = round(variableCount - 8, 8);
    if (offsetAmount)
      push(Instr{.var = SubInstr{.lhs = "%rsp",
                                 .rhs = "$" + std::to_string(offsetAmount),
                                 .size = DataSize::Qword},
                 .type = InstrType::Sub},
           Section::Main);
    int intArgCount = 0;
    int floatArgCount = 0;

    for (size_t i = 0; i < e->args.size(); i++) {
      // evaluate them
      visitExpr(e->args.at(i));
      // Check if the argument was a struct or array
      // That requires an lea, not a mov (that will be implicitly created here)
      bool isLea = false;
      if (e->args[i]->asmType->kind == ND_ARRAY_TYPE) {
        // lea should be false, because the value being processed is already
        // technically a pointer
      }
      if (e->args[i]->asmType->kind == ND_SYMBOL_TYPE) {
        SymbolType *sym = static_cast<SymbolType *>(e->args[i]->asmType);
        if (structByteSizes.find(sym->name) != structByteSizes.end()) {
          // FIRST we check the size of the struct type
          if (structByteSizes[sym->name].first > 16) {
            isLea = true;  // It was in there!
          }
        }
      }
      if (isLea) {
        // Get rid of the pushexpr
        PushInstr instr =
            std::get<PushInstr>(text_section.at(text_section.size() - 1).var);
        std::string whatWasPushed = instr.what;
        text_section.pop_back();
        // Now we can lea
        // ... IF what was pushed was an address ...
        if (whatWasPushed.find('(') == std::string::npos) {
          // run additional check to see if float or int
          if (getUnderlying(e->args[i]->asmType) == "float" &&
              e->args[i]->asmType->kind == ND_SYMBOL_TYPE) {
            popToRegister(floatArgOrder[floatArgCount++]);
          } else {
            popToRegister(intArgOrder[intArgCount++]);
          }
        } else
          push(Instr{.var = LeaInstr{.size = DataSize::Qword,
                                     .dest = intArgOrder[i],
                                     .src = whatWasPushed},
                     .type = InstrType::Lea},
               Section::Main);
      } else
        popToRegister(intArgOrder[i]);
    }
    // Call the function
    push(Instr{.var = CallInstr{.name = "usr_" + n->name},
               .type = InstrType::Call,
               .optimize = false},
         Section::Main);
    if (offsetAmount)
      push(Instr{.var = AddInstr{.lhs = "%rsp",
                                 .rhs = "$" + std::to_string(offsetAmount),
                                 .size = DataSize::Qword},
                 .type = InstrType::Sub},
           Section::Main);
    // What we push as the result depends on the return type of the function
    if (e->asmType->kind == ND_POINTER_TYPE ||
        e->asmType->kind == ND_ARRAY_TYPE ||
        e->asmType->kind == ND_FUNCTION_TYPE ||
        e->asmType->kind == ND_FUNCTION_TYPE_PARAM) {
      pushRegister("%rax");
    } else {
      SymbolType *st = static_cast<SymbolType *>(e->asmType);
      if (st->name == "float" || st->name == "double") {
        push(
            Instr{.var = PushInstr{.what = "%xmm0", .whatSize = DataSize::SS},
                  .type = InstrType::Push},
            Section::Main);  // abi standard (can hold many bytes of data, so its
                             // fine for both floats AND doubles to fit in here)
      } else if (structByteSizes.find(st->name) != structByteSizes.end()) {
        // TODO: No! This is really wrong!!!!
        pushRegister("%rax");  // When tossing this thing around, its handled
                               // basically as a pointer
      } else {
        // switch over the byte size of return type
        switch (getByteSizeOfType(e->asmType)) {
          case 1:
            push(
                Instr{.var = PushInstr{.what = "%al", .whatSize = DataSize::Byte},
                      .type = InstrType::Push},
                Section::Main);
            break;
          case 2:
            push(
                Instr{.var = PushInstr{.what = "%ax", .whatSize = DataSize::Word},
                      .type = InstrType::Push},
                Section::Main);
            break;
          case 4:
            push(Instr{.var =
                           PushInstr{.what = "%eax", .whatSize = DataSize::Dword},
                       .type = InstrType::Push},
                 Section::Main);
            break;
          case 8:
          default:
            push(Instr{.var =
                           PushInstr{.what = "%rax", .whatSize = DataSize::Qword},
                       .type = InstrType::Push},
                 Section::Main);
            break;
        }
      }
    }
  } else if (e->callee->kind == ND_MEMBER) {
    // Get the struct name
    MemberExpr *member = static_cast<MemberExpr *>(e->callee);
    std::string structName;
    if (member->lhs->kind == ND_INDEX) {
      structName =
          getUnderlying(static_cast<IndexExpr *>(member->lhs)->lhs->asmType);
    } else
      structName = getUnderlying(member->lhs->asmType);
    std::string fnName = static_cast<IdentExpr *>(member->rhs)->name;
    pushDebug(e->line, expr->file_id, e->pos);
    // Push each argument one by one.
    // Evaluate the struct
    visitExpr(member->lhs);
    // Note: Removed LEA Because when visiting a struct, we push %rcx, which alr
    // contains the address :P
    popToRegister(intArgOrder[0]);
    int intArgCount = 1;  // 1st is preserved for struct ptr above
    int floatArgCount = 0;
    long long offsetAmount = round(variableCount - 8, 8);
    if (offsetAmount)
      push(Instr{.var = SubInstr{.lhs = "%rsp",
                                 .rhs = "$" + std::to_string(offsetAmount),
                                 .size = DataSize::Qword},
                 .type = InstrType::Sub},
           Section::Main);
    for (size_t i = 0; i < e->args.size(); i++) {
      // evaluate them
      visitExpr(e->args.at(i));
      if (getUnderlying(e->args[i]->asmType) == "float" &&
          e->args[i]->asmType->kind == ND_SYMBOL_TYPE) {
        popToRegister(floatArgOrder[floatArgCount++]);
      } else {
        popToRegister(intArgOrder[intArgCount++]);
      }
    }
    // Call the function
    push(Instr{.var =
                   CallInstr{.name = "usrstruct_" + structName + "_" + fnName},
               .type = InstrType::Call,
               .optimize = false},
         Section::Main);
    if (offsetAmount)
      push(Instr{.var = AddInstr{.lhs = "%rsp",
                                 .rhs = "$" + std::to_string(offsetAmount),
                                 .size = DataSize::Qword},
                 .type = InstrType::Sub},
           Section::Main);
    if (e->asmType->kind == ND_POINTER_TYPE ||
        e->asmType->kind == ND_ARRAY_TYPE ||
        e->asmType->kind == ND_FUNCTION_TYPE ||
        e->asmType->kind == ND_FUNCTION_TYPE_PARAM) {
      pushRegister("%rax");
    } else {
      SymbolType *st = static_cast<SymbolType *>(e->asmType);
      if (st->name == "float" || st->name == "double") {
        push(
            Instr{.var = PushInstr{.what = "%xmm0", .whatSize = DataSize::SS},
                  .type = InstrType::Push},
            Section::Main);  // abi standard (can hold many bytes of data, so its
                             // fine for both floats AND doubles to fit in here)
      } else if (structByteSizes.find(st->name) != structByteSizes.end()) {
        // TODO: No! This is really wrong!!!!
        pushRegister("%rax");  // When tossing this thing around, its handled
                               // basically as a pointer
      } else {
        // switch over the byte size of return type
        switch (getByteSizeOfType(e->asmType)) {
          case 1:
            push(
                Instr{.var = PushInstr{.what = "%al", .whatSize = DataSize::Byte},
                      .type = InstrType::Push},
                Section::Main);
            break;
          case 2:
            push(
                Instr{.var = PushInstr{.what = "%ax", .whatSize = DataSize::Word},
                      .type = InstrType::Push},
                Section::Main);
            break;
          case 4:
            push(Instr{.var =
                           PushInstr{.what = "%eax", .whatSize = DataSize::Dword},
                       .type = InstrType::Push},
                 Section::Main);
            break;
          case 8:
          default:
            push(Instr{.var =
                           PushInstr{.what = "%rax", .whatSize = DataSize::Qword},
                       .type = InstrType::Push},
                 Section::Main);
            break;
        }
      }
    }
  }
}

// TODO: FIX this to evalueate correctly
void codegen::ternary(Node::Expr *expr) {
  TernaryExpr *e = static_cast<TernaryExpr *>(expr);

  int ternay = 0;
  std::string ternayCount = std::to_string(ternay);

  visitExpr(e->condition);  // Condition will be a bool. It will technically be 1 byte, but we can extend
  popToRegister("%rax");

  push(Instr{.var = CmpInstr{.lhs = "%rax", .rhs = "$0", .size = DataSize::Qword},
             .type = InstrType::Cmp},
       Section::Main);
  push(Instr{.var = JumpInstr{.op = JumpCondition::Equal,
                              .label = "ternaryFalse" + ternayCount},
             .type = InstrType::Jmp},
       Section::Main);

  visitExpr(e->lhs);

  push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned,
                              .label = "ternaryEnd" + ternayCount},
             .type = InstrType::Jmp},
       Section::Main);
  push(Instr{.var = Label{.name = "ternaryFalse" + ternayCount},
             .type = InstrType::Label},
       Section::Main);

  visitExpr(e->rhs);

  push(Instr{.var = Label{.name = "ternaryEnd" + ternayCount},
             .type = InstrType::Label},
       Section::Main);
  ternay++;
}

void codegen::assign(Node::Expr *expr) {
  AssignmentExpr *e = static_cast<AssignmentExpr *>(expr);
  pushDebug(e->line, expr->file_id, e->pos);

  switch (e->assignee->kind) {
    case ND_MEMBER:
    case ND_STRUCT:
      assignStructMember(e);
      break;
    case ND_INDEX:
    case ND_ARRAY:
      assignArray(e);
      break;
    case ND_DEREFERENCE:
      assignDereference(e);
      break;
    default: {
      IdentExpr *lhs = static_cast<IdentExpr *>(e->assignee);
      visitExpr(e->rhs);
      std::string res = variableTable[lhs->name];
      popToRegister(res);
      pushRegister(res);  // Expressions return values!
    }
  }
}

void codegen::arrayElem(Node::Expr *expr) {
  IndexExpr *e = static_cast<IndexExpr *>(expr);
  pushDebug(e->line, expr->file_id, e->pos);

}

// z: [1, 2, 3, 4, 5]
void codegen::_arrayExpr(Node::Expr *expr) {
  ArrayExpr *e = static_cast<ArrayExpr *>(expr);
  // Random array just floating around here...
  // That's not very good.

  // error
  handleError(e->line, e->pos, "Orphaned arrays are not allowed.", "Codegen",
              true);
}

void codegen::memberExpr(Node::Expr *expr) {
  MemberExpr *e = static_cast<MemberExpr *>(expr);
  pushDebug(e->line, expr->file_id, e->pos);
  // If lhs is enum
  std::string lhsName = getUnderlying(e->lhs->asmType);
  if (lhsName == "enum") {
    IdentExpr *lhs = static_cast<IdentExpr *>(e->lhs);
    IdentExpr *rhs = static_cast<IdentExpr *>(e->rhs);
    push(Instr{.var = PushInstr{.what = "$enum_" + lhs->name + "_" + rhs->name,
                                .whatSize = DataSize::Dword},
               .type = InstrType::Push},
         Section::Main);
    return;
  }


  std::cerr
      << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)"
      << std::endl;
  std::cerr << "Member expression not implemented! (Type: NodeKind["
            << (int)e->lhs->kind << "])" << std::endl;
  exit(-1);
  /*
  pushDebug(e->line, expr->file_id, e->pos);
  */
}

void codegen::_struct(Node::Expr *expr) {
  StructExpr *e =
      static_cast<StructExpr *>(expr);  // Needed for the line and pos numbers
  handleError(e->line, e->pos, "Orphaned structs are not allowed.", "Codegen",
              true);
};

void codegen::addressExpr(Node::Expr *expr) {
  AddressExpr *e = static_cast<AddressExpr *>(expr);
  // It was a struct or something like that
  // Get the address of it. The address is the return type (becuase it's a
  // pointer)
  Node::Expr *realRight = CompileOptimizer::optimizeExpr(e->right);
  visitExpr(realRight);
  // We don't want that push!
  PushInstr instr =
      std::get<PushInstr>(text_section.at(text_section.size() - 1).var);
  std::string whatWasPushed = instr.what;
  text_section.pop_back();
  // check if we did this to a struct

  if (realRight->kind == ND_IDENT) {
    IdentExpr *ident = static_cast<IdentExpr *>(realRight);

    // Analyze the variable table contents
    std::string var = variableTable[ident->name];
    // number offset
    long long int offset = std::stoll(var.substr(0, var.find("(")));
    long long int trueOffset = 0;
    if (structByteSizes.find(getUnderlying(ident->asmType)) != structByteSizes.end())
      trueOffset = offset + 8;
    else
      trueOffset = offset;

    push(Instr{.var = LeaInstr{.size = DataSize::Qword,
                               .dest = "%rcx",
                               .src = std::to_string(trueOffset) + "(%rbp)"},
               .type = InstrType::Lea},
         Section::Main);
    pushRegister("%rcx");
    return;
  }
  if (realRight->kind == ND_MEMBER) {
  }
};

void codegen::dereferenceExpr(Node::Expr *expr) {
  DereferenceExpr *e = static_cast<DereferenceExpr *>(expr);

  // Do basic checking!!!
  // Is the lhs a pointer to an array?

  if (static_cast<PointerType *>(e->left->asmType)->underlying->kind == ND_ARRAY_TYPE) {
    // Don't do literally of this shit
    visitExpr(e->left);
    return;
  }

  push(Instr{.var = Comment{.comment = "Dereference"},
             .type = InstrType::Comment},
       Section::Main);
  visitExpr(e->left);  // This should push the address of the thing we want to
                       // dereference

  // Pop the value into a register (lets pick it from the register stack)
  popToRegister("%rax");
  // Push the value at the address
  pushRegister("(%rax)");
}

void codegen::assignStructMember(Node::Expr *expr) {
  AssignmentExpr *e = static_cast<AssignmentExpr *>(expr);
  // This is where you go
  // struct.member = value;

};

void codegen::assignDereference(Node::Expr *expr) {
  AssignmentExpr *e = static_cast<AssignmentExpr *>(expr);
};

void codegen::assignArray(Node::Expr *expr) {
  AssignmentExpr *assign = static_cast<AssignmentExpr *>(expr);
};

void codegen::openExpr(Node::Expr *expr) {
  OpenExpr *e = static_cast<OpenExpr *>(expr);
  pushDebug(e->line, expr->file_id, e->pos);

  visitExpr(e->filename);
  // DEAL WITH THESE LATER (VERY IMPORANT IG)
  // visitExpr(e->canRead);
  // visitExpr(e->canWrite);
  // visitExpr(e->canCreate);

  // Default flags: O_RDWR | O_CREAT | O_TRUNC
  // values:          2    |   100   |  1000   = 578

  // Create the syscall
  popToRegister("%rdi");
  if (e->canRead == nullptr && e->canWrite == nullptr &&
      e->canCreate == nullptr) {
    // The user did not specify any arguments. By default, they are Can Read,
    // Can Write, and Can Create
    moveRegister("%rsi", "$578", DataSize::Qword, DataSize::Qword);
  } else {
    bool canReadLiteral = false;
    bool canWriteLiteral = false;
    bool canCreateLiteral = false;
    if (e->canRead == nullptr)
      canReadLiteral = true;  // Effectively, you wrote 'true'
    if (e->canWrite == nullptr)
      canWriteLiteral = true;
    if (e->canCreate == nullptr)
      canCreateLiteral = true;

    if (e->canRead != nullptr)
      if (e->canRead->kind == ND_BOOL)
        canReadLiteral = true;
    if (e->canWrite != nullptr)
      if (e->canWrite->kind == ND_BOOL)
        canWriteLiteral = true;
    if (e->canCreate != nullptr)
      if (e->canCreate->kind == ND_BOOL)
        canCreateLiteral = true;
    if (canReadLiteral && canWriteLiteral && canCreateLiteral) {
      // We can run these values and evaluate them in comptime
      bool canReadValue;
      bool canWriteValue;
      bool canCreateValue;
      if (e->canRead == nullptr)
        canReadValue = true;
      else
        canReadValue = static_cast<BoolExpr *>(e->canRead)->value;

      if (e->canWrite == nullptr)
        canWriteValue = true;
      else
        canWriteValue = static_cast<BoolExpr *>(e->canWrite)->value;

      if (e->canCreate == nullptr)
        canCreateValue = true;
      else
        canCreateValue = static_cast<BoolExpr *>(e->canCreate)->value;
      // read | write | create
      //  02 |  0100  | 01000
      const static int canRead = 02;
      const static int canWrite = 0100;
      const static int canCreate = 01000;
      moveRegister("%rsi",
                   "$" + std::to_string((canReadValue ? canRead : 0) |
                                        (canWriteValue ? canWrite : 0) |
                                        (canCreateValue ? canCreate : 0)),
                   DataSize::Qword, DataSize::Qword);
    } else {
      // Some values are literals, some are not
      // I could personally not care less about what you specify, so how about
      // we return the default
      moveRegister("%rsi", "$578", DataSize::Qword, DataSize::Qword);
    }
  }
  // mode_t mode = S_IRUSR | S_IWUSR | S_IROTH
  // values: total = 388

  // Have you ever wanted to run a shell file but had to run `chmod +x file.sh`
  // first? This rdx register, 'mode', says who has the permissions (like 'x'
  // for 'execute') on the file I'm not sure why this is needed, because the
  // file literally wouldn't be opened if you didn't have permission, but I have
  // to supply this otherwise the syscall will fail.
  moveRegister("%rdx", "$388", DataSize::Qword, DataSize::Qword);
  moveRegister("%rax", "$2", DataSize::Qword, DataSize::Qword);
  push(Instr{.var = Syscall{.name = "SYS_OPEN"}, .type = InstrType::Syscall},
       Section::Main);
  pushRegister("%rax");
};

void codegen::nullExpr(Node::Expr *expr) {
  // Has implicit value of 0.
  NullExpr *e = static_cast<NullExpr *>(expr);
  push(Instr{.var = Comment{.comment = "Null on " + std::to_string(e->line)},
             .type = InstrType::Comment},
       Section::Main);
  pushRegister("$0");  // 8 bytes is fine here, since it is a pointer
  return;
}

void codegen::getArgcExpr(Node::Expr *expr) {
  // Push debug
  GetArgcExpr *e = static_cast<GetArgcExpr *>(expr);
  pushDebug(e->line, expr->file_id, e->pos);

  // yes, that's it LOL
  pushRegister(".Largc(%rip)");
  useArguments = true;
}

void codegen::getArgvExpr(Node::Expr *expr) {
  // Push debug
  GetArgvExpr *e = static_cast<GetArgvExpr *>(expr);
  pushDebug(e->line, expr->file_id, e->pos);

  pushRegister(".Largv(%rip)");
  useArguments = true;
}

void codegen::strcmp(Node::Expr *expr) {
  StrCmp *s = static_cast<StrCmp *>(expr);
  pushDebug(s->line, expr->file_id, s->pos);

  // Push the first string
  visitExpr(s->v1);
  popToRegister("%rdi");
  
  // Push the second string
  visitExpr(s->v2);
  popToRegister("%rsi");

  // call the native strcmp function
  push(Instr{.var = CallInstr{.name = "native_strcmp"}, .type = InstrType::Call}, Section::Main);
  nativeFunctionsUsed[NativeASMFunc::strcmp] = true;

  // The return value is in %rax, so we can just push it
  pushRegister("%rax");
}
