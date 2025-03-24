#include "gen.hpp"
#include "optimizer/compiler.hpp"
#include "optimizer/instr.hpp"
#include "../typeChecker/type.hpp"
#include <string>

void codegen::visitExpr(Node::Expr *expr) {
  // Optimize the expression before we handle it!
  Node::Expr *realExpr = CompileOptimizer::optimizeExpr(expr);
  if (int(realExpr->kind) > (int)NodeKind::ND_NULL) { // im dumb ignore me i put
                                                      // the wrong sign :sob: ya
    std::cout << "stinky node D:"
              << std::endl; // bp here if the nodekind is something weird
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
    pushRegister("$" + std::to_string(e->value));
    break;
  } // It happened here in the ident
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
    switch (getByteSizeOfType(e->asmType)) {
    case 1:
      push(Instr{.var = PushInstr{.what = res, .whatSize = DataSize::Byte},
                 .type = InstrType::Push},
           Section::Main);
      break;
    case 2:
      push(Instr{.var = PushInstr{.what = res, .whatSize = DataSize::Word},
                 .type = InstrType::Push},
           Section::Main);
      break;
    case 4:
      push(Instr{.var = PushInstr{.what = res, .whatSize = DataSize::Dword},
                 .type = InstrType::Push},
           Section::Main);
      break;
    case 8:
    default:
      push(Instr{.var = PushInstr{.what = res, .whatSize = DataSize::Qword},
                 .type = InstrType::Push},
           Section::Main);
      break;
    }
    break;
  }
  case ND_STRING: {
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
  case ND_CHAR: {
    CharExpr *charExpr = static_cast<CharExpr *>(expr);
    pushDebug(charExpr->line, expr->file_id, charExpr->pos);
    pushRegister("$" + std::to_string((int)charExpr->value));
    break;
  }
  case ND_FLOAT: {
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
                       .bytesToDefine = DataSize::Dword /* long, aka 32-bits */,
                       .what =
                           std::to_string(convertFloatToInt(floating->value))},
               .type = InstrType::DB},
         Section::ReadonlyData);
    break;
  }
  case ND_BOOL: {
    BoolExpr *boolExpr = static_cast<BoolExpr *>(expr);
    // Technically just an int but SHHHHHHHH.............................
    push(Instr{.var = PushInstr{.what = "$" + std::to_string(boolExpr->value),
                                .whatSize = DataSize::Byte},
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
    default: // Not like i'll be dealing with 3-byte integers anyways
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
      push(Instr{.var=PopInstr{.where = rhsReg, .whereSize = size}, .type=InstrType::Pop}, Section::Main);
      push(Instr{.var=PopInstr{.where = lhsReg, .whereSize = size}, .type=InstrType::Pop}, Section::Main);
    } else {
      visitExpr(e->rhs);
      visitExpr(e->lhs);
      push(Instr{.var=PopInstr{.where = lhsReg, .whereSize = size}, .type=InstrType::Pop}, Section::Main);
      push(Instr{.var=PopInstr{.where = rhsReg, .whereSize = size}, .type=InstrType::Pop}, Section::Main);
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
            pushLinker("movsbw " + lhsReg + ", %ax" , Section::Main);
            break;
        }
        push(Instr{.var = DivInstr{.from = rhsReg, .isSigned = true, .size = size}, .type = InstrType::Div}, Section::Main);
      } else {
        // it was unsigned so we have way less shit to worry about
        DataSize size = DataSize::Qword;
        push(Instr{.var=XorInstr{.lhs="%rdi", .rhs="%rdi"},.type=InstrType::Xor},Section::Main);
        switch (getByteSizeOfType(returnType)) {
          case 1:
            size = DataSize::Byte;
            break;
          case 2:
            size = DataSize::Word;
            break;
          case 4:
            size = DataSize::Dword;
            break;
          case 8:
          default:
            size = DataSize::Qword;
            break;
        }
        push(Instr{.var=DivInstr{.from=rhsReg, .isSigned = false, .size = size}, .type = InstrType::Div}, Section::Main);
      }

      if (op == "mod") {
        switch (getByteSizeOfType(returnType)) {
          case 1:
            push(Instr{.var=PushInstr{.what="%dl", .whatSize = DataSize::Byte},.type = InstrType::Push}, Section::Main);
            break;
          case 2:
            push(Instr{.var=PushInstr{.what="%dx", .whatSize = DataSize::Word},.type = InstrType::Push}, Section::Main);
            break;
          case 4:
            push(Instr{.var=PushInstr{.what="%ebx", .whatSize = DataSize::Dword},.type = InstrType::Push}, Section::Main);
             break;
          case 8:
          default:
            push(Instr{.var=PushInstr{.what="%rbx", .whatSize = DataSize::Qword},.type = InstrType::Push}, Section::Main);
            break;
        }
      } else {
        // It doesnt matter, push the real thing and get out of here
        push(Instr{.var=PushInstr{.what=lhsReg, .whatSize = size},.type = InstrType::Push}, Section::Main);
      }
    } else if (op == "shl" || op == "shr" || op == "sar" || op == "sal") {
      // Shift operations require either the CL register or an immediate
      // Check if there is an immediate
      if (e->rhs->kind == ND_INT) {
        long long shiftAmount = static_cast<IntExpr *>(e->rhs)->value;
        if (shiftAmount == 1) {
          // This requires NO number 1
          pushLinker(op + " " + lhsReg + "\n\t", Section::Main);
          push(Instr{.var=PushInstr{.what=lhsReg, .whatSize = size},.type = InstrType::Push}, Section::Main);
          return;
        }
        push(Instr{.var=BinaryInstr{.op=op, .src="$" + std::to_string(shiftAmount), .dst=rhsReg},
                   .type=InstrType::Binary},
             Section::Main);
        // push the result
        push(Instr{.var=PushInstr{.what=rhsReg, .whatSize = size},.type = InstrType::Push}, Section::Main);
      } else {
        // If there is no immediate, we need to pop the value into CL
        push(Instr{.var = MovInstr{.dest = "%cl", .src = rhsReg, .destSize = DataSize::Byte, .srcSize = size},
                   .type = InstrType::Mov},
             Section::Main);
        push(Instr{.var=BinaryInstr{.op=op, .src="%cl", .dst=lhsReg},
                   .type=InstrType::Binary},
             Section::Main);
        // push the result
        push(Instr{.var=PushInstr{.what=lhsReg, .whatSize = size},.type = InstrType::Push}, Section::Main);
      }
    } else {
      // Every other operation ...
      push(Instr{.var = BinaryInstr{.op = op, .src = rhsReg, .dst = lhsReg},
                 .type = InstrType::Binary},
           Section::Main);
      push(Instr{.var=PushInstr{.what=lhsReg, .whatSize = size},.type = InstrType::Push}, Section::Main);
    }
    return; // Done
  } else if (returnType->name == "float" || returnType->name == "double") {
    // Similar logic for floats
    size = returnType->name == "float" ? DataSize::SS : DataSize::SD;
    std::string suffix = size == DataSize::SS ? "ss" : "sd";
    int lhsDepth = getExpressionDepth(static_cast<BinaryExpr *>(e->lhs));
    int rhsDepth = getExpressionDepth(static_cast<BinaryExpr *>(e->rhs));
    if (lhsDepth > rhsDepth) {
      visitExpr(e->lhs);
      visitExpr(e->rhs);
      popToRegister("%xmm1"); // Pop RHS into XMM1
      popToRegister("%xmm0"); // Pop LHS into XMM0
    } else {
      visitExpr(e->rhs);
      visitExpr(e->lhs);
      popToRegister("%xmm0"); // Pop LHS into XMM0
      popToRegister("%xmm1"); // Pop RHS into XMM1
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
    push(Instr{.var=PushInstr{.what="%xmm0", .whatSize = size},.type = InstrType::Push}, Section::Main);
  } else if (returnType->name == "bool") {
    // We need to compare the two values
    // Check depth
    bool isFloating = e->lhs->asmType->kind == ND_SYMBOL_TYPE && (getUnderlying(e->lhs->asmType) == "float" || getUnderlying(e->lhs->asmType) == "double");
    std::string lhsReg = "";
    std::string rhsReg = "";
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
      push(Instr{.var=PopInstr{.where = rhsReg, .whereSize = size}, .type=InstrType::Pop}, Section::Main);
      push(Instr{.var=PopInstr{.where = lhsReg, .whereSize = size}, .type=InstrType::Pop}, Section::Main);
    } else {
      visitExpr(e->rhs);
      visitExpr(e->lhs);
      push(Instr{.var=PopInstr{.where = lhsReg, .whereSize = size}, .type=InstrType::Pop}, Section::Main);
      push(Instr{.var=PopInstr{.where = rhsReg, .whereSize = size}, .type=InstrType::Pop}, Section::Main);
    }
    // Get the operation
    std::string op = lookup(opMap, e->op);
    // Perform the operation
    // by running a comparison
    if (isFloating) {
      std::string letter = size == DataSize::SS ? "s" : "d";
      pushLinker("ucomis" + letter + " %xmm1, %xmm0\n\t", Section::Main);
    } else {
      push(Instr{.var = CmpInstr{.lhs = lhsReg, .rhs = rhsReg},
                 .type = InstrType::Cmp},
           Section::Main);
    }
    // Move the bytes up to 64-bits
    push(Instr{.var=PushInstr{.what="%al", .whatSize = DataSize::Byte},.type = InstrType::Push}, Section::Main);
  }
}

void codegen::unary(Node::Expr *expr) {
  UnaryExpr *e = static_cast<UnaryExpr *>(expr);
  pushDebug(e->line, expr->file_id, e->pos);
  visitExpr(e->expr);
  // Its gonna be a pop guys
  PushInstr instr = std::get<PushInstr>(text_section.at(text_section.size() - 1).var);
  std::string whatWasPushed = instr.what;

  text_section.pop_back();
  
  if (e->op == "-") {
    // Perform the operation
    push(Instr{.var = NegInstr{.what = whatWasPushed}, .type = InstrType::Neg}, Section::Main);
  } else if (e->op == "++" || e->op == "--") {
    // Perform the operation
    std::string res = (e->op == "++") ? "inc" : "dec";
    // Dear code reader, i apologize
    push(Instr{.var = LinkerDirective{.value = res + "q " + whatWasPushed + "\n\t"}, .type = InstrType::Linker }, Section::Main);
  }
  // Push the result
  pushRegister(whatWasPushed);
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
            isLea = true; // It was in there!
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
            Section::Main); // abi standard (can hold many bytes of data, so its
                            // fine for both floats AND doubles to fit in here)
      } else if (structByteSizes.find(st->name) != structByteSizes.end()) {
        // TODO: No! This is really wrong!!!!
        pushRegister("%rax"); // When tossing this thing around, its handled
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
    int intArgCount = 1; // 1st is preserved for struct ptr above
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
            Section::Main); // abi standard (can hold many bytes of data, so its
                            // fine for both floats AND doubles to fit in here)
      } else if (structByteSizes.find(st->name) != structByteSizes.end()) {
        // TODO: No! This is really wrong!!!!
        pushRegister("%rax"); // When tossing this thing around, its handled
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

  visitExpr(e->condition);
  popToRegister("%rax");

  push(Instr{.var = CmpInstr{.lhs = "%rax", .rhs = "$0"},
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
    pushRegister(res); // Expressions return values!
  }
  }
}

void codegen::arrayElem(Node::Expr *expr) {
  IndexExpr *e = static_cast<IndexExpr *>(expr);
  pushDebug(e->line, expr->file_id, e->pos);
  // Evaluate the arrauy
  // Evaluate the index
  if (e->rhs->kind == ND_INT) {
    // It's a constant index
    IntExpr *index = static_cast<IntExpr *>(e->rhs);
    if (e->lhs->kind == ND_IDENT) {
      IdentExpr *ident = static_cast<IdentExpr *>(e->lhs);
      Node::Type *underlying = static_cast<ArrayType *>(e->asmType)->underlying;
      std::string whereBytes = variableTable[ident->name];
      long long offset = std::stoll(whereBytes.substr(
          0, whereBytes.find("("))); // this is the base of the array - the
                                     // first byte of the first element
      offset -= (index->value * getByteSizeOfType(underlying));
      pushRegister(std::to_string(offset) + "(%rbp)");
    } else {
      visitExpr(e->lhs);
      // it is an array type so we must actually lea this!
      PushInstr instr =
          std::get<PushInstr>(text_section.at(text_section.size() - 1).var);
      std::string whatWasPushed = instr.what;
      text_section.pop_back();
      push(Instr{.var = LeaInstr{.size = DataSize::Qword,
                                 .dest = "%rcx",
                                 .src = whatWasPushed},
                 .type = InstrType::Lea},
           Section::Main);
      short byteSize = getByteSizeOfType(e->lhs->asmType);
      size_t offset = -index->value * byteSize;
      if (offset == 0)
        pushRegister("(%rcx)");
      else
        pushRegister(std::to_string(offset) + "(%rcx)");
    }
  } else {
    // This is a little more intricate.
    // We have to evaluate the index and multiply it by the size of the type
    if (e->lhs->kind == ND_IDENT) {
      push(Instr{.var = LeaInstr{.size = DataSize::Qword,
                                 .dest = "%rcx",
                                 .src = variableTable
                                     [static_cast<IdentExpr *>(e->lhs)->name]},
                 .type = InstrType::Lea},
           Section::Main);
    } else {
      visitExpr(e->lhs);
      // lhs is always gonna be an array, but we need it to hold the addr
      PushInstr instr =
          std::get<PushInstr>(text_section.at(text_section.size() - 1).var);
      std::string whatWasPushed = instr.what;
      text_section.pop_back();
      push(Instr{.var = LeaInstr{.size = DataSize::Qword,
                                 .dest = "%rcx",
                                 .src = whatWasPushed},
                 .type = InstrType::Lea},
           Section::Main);
      // %rcx contains the base of the array
    }
    visitExpr(e->rhs);
    popToRegister("%rax");
    // Multiply it by the size of the type
    int underlyingByteSize = getByteSizeOfType(
        static_cast<ArrayType *>(e->lhs->asmType)->underlying);
    switch (underlyingByteSize) {
    case 1: {
      // No need to multiply
      // Ex: []char or []bool
      push(Instr{.var = PushInstr{.what = "(%rcx, %rax)",
                                  .whatSize = DataSize::Byte},
                 .type = InstrType::Push},
           Section::Main);
      break;
    }
    case 2:
      push(Instr{.var = PushInstr{.what = "(%rcx, %rax, 2)",
                                  .whatSize = DataSize::Word},
                 .type = InstrType::Push},
           Section::Main);
      break;
    case 4:
      push(Instr{.var = PushInstr{.what = "(%rcx, %rax, 4)",
                                  .whatSize = DataSize::Dword},
                 .type = InstrType::Push},
           Section::Main);
      break;
    case 8:
      push(Instr{.var = PushInstr{.what = "(%rcx, %rax, 8)",
                                  .whatSize = DataSize::Qword},
                 .type = InstrType::Push},
           Section::Main);
      break;
    default: {
      // Sad, we can't rely on little syntactical sugar of the assembler to
      // cheat our way out :(
      push(Instr{.var = BinaryInstr{.op = "imul",
                                    .src = "$" +
                                           std::to_string(underlyingByteSize),
                                    .dst = "%rax"},
                 .type = InstrType::Binary},
           Section::Main);
      pushRegister("(%rcx, %rax)");
      break;
    }
    }
  }
  // the end!
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
    pushRegister("$enum_" + lhs->name + "_" + rhs->name);
    return;
  }

  if (structByteSizes.find(lhsName) != structByteSizes.end()) {
    // It's a struct
    std::string structName = lhsName;
    // Now we have the name of the struct!
    if (structName == "unknown") {
      // Typechecker thought that its fields did not correlate to a struct.
      // That means we cannot calculate byte sizes/offsets or any of that good
      // stuff since the struct wasn't valid.
      std::cerr << "Cannot access member of unknown struct" << std::endl;
      exit(-1);
    }
    // Now we can access the struct's fields
    std::vector<StructMember> &fields = structByteSizes[structName].second;
    // Eval lhs (this will put the struct's address onto the stack)
    visitExpr(e->lhs); // this could be an ident or something
    /*
     * This is an optimization for nested member expressions.
     * In the assembly, it may look like this:
     * leaq -8(%rbp), %rcx
     * leaq -24(%rcx), %rcx
     * leaq -8(%rcx), %rcx
     * ...
     * Now, they will be optimized to only one leaq instruction.
     * leaq -40(%rbp), %rcx ; :D
     *
     * Additionally, this system is robust enough to handle MemberExpr's on
     * dereferencing pointers due to the fact that the register used is %rcx
     * rather than the actual location on the stack.
     */
    // It likely looks like this: leaq -8(%rbp), %rcx --- pushq %rcx. We want to
    // get that RBP offset there!!
    std::string whatWasPushed = "";
    if (text_section.at(text_section.size() - 2).type != InstrType::Lea) {
      // It was just a push. This means that it was a pointer and we have to actually just do the work here.
      whatWasPushed = std::get<PushInstr>(text_section.at(text_section.size() - 1)
                                             .var).what;
      int offsetFromBase = 0;
      for (size_t i = 0; i < fields.size(); i++) {
        if (fields[i].first == static_cast<IdentExpr *>(e->rhs)->name) {
          offsetFromBase = -(fields[i].second.second);
          break;
        }
      }
      bool resultIsStruct = structByteSizes.find(getUnderlying(e->asmType)) !=
                            structByteSizes.end();
      
      popToRegister("%rcx");
      if (resultIsStruct) {
        push(Instr{.var = LeaInstr{.size = DataSize::Qword, // Leas will always be qword, because we are 64-bit
                                    .dest = "%rcx",
                                    .src = std::to_string(offsetFromBase) + "(%rcx)"},
                    .type = InstrType::Lea},
              Section::Main);
        pushRegister("%rcx");
      } else {
        DataSize size = DataSize::Qword;
        switch (getByteSizeOfType(e->asmType)) {
        case 1:
          size = DataSize::Byte;
          break;
        case 2:
          size = DataSize::Word;
          break;
        case 4:
          size = DataSize::Dword;
          break;
        case 8:
        default:
          size = DataSize::Qword;
          break;
        }
        push(Instr{.var=PushInstr{.what = std::to_string(offsetFromBase) + "(%rcx)",
                                  .whatSize = size},
                  .type = InstrType::Push},
              Section::Main);
      }
      return;
    }
    text_section.pop_back(); // Just a push. Nobody cares.
    whatWasPushed = std::get<LeaInstr>(text_section.at(text_section.size() - 1)
                                           .var).src;
    text_section.pop_back(); // The lea instruction
    bool pushedHasOffset = whatWasPushed.find("(") != std::string::npos;
    signed int whatWasPushedOffset =
        pushedHasOffset
            ? (whatWasPushed.find('(') == 0 ? 0
                                            : std::stoi(whatWasPushed.substr(
                                                  0, whatWasPushed.find("("))))
            : 0;
    std::string whatWasPushedReg =
        pushedHasOffset
            ? whatWasPushed.substr(
                  whatWasPushed.find('(') + 1,
                  (whatWasPushed.size() - whatWasPushed.find('(')) - 2)
            : whatWasPushed;

    // Now, we have to do some crazy black magic shit like -8(%rcx) but we find
    // what to replace '8' with.
    std::string fieldName = static_cast<IdentExpr *>(e->rhs)->name;

    // Right now, %rcx points to the beginning of the struct, which contains the
    // last field. In order to get the field we need, we should just loop over
    // all the fields and add to an offset.
    short offset = 0;
    for (size_t i = 0; i < structByteSizes[structName].second.size(); i++) {
      if (structByteSizes[structName].second.at(i).first == fieldName) {
        offset = -(signed short)(structByteSizes[structName]
                      .second.at(i)
                      .second
                      .second); // This is the offset, not the size of the type!
        break;
      }
    }
    // check if the type of the result is another struct (push the address
    // instead)
    if (structByteSizes.find(getUnderlying(e->asmType)) !=
        structByteSizes.end()) {
      // Instead of pushing straight up, we must lea
      if (pushedHasOffset) {
        push(Instr{.var = LeaInstr{.size = DataSize::Qword,
                                   .dest = "%rcx",
                                   .src = std::to_string(whatWasPushedOffset +
                                                         (offset)) +
                                          "(" + whatWasPushedReg + ")"},
                   .type = InstrType::Lea},
             Section::Main);
      } else {
        push(Instr{.var = LeaInstr{.size = DataSize::Qword,
                                   .dest = "%rcx",
                                   .src = std::to_string(offset) + "(" +
                                          whatWasPushedReg + ")"},
                   .type = InstrType::Lea},
             Section::Main);
      }
      pushRegister("%rcx");
      return;
    }
    // submit the answer :D
    if (offset == 0) {
      if (pushedHasOffset)
        pushRegister(whatWasPushed);
      else
        pushRegister(
            "(" + whatWasPushed +
            ")"); // It was not an effective address. It was just a register
    } else {
      if (pushedHasOffset)
        pushRegister(std::to_string(whatWasPushedOffset + offset) + "(" +
                     whatWasPushedReg + ")");
      else
        pushRegister(std::to_string(offset) + "(" + whatWasPushed + ")");
    }
    return;
  }

  if (e->lhs->kind == ND_INDEX) {
    visitExpr(e->lhs);
    // Should push the value of the array element. If not, then I'm dumb and it
    // pushed its addr Pop the value into a register
    popToRegister("%rcx");
    // Now we can access the member
    IndexExpr *index = static_cast<IndexExpr *>(e->lhs);
    // get the member
    IdentExpr *member = static_cast<IdentExpr *>(e->rhs);
    // get the type of whatever we are indexing
    Node::Type *indexType = index->asmType;
    // if type was a struct
    if (indexType->kind == ND_SYMBOL_TYPE) {
      SymbolType *sym = static_cast<SymbolType *>(indexType);
      if (structByteSizes.find(sym->name) != structByteSizes.end()) {
        int offset = 0; // im gonna do something about that later
        std::string structName = sym->name;
        std::vector<StructMember> fields = structByteSizes[structName].second;
        for (size_t i = 0; i < fields.size(); i++) {
          if (fields[i].first == member->name)
            break;
          offset += fields[i].second.second;
        }
        // now we can access the member
        pushRegister(std::to_string(offset) + "(%rcx)");
      }
      // if type was an array (not dealing with this today)
    }
    return;
  };

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
      static_cast<StructExpr *>(expr); // Needed for the line and pos numbers
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

    push(Instr{.var = LeaInstr{.size = DataSize::Qword,
                               .dest = "%rcx",
                               .src = variableTable[ident->name]},
               .type = InstrType::Lea},
         Section::Main);
    pushRegister("%rcx");
    return;
  } else if (realRight->kind == ND_MEMBER) {
    MemberExpr *member = static_cast<MemberExpr *>(realRight);
    // Visit the lhs of the member
    visitExpr(member->lhs); // This SHOULD be a struct
    // Pop the value into a register
    popToRegister("%rcx");
    // But we must know the type of the lhs in order to know the offset of the
    // field
    std::string structName = getUnderlying(member->lhs->asmType);
    std::string fieldName = static_cast<IdentExpr *>(member->rhs)->name;
    // Now we can access the member
    int offset = 0;
    for (size_t i = 0; i < structByteSizes[structName].second.size(); i++) {
      offset =
          structByteSizes[structName]
              .second.at(i)
              .second.second; // This is the offset, not the size of the type!
      if (structByteSizes[structName].second.at(i).first == fieldName)
        break;
    }
    // Lea the address of the field
    push(Instr{.var = LeaInstr{.size = DataSize::Qword,
                               .dest = "%rcx",
                               .src = std::to_string(offset) + "(%rcx)"},
               .type = InstrType::Lea},
         Section::Main);
    pushRegister("%rcx");
    return;
  } else if (realRight->kind == ND_INDEX) {
    IndexExpr *index = static_cast<IndexExpr *>(realRight);
    Node::Expr *compute = CompileOptimizer::optimizeExpr(index->rhs);
    if (compute->kind == ND_INT) {
      // It's constant, which means we can do some HEAVY optimization here!
      IntExpr *intExpr = static_cast<IntExpr *>(compute);
      signed long long int value = intExpr->value;
      // Visit the lhs of the index
      visitExpr(index->lhs);   // This SHOULD be an array, which means it will
                               // push the address
      text_section.pop_back(); // Remove the 'push'
      // There should be an LEA on top of the text_section rn
      LeaInstr instr =
          std::get<LeaInstr>(text_section.at(text_section.size() - 1).var);
      text_section.pop_back(); // Remove the 'lea'
      // Now we can "re-push" that value because its constant
      int underlyingByteSize = getByteSizeOfType(
          static_cast<ArrayType *>(index->lhs->asmType)->underlying);
      std::string leaRegister =
          instr.src.find('(')
              ? instr.src.substr(instr.src.find('(') + 1,
                                 instr.src.size() - instr.src.find('(') - 2)
              : instr.src;
      int leaOffset = instr.src.find('(')
                          ? std::stoi(instr.src.substr(0, instr.src.find('(')))
                          : 0;
      push(Instr{.var = LeaInstr{.size = DataSize::Qword,
                                 .dest = "%rcx",
                                 .src = std::to_string(
                                            leaOffset +
                                            (value * underlyingByteSize)) +
                                        "(" + leaRegister + ")"},
                 .type = InstrType::Lea},
           Section::Main);
      pushRegister("%rcx");
      return;
    }
    // We have to compute this :(
    // Visit the weird rhs
    visitExpr(compute);
    int computeByteSize = getByteSizeOfType(compute->asmType);
    switch (computeByteSize) {
    case 1:
      push(Instr{.var = PopInstr{.where = "%dl", .whereSize = DataSize::Byte},
                 .type = InstrType::Pop},
           Section::Main);
      // Movzx
      // TODO: If the src type is signed, do a sign extend instead
      pushLinker("movzbq %dl, %rdx\n\t", Section::Main);
      break;
    case 2:
      push(Instr{.var = PopInstr{.where = "%dx", .whereSize = DataSize::Word},
                 .type = InstrType::Pop},
           Section::Main);
      // Movzx
      pushLinker("movzwq %dx, %rdx\n\t", Section::Main);
      break;
    case 4:
      push(Instr{.var = PopInstr{.where = "%edx", .whereSize = DataSize::Dword},
                 .type = InstrType::Pop},
           Section::Main);
      // Movzx
      pushLinker("movzlq %edx, %rdx\n\t", Section::Main);
      break;
    default: // What went wrong? Why would it not be 8? It should be 8....
    case 8:
      push(Instr{.var = PopInstr{.where = "%rdx", .whereSize = DataSize::Qword},
                 .type = InstrType::Pop},
           Section::Main);
      break;
    }
    // Pop the value into a register
    popToRegister("%rax");
    // Visit the lhs of the index
    visitExpr(index->lhs); // This SHOULD be an array, which means it will push
                           // the address
    popToRegister("%rdx");
    // Push the address as a effective address and multiplication
    int underlyingByteSize = getByteSizeOfType(
        static_cast<ArrayType *>(index->lhs->asmType)->underlying);
    switch (underlyingByteSize) {
    case 1:
      push(Instr{.var = LeaInstr{.size = DataSize::Qword,
                                 .dest = "%rcx",
                                 .src = "(%rdx, %rax)"},
                 .type = InstrType::Lea},
           Section::Main);
      break;
    case 2:
      push(Instr{.var = LeaInstr{.size = DataSize::Qword,
                                 .dest = "%rcx",
                                 .src = "(%rdx, %rax, 2)"},
                 .type = InstrType::Lea},
           Section::Main);
      break;
    case 4:
      push(Instr{.var = LeaInstr{.size = DataSize::Qword,
                                 .dest = "%rcx",
                                 .src = "(%rdx, %rax, 4)"},
                 .type = InstrType::Lea},
           Section::Main);
      break;
    case 8:
      push(Instr{.var = LeaInstr{.size = DataSize::Qword,
                                 .dest = "%rcx",
                                 .src = "(%rdx, %rax, 8)"},
                 .type = InstrType::Lea},
           Section::Main);
      break;
    default:
      push(Instr{.var = BinaryInstr{.op = "imul",
                                    .src = "$" +
                                           std::to_string(underlyingByteSize),
                                    .dst = "%rax"},
                 .type = InstrType::Binary},
           Section::Main);
      push(Instr{.var = LeaInstr{.size = DataSize::Qword,
                                 .dest = "%rcx",
                                 .src = "(%rdx, %rax)"},
                 .type = InstrType::Lea},
           Section::Main);
      break;
    }
    pushRegister("%rcx");
    return;
  }
  push(Instr{.var = LeaInstr{.size = DataSize::Qword,
                             .dest = "%rcx",
                             .src = whatWasPushed},
             .type = InstrType::Lea},
       Section::Main);
  pushRegister("%rcx");
};

void codegen::dereferenceExpr(Node::Expr *expr) {
  DereferenceExpr *e = static_cast<DereferenceExpr *>(expr);

  push(Instr{.var = Comment{.comment = "Dereference"},
             .type = InstrType::Comment},
       Section::Main);
  visitExpr(e->left); // This should push the address of the thing we want to
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

  if (structByteSizes.find(getUnderlying(e->rhs->asmType)) !=
      structByteSizes.end()) {
    if (e->rhs->kind == ND_STRUCT) {
      // It's a struct
      visitExpr(e->assignee);
      popToRegister("%rdx");
      // TODO: This function will always make the relative offsets to the base
      // register NEGATIVE which in this case it shouldnt be
      // TODO: Other than that though, this implmentation works 100% fine as it
      // seems
      declareStructVariable(e->rhs, getUnderlying(e->assignee->asmType), "%rdx",
                            0);
      // Assign expressions must return something, so we will return the offset
      // of the rcx thing Lea instr
      push(Instr{.var = LeaInstr{.size = DataSize::Qword,
                                 .dest = "%rcx",
                                 .src = "(%rdx)"},
                 .type = InstrType::Lea},
           Section::Main);
      visitExpr(e->assignee);
      popToRegister(
          "%rsi"); // The register used here doesn't matter, but rsi is rarely
                   // used in Zura (other than syscall @ functions)
      // Rdx contains the smaller struct. Rsi contains the bigger struct. Move
      // each byte!
      short byteCount = getByteSizeOfType(e->rhs->asmType);
      while (byteCount > 0) {
        if (byteCount >= 8) {
          // Move a qword
          byteCount -= 8;
          moveRegister("%rax", std::to_string(-byteCount) + "(%rdx)",
                       DataSize::Qword, DataSize::Qword);
          moveRegister(std::to_string(-byteCount) + "(%rsi)", "%rax",
                       DataSize::Qword, DataSize::Qword);
        } else if (byteCount >= 4) {
          // Move a dword (long)
          byteCount -= 4;
          moveRegister("%eax", std::to_string(-byteCount) + "(%rdx)",
                       DataSize::Dword, DataSize::Dword);
          moveRegister(std::to_string(-byteCount) + "(%rsi)", "%eax",
                       DataSize::Dword, DataSize::Dword);
        } else if (byteCount >= 2) {
          // Move a word (short)
          byteCount -= 2;
          moveRegister("%ax", std::to_string(-byteCount) + "(%rdx)",
                       DataSize::Word, DataSize::Word);
          moveRegister(std::to_string(-byteCount) + "(%rsi)", "%ax",
                       DataSize::Word, DataSize::Word);
        } else {
          // Move a byte (char)
          byteCount -= 1;
          moveRegister("%al", std::to_string(-byteCount) + "(%rdx)",
                       DataSize::Byte, DataSize::Byte);
          moveRegister(std::to_string(-byteCount) + "(%rsi)", "%al",
                       DataSize::Byte, DataSize::Byte);
        }
      }
      // Assignment expr's must return something
      // Let's return the little struct ptr
      pushRegister("%rdx");
      return;
    }
  }
  // The value is not a struct-- hopefully this is very easy
  // We have to evaluate the rhs
  // Now we pop into the surrounding struct
  visitExpr(e->assignee);
  text_section.pop_back(); // previous instr would juts be a push rcx which we
                           // do not need
  visitExpr(e->rhs);
  // Pop the value into a register
  switch (getByteSizeOfType(e->rhs->asmType)) {
  case 1:
    push(Instr{.var = PopInstr{.where = std::to_string(getByteSizeOfType(
                                            e->rhs->asmType)) +
                                        "(%rcx)",
                               .whereSize = DataSize::Byte},
               .type = InstrType::Pop},
         Section::Main);
    break;
  case 2:
    push(Instr{.var = PopInstr{.where = std::to_string(getByteSizeOfType(
                                            e->rhs->asmType)) +
                                        "(%rcx)",
                               .whereSize = DataSize::Word},
               .type = InstrType::Pop},
         Section::Main);
    break;
  case 4:
    push(Instr{.var = PopInstr{.where = std::to_string(getByteSizeOfType(
                                            e->rhs->asmType)) +
                                        "(%rcx)",
                               .whereSize = DataSize::Dword},
               .type = InstrType::Pop},
         Section::Main);
    break;
  case 8:
  default:
    push(Instr{.var = PopInstr{.where = std::to_string(getByteSizeOfType(
                                            e->rhs->asmType)) +
                                        "(%rcx)",
                               .whereSize = DataSize::Qword},
               .type = InstrType::Pop},
         Section::Main);
    break;
  }
  // Assign expressions must return something
  pushRegister("%rcx");
};

void codegen::assignDereference(Node::Expr *expr) {
  AssignmentExpr *e = static_cast<AssignmentExpr *>(expr);

  if (structByteSizes.find(getUnderlying(e->asmType)) != structByteSizes.end()) {
    visitExpr(e->rhs);
    PushInstr instr =
        std::get<PushInstr>(text_section.at(text_section.size() - 1).var);
    text_section.pop_back();
    std::string offsetRegister = instr.what.find('(')
                                     ? instr.what.substr(instr.what.find('(') + 1,
                                                         instr.what.size() -
                                                             instr.what.find('(') -
                                                             2)
                                     : instr.what;
    int offset = instr.what.find('(')
                             ? std::stoi(instr.what.substr(0, instr.what.find('(')))
                             : 0;
    dereferenceStructPtr(static_cast<DereferenceExpr *>(e->assignee)->left, getUnderlying(e->asmType), offsetRegister, offset);
    return;
  }

  visitExpr(e->rhs);  // Generate code for the right-hand side (RHS) value
  popToRegister("%rax"); // Get the value from the stack into %rax

  DereferenceExpr *deref = static_cast<DereferenceExpr *>(e->assignee);
  visitExpr(deref->left); // Generate code for the left-hand side (LHS) address
  popToRegister("%rcx"); // Get the address from the stack into %rcx

  // Move the value from %rax to the address in %rcx
  moveRegister("(%rcx)", "%rax", DataSize::Qword, DataSize::Qword);
  // Assign expressions must return something
  pushRegister("%rax");
};

void codegen::assignArray(Node::Expr *expr) {
  AssignmentExpr *assign = static_cast<AssignmentExpr *>(expr);
  if (assign->assignee->kind == ND_INDEX) {
    IndexExpr *e = static_cast<IndexExpr *>(assign->assignee);
    IntExpr *index = static_cast<IntExpr *>(e->rhs);
    signed long long idx = index->value;
    // lhs is likely an identifier to an array
    // it doesn't matter for this example, though
    visitExpr(e->lhs);
    popToRegister("%rcx"); // rcx now has the base of the array
    signed short int size = getByteSizeOfType(e->lhs->asmType);
    size_t offset = idx * size;
    visitExpr(e->rhs);
    popToRegister(std::to_string(offset) + "(%rcx)");
    // Push the value again
    pushRegister(std::to_string(offset) + "(%rcx)");
    return;
  } else if (assign->rhs->kind == ND_ARRAY) {
    // assume that the bytes are already allocated for us
    // (for example, in a sub from rax or from the member of a struct)
    if (assign->assignee->asmType->kind == ND_ARRAY_TYPE) {
      // array = array
      // Go through each member of the first array and override with the second.
      visitExpr(assign->assignee);
      PushInstr instr =
          std::get<PushInstr>(text_section.at(text_section.size() - 1).var);
      text_section.pop_back();
      push(Instr{.var = LeaInstr{.size = DataSize::Qword,
                                 .dest = "%rcx",
                                 .src = instr.what},
                 .type = InstrType::Lea},
           Section::Main);
      ArrayExpr *rhs = static_cast<ArrayExpr *>(assign->rhs);
      for (size_t i = 0; i < rhs->elements.size(); i++) {
        visitExpr(rhs->elements.at(i));
        popToRegister(std::to_string(-i * getByteSizeOfType(rhs->type)) +
                      "(%rcx)");
      }
      // push the base of the array
      pushRegister("%rcx");
      return;
    }
  }
  // I think that's it??!!!
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
      canReadLiteral = true; // Effectively, you wrote 'true'
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
  push(Instr{.var=Comment{.comment = "Null on " + std::to_string(e->line) },
            .type=InstrType::Comment},
       Section::Main);
  pushRegister("$0"); // 8 bytes is fine here, since it is a pointer
  return;
}