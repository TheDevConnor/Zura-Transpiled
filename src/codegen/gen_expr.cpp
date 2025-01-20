#include "gen.hpp"
#include "optimizer/instr.hpp"
#include "optimizer/compiler.hpp"
#include <string>

void codegen::visitExpr(Node::Expr *expr) {
  // Optimize the expression before we handle it!
  Node::Expr *realExpr = CompileOptimizer::optimizeExpr(expr);
  if (int(realExpr->kind) > (int)NodeKind::ND_NULL) { // im dumb ignore me i put the wrong sign :sob: ya
    std::cout << "stinky node D:" << std::endl; // bp here if the nodekind is something weird 
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

    push(Instr{.var = Comment{.comment = "Retrieve identifier: '" + e->name + "' located at " + res},
               .type = InstrType::Comment},
         Section::Main);

    pushDebug(e->line, expr->file_id, e->pos);

    // Push the result
    // Check for struct
    if (e->asmType->kind == ND_SYMBOL_TYPE) {
      if (structByteSizes.find(static_cast<SymbolType *>(e->asmType)->name) != structByteSizes.end()) {
        push(Instr{.var = LeaInstr { .size = DataSize::Qword, .dest = "%rcx", .src = res }, .type = InstrType::Lea}, Section::Main);
        pushRegister("%rcx");
        break;
      }
    }
    pushRegister(res);
    break;
  }
  case ND_STRING: {
    StringExpr *string = static_cast<StringExpr *>(expr);
    std::string label = "string" + std::to_string(stringCount++);

    pushDebug(string->line, expr->file_id, string->pos);
    // Push the label onto the stack
    push(Instr{.var=LeaInstr{.size=DataSize::Qword,.dest="%r13",.src=label+"(%rip)"},.type=InstrType::Lea},Section::Main);
    pushRegister("%r13");

    // define the string in the data section
    push(Instr{.var = Label{.name = label}, .type = InstrType::Label},
         Section::ReadonlyData);

    // Push the string onto the data section
    push(Instr{.var = AscizInstr{.what=string->value},
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
    push(Instr{.var=PushInstr{.what="%xmm0",.whatSize=DataSize::SS},.type=InstrType::Push},Section::Main);

    // define the string in the data section
    push(Instr{.var = Label{.name = label}, .type = InstrType::Label},
         Section::ReadonlyData);

    // Push the string onto the data section
    push(Instr{.var = DataSectionInstr{.bytesToDefine = DataSize::Dword /* long, aka 32-bits */, .what=std::to_string(convertFloatToInt(floating->value))},
               .type = InstrType::DB},
         Section::ReadonlyData);
    break;
  }
  case ND_BOOL: {
    BoolExpr *boolExpr = static_cast<BoolExpr *>(expr);
    // Technically just an int but SHHHHHHHH.............................
    pushRegister("$" + std::to_string((int)boolExpr->value));
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

  if (returnType->name == "int") {
    int lhsDepth = getExpressionDepth(e->lhs);
    int rhsDepth = getExpressionDepth(e->rhs);
    if (lhsDepth > rhsDepth) {
      visitExpr(e->lhs);
      visitExpr(e->rhs);
      popToRegister("%rbx"); // Pop RHS into RBX
      popToRegister("%rax"); // Pop LHS into RAX
    } else {
      visitExpr(e->rhs);
      visitExpr(e->lhs);
      popToRegister("%rax"); // Pop LHS into RAX
      popToRegister("%rbx"); // Pop RHS into RBX
    }

    // Perform the operation
    std::string op = lookup(opMap, e->op);
    if (op == "idiv" || op == "div" || op == "mod") {
      // Division requires special handling
      push(Instr {.var = DivInstr{.from = "%rbx"}, .type = InstrType::Div}, Section::Main);
      if (op == "mod")
        pushRegister("%rdx"); // Push remainder
      else
        pushRegister("%rax"); // Push result
    } else {
      // General binary operations
      push(Instr{.var = BinaryInstr{.op = op, .src = "%rbx", .dst = "%rax"},
                 .type = InstrType::Binary},
           Section::Main);
      pushRegister("%rax"); // Push the result
    }
    return; // Done
  } else if (returnType->name == "float") {
    // Similar logic for floats
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
    if (e->op == "+") op = "addss";
    if (e->op == "-") op = "subss";
    if (e->op == "*") op = "mulss";
    if (e->op == "/") op = "divss";

    push(Instr{.var = BinaryInstr{.op = op, .src = "%xmm1", .dst = "%xmm0"},
               .type = InstrType::Binary},
         Section::Main);
    pushRegister("%xmm0"); 
  } else if (returnType->name == "bool") {
    // We need to compare the two values
    // Check depth
    int lhsDepth = getExpressionDepth(e->lhs);
    int rhsDepth = getExpressionDepth(e->rhs);
    if (lhsDepth > rhsDepth) {
      visitExpr(e->lhs);
      visitExpr(e->rhs);
      popToRegister("%rbx"); // Pop RHS into RBX
      popToRegister("%rax"); // Pop LHS into RAX
    } else {
      visitExpr(e->rhs);
      visitExpr(e->lhs);
      popToRegister("%rax"); // Pop LHS into RAX
      popToRegister("%rbx"); // Pop RHS into RBX
    }
    // Get the operation
    std::string op = lookup(opMap, e->op);
    // Perform the operation
    // by running a comparison
    push(Instr{.var = CmpInstr{.lhs = "%rax", .rhs = "%rbx"}, .type = InstrType::Cmp}, Section::Main);
    pushLinker(op + " %al\n\tmovzbq %al, %rax\n\t", Section::Main); // There is no instruction like `cltq` for bytes
    // Move the bytes up to 64-bits
    pushRegister("%rax"); // Push the result
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

  if (e->op == "++" || e->op == "--") {
    // Perform the operation
    std::string res = (e->op == "++") ? "inc" : "dec";
    // Dear code reader, i apologize
    push(Instr {
      .var = LinkerDirective{ .value = res + "q " + whatWasPushed + "\n\t" },
      .type = InstrType::Linker   // Hey do you know why this infinatly loops? show asm
    }, Section::Main);
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
      std::cerr << "Too many arguments in call - consider reducing them or moving them to a globally defined space." << std::endl;
      exit(-1);
    }
    int offsetAmount = round(variableCount - 8, 8);
    if (offsetAmount) push(Instr{.var=SubInstr{.lhs="%rsp",.rhs="$"+std::to_string(offsetAmount)},.type=InstrType::Sub},Section::Main);
    int intArgCount = 0;
    int floatArgCount = 0;

    for (size_t i = 0; i < e->args.size(); i++) {
      // evaluate them
      visitExpr(e->args.at(i));
      // Check if the argument was a struct or array
      // That requires an lea, not a mov (that will be implicitly created here)
      bool isLea = false;
      if (e->args[i]->asmType->kind == ND_ARRAY_TYPE) {
        // lea should be false, because the value being processed is already technically a pointer
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
        PushInstr instr = std::get<PushInstr>(text_section.at(text_section.size() - 1).var);
        std::string whatWasPushed = instr.what;
        text_section.pop_back();
        // Now we can lea
        // ... IF what was pushed was an address ...
        if (whatWasPushed.find('(') == std::string::npos) {
          // run additional check to see if float or int
          if (getUnderlying(e->args[i]->asmType) == "float" && e->args[i]->asmType->kind == ND_SYMBOL_TYPE) {
            popToRegister(floatArgOrder[floatArgCount++]);
          } else {
            popToRegister(intArgOrder[intArgCount++]);
          }
        } else
          push(Instr{.var = LeaInstr{.size = DataSize::Qword, .dest = intArgOrder[i], .src = whatWasPushed},
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
    if (offsetAmount) push(Instr{.var=AddInstr{.lhs="%rsp",.rhs="$"+std::to_string(offsetAmount)},.type=InstrType::Sub},Section::Main);
    pushRegister("%rax");
  } else if (e->callee->kind == ND_MEMBER) {
    // Get the struct name
    MemberExpr *member = static_cast<MemberExpr *>(e->callee);
    std::string structName;
    if (member->lhs->kind == ND_INDEX) {
      structName = getUnderlying(static_cast<IndexExpr *>(member->lhs)->lhs->asmType);
    } else structName = getUnderlying(member->lhs->asmType);
    std::string fnName = static_cast<IdentExpr *>(member->rhs)->name;
    pushDebug(e->line, expr->file_id, e->pos);
    // Push each argument one by one.
    // Evaluate the struct
    visitExpr(member->lhs);
    // Note: Removed LEA Because when visiting a struct, we push %rcx, which alr contains the address :P
    popToRegister(intArgOrder[0]);
    int intArgCount = 1; // 1st is preserved for struct ptr above
    int floatArgCount = 0;
    int offsetAmount = round(variableCount-8, 8);
    if (offsetAmount) push(Instr{.var=SubInstr{.lhs="%rsp",.rhs="$"+std::to_string(offsetAmount)},.type=InstrType::Sub},Section::Main);
    for (size_t i = 0; i < e->args.size(); i++) {
      // evaluate them
      visitExpr(e->args.at(i));
      if (getUnderlying(e->args[i]->asmType) == "float" && e->args[i]->asmType->kind == ND_SYMBOL_TYPE) {
        popToRegister(floatArgOrder[floatArgCount++]);
      } else {
        popToRegister(intArgOrder[intArgCount++]);
      }
    }
    // Call the function
    push(Instr{.var = CallInstr{.name = "usrstruct_" + structName + "_" + fnName},
              .type = InstrType::Call,
              .optimize = false},
        Section::Main);
    if (offsetAmount) push(Instr{.var=AddInstr{.lhs="%rsp",.rhs="$"+std::to_string(offsetAmount)},.type=InstrType::Sub},Section::Main);
    pushRegister("%rax");
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
             .type = InstrType::Cmp},Section::Main);
  push(Instr{.var = JumpInstr{.op = JumpCondition::Equal,
                              .label = "ternaryFalse" + ternayCount},
             .type = InstrType::Jmp}, Section::Main);

  visitExpr(e->lhs);

  push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned,
                              .label = "ternaryEnd" + ternayCount},
             .type = InstrType::Jmp},Section::Main);
  push(Instr{.var = Label{.name = "ternaryFalse" + ternayCount},
             .type = InstrType::Label},Section::Main);

  visitExpr(e->rhs);

  push(Instr{.var = Label{.name = "ternaryEnd" + ternayCount},
             .type = InstrType::Label},Section::Main);
  ternay++;
}

void codegen::assign(Node::Expr *expr) {
  AssignmentExpr *e = static_cast<AssignmentExpr *>(expr);
  pushDebug(e->line, expr->file_id, e->pos);
  if (e->assignee->kind == ND_MEMBER || e->rhs->kind == ND_STRUCT) {
    assignStructMember(e);
    return;
  } else if (e->assignee->kind == ND_INDEX || e->rhs->kind == ND_ARRAY) {
    assignArray(e);
    return;
  };
  IdentExpr *lhs = static_cast<IdentExpr *>(e->assignee);
  visitExpr(e->rhs);
  std::string res = variableTable[lhs->name];
  popToRegister(res);
  pushRegister(res); // Expressions return values!
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
      std::string whereBytes = variableTable[static_cast<IdentExpr *>(e->lhs)->name];
      int offset = std::stoi(whereBytes.substr(0, whereBytes.find("("))); // this is the base of the array - the first byte of the first element
      offset -= (index->value * getByteSizeOfType(underlying));
      pushRegister(std::to_string(offset) + "(%rbp)");
    } else {
      visitExpr(e->lhs);
      // it is an array type so we must actually lea this!
      PushInstr instr = std::get<PushInstr>(text_section.at(text_section.size() - 1).var);
      std::string whatWasPushed = instr.what;
      text_section.pop_back();
      push(Instr{.var = LeaInstr{.size = DataSize::Qword, .dest = "%rcx", .src = whatWasPushed},
                 .type = InstrType::Lea},
           Section::Main);
      int byteSize = getByteSizeOfType(e->lhs->asmType);
      int offset = -index->value * byteSize;
      if (offset == 0)
        pushRegister("(%rcx)");
      else
        pushRegister(std::to_string(offset) + "(%rcx)");
    }
  } else {
    // This is a little more intricate.
    // We have to evaluate the index and multiply it by the size of the type
    if (e->lhs->kind == ND_IDENT) {
      push(Instr{.var = LeaInstr{.size=DataSize::Qword, .dest = "%rcx", .src = variableTable[static_cast<IdentExpr *>(e->lhs)->name]}, .type = InstrType::Lea}, Section::Main);
    } else {
      visitExpr(e->lhs);
      // lhs is always gonna be an array, but we need it to hold the addr
      PushInstr instr = std::get<PushInstr>(text_section.at(text_section.size() - 1).var);
      std::string whatWasPushed = instr.what;
      text_section.pop_back();
      push(Instr{.var = LeaInstr{.size = DataSize::Qword, .dest = "%rcx", .src = whatWasPushed},
                .type = InstrType::Lea},
          Section::Main);
      // %rcx contains the base of the array
    }
    visitExpr(e->rhs);
    popToRegister("%rax");
    // %rax contains the index
    // Negate rax
    pushLinker("negq %rax\n\t", Section::Main);
    // Multiply it by the size of the type
    int underlyingByteSize = getByteSizeOfType(static_cast<ArrayType *>(e->lhs->asmType)->underlying);
    switch (underlyingByteSize) {
      case 1: {
        // No need to multiply
        // Ex: []char or []bool
        pushRegister("(%rcx, %rax)");
        break;
      }
      case 2:
      case 4:
      case 8: {
        // Multiply by the size of the type
        pushRegister("(%rcx, %rax, " + std::to_string(underlyingByteSize) + ")");
        break;
      }

      default: {
        // Sad, we can't rely on little syntactical sugar of the assembler to cheat our way out :(
        push(Instr{.var = BinaryInstr{.op = "imul", .src = "$" + std::to_string(underlyingByteSize), .dst = "%rax"},
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
  pushDebug(e->line, expr->file_id, e->pos);

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
      // That means we cannot calculate byte sizes/offsets or any of that good stuff
      // since the struct wasn't valid.
      std::cerr << "Cannot access member of unknown struct" << std::endl;
      exit(-1);
    }
    // Now we can access the struct's fields
    int size = structByteSizes[structName].first;
    std::vector<StructMember> fields = structByteSizes[structName].second;
    // Eval lhs (this will put the struct's address onto the stack)
    visitExpr(e->lhs);
    // Pop the struct's address into a register
    popToRegister("%rcx");

    // Now, we have to do some crazy black magic shit like -8(%rcx) but we find what to replace '8' with.
    std::string fieldName = static_cast<IdentExpr *>(e->rhs)->name;
    
    // Right now, %rcx points to the beginning of the struct, which contains the last field.
    // In order to get the field we need, we should just loop over all the fields
    // and add to an offset.
    int offset = size;
    for (int i = fields.size(); i > 0; i--) {
      offset -= fields[i-1].second.second;
      if (fields[i-1].first == fieldName) break;
    }
    // submit the answer :D
    if (offset == 0) {
      pushRegister("(%rcx)");
    } else {
      pushRegister(std::to_string(offset) + "(%rcx)");
    }
    return;
  }

  if (e->lhs->kind == ND_INDEX) {
    visitExpr(e->lhs);
    // Should push the value of the array element. If not, then I'm dumb and it pushed its addr
    // Pop the value into a register
    popToRegister("%rcx");
    // Now we can access the member
    IndexExpr *index = static_cast<IndexExpr* >(e->lhs);
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
        for (int i = 0; i < fields.size(); i++) {
          if (fields[i].first == member->name) break;
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
  std::cerr << "Lonely struct expression just dangling here. That's not allowed!" << std::endl;
  exit(-1);
  return;
};

void codegen::addressExpr(Node::Expr *expr) {
  AddressExpr *e = static_cast<AddressExpr *>(expr);
  // It was a struct or something like that
  // Get the address of it. The address is the return type (becuase it's a pointer)
  Node::Expr *realRight = CompileOptimizer::optimizeExpr(e->right);
  visitExpr(realRight);
  // We don't want that push!
  PushInstr instr = std::get<PushInstr>(text_section.at(text_section.size() - 1).var);
  std::string whatWasPushed = instr.what;
  text_section.pop_back();
  // check if we did this to a struct

  if (realRight->kind == ND_IDENT) {
    IdentExpr *ident = static_cast<IdentExpr *>(realRight);

    if (structByteSizes.find(getUnderlying(ident->asmType)) != structByteSizes.end()) {
      // It was a struct!
      // It's very likely that what was pushed was "rcx" in this scenario.
      // That, however, just does not apply to the lea instruction.
      if (whatWasPushed == "%rcx") {
        pushRegister("%rcx");
        return;
      }
    }
  }
  push(Instr{.var = LeaInstr{.size = DataSize::Qword, .dest = "%rcx", .src = whatWasPushed},
             .type = InstrType::Lea},
       Section::Main);
  pushRegister("%rcx");
};

void codegen::assignStructMember(Node::Expr *expr) {
  AssignmentExpr *e = static_cast<AssignmentExpr *>(expr);
  if (e->assignee->kind == ND_MEMBER) {
    // Reassigning a struct's member.
    // The new value is already pushed onto the stack.
    // We just have to find where to pop it!
    MemberExpr *member = static_cast<MemberExpr *>(e->assignee);
    IdentExpr *lhs = static_cast<IdentExpr *>(member->lhs);
    IdentExpr *rhs = static_cast<IdentExpr *>(member->rhs);
    std::string asmName = static_cast<SymbolType *>(lhs->asmType)->name;
    std::string structTypeName = asmName;
    int size = structByteSizes[structTypeName].first;
    std::vector<StructMember> fields = structByteSizes[structTypeName].second;
    // Find the position of the field
    int offset = size;
    for (int i = fields.size(); i > 0; i--) {
      offset -= fields[i].second.second;
      if (fields[i].first == rhs->name) {
        break;
      }
    }
    // If it's a pointer, it's very simple.
    // This is if it's NOT simple...
    if (structByteSizes.find(getUnderlying(member->asmType)) != structByteSizes.end()
      && member->asmType->kind == ND_SYMBOL_TYPE
      && e->rhs->kind == ND_STRUCT) {
      // Find where the inner struct was previously stored
      // so we can override those values
      // step 1: find base address of the whole struct
      std::string base = variableTable[rhs->name];
      std::string subbedString = base.substr(0, base.find('('));
      int baseBytes = std::stoi(subbedString);
      // step 2: find the offset of the field
      int fieldOffset = offset;
      // step 3: order the fields
      std::vector<std::pair<std::string, Node::Expr *>> orderedFields;
      for (int i = 0; i < fields.size(); i++) {
        for (std::pair<Node::Expr *, Node::Expr *> field : static_cast<StructExpr *>(e->rhs)->values) {
          if (static_cast<IdentExpr *>(field.first)->name
              == fields.at(i).first) {
            orderedFields.push_back({fields.at(i).first, field.second});
            break;
          }
        }
      }
      // step 4: evaluate the ordered fields and store them in the inner struct
      for (int i = 0; i < orderedFields.size(); i++) {
        std::pair<std::string, Node::Expr *> field = orderedFields.at(i);
        // Evaluate the expression
        visitExpr(field.second);
        // Pop the value into a register
        std::string popExpr = std::to_string(baseBytes + offset + (fields.at(i).second.second)) + "(%rcx)";
        // optimizer bug fsr
        popToRegister(popExpr);
      }
      push(Instr{.var=LeaInstr{.size=DataSize::Qword,.dest="%rcx",.src=std::to_string(8 - (size + offset)) + "(%rbp)"},.type=InstrType::Lea},Section::Main);
      pushRegister("%rcx");
      return;
    } else {
      // We are setting a memberexpr to a ptr
      // ex. human.pet = dog;
      // Pop the value into a register by evaluating the lhs
      visitExpr(lhs);
      popToRegister("%rcx");
      visitExpr(e->rhs);
      std::string popExpr = std::to_string(offset) + "(%rcx)";
      popToRegister(popExpr);
      pushRegister(popExpr); // Return the value (assignments are exprs after all)
    }
    return;
  } else if (e->rhs->kind == ND_STRUCT) {
    // Just evaluate the struct expression but use the base of the already
    // defined struct as the base of the new one
    StructExpr *s = static_cast<StructExpr *>(e->rhs);
    std::string structTypeName = static_cast<SymbolType *>(s->asmType)->name;
    int size = structByteSizes[structTypeName].first;
    std::string base = variableTable[static_cast<IdentExpr *>(e->assignee)->name];
    std::vector<StructMember> fields = structByteSizes[structTypeName].second;
    // The fields of the expression might be out of order from which they are defined
    // in the struct. We need to reorder them.
    std::vector<std::pair<std::string, Node::Expr *>> orderedFields;
    for (int i = 0; i < fields.size(); i++) {
      for (std::pair<Node::Expr *, Node::Expr *> field : s->values) {
        if (static_cast<IdentExpr *>(field.first)->name
            == fields.at(i).first) {
          orderedFields.push_back({fields.at(i).first, field.second});
          break;
        }
      }
    }

    // Evaluate the orderedFields and store them in the struct!!!!
    std::string subbedString = base.substr(0, base.find('('));
    int baseBytes = std::stoi(subbedString);

    // Evaluate what rcx has to be
    visitExpr(e->assignee); // The identifier's position
    popToRegister("%rcx");
    for (int i = 0; i < orderedFields.size(); i++) {
      std::pair<std::string, Node::Expr *> field = orderedFields.at(i);
      // Evaluate the expression
      visitExpr(field.second);
      // Pop the value into a register
      int popToOffset = 0;
      if (i != 0) for (int j = 0; j < i; j++) {
        popToOffset += fields[j].second.second;
      }
      std::string popExpr = std::to_string(-popToOffset) + "(%rcx)";
      // optimizer bug fsr
      popToRegister(popExpr);
    }
    push(Instr{.var=LeaInstr{.size=DataSize::Qword,.dest="%rcx",.src=variableTable[static_cast<IdentExpr *>(e->assignee)->name]},.type=InstrType::Lea},Section::Main);
    pushRegister("%rcx");

  }
};

void codegen::assignArray(Node::Expr *expr) {
  AssignmentExpr *assign = static_cast<AssignmentExpr *>(expr);
  if (assign->assignee->kind == ND_INDEX) {
    IndexExpr *e = static_cast<IndexExpr *>(assign->assignee);
    IntExpr *index = static_cast<IntExpr *>(e->rhs);
    int idx = index->value;
    // lhs is likely an identifier to an array
    // it doesn't matter for this example, though
    visitExpr(e->lhs);
    popToRegister("%rcx"); // rcx now has the base of the array
    int size = getByteSizeOfType(e->lhs->asmType);
    int offset = idx * size;
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
      PushInstr instr = std::get<PushInstr>(text_section.at(text_section.size() - 1).var);
      text_section.pop_back();
      push(Instr{.var=LeaInstr{.size=DataSize::Qword,.dest="%rcx",.src=instr.what}, .type = InstrType::Lea},Section::Main);
      ArrayExpr *rhs = static_cast<ArrayExpr *>(assign->rhs);
      for (int i = 0; i < rhs->elements.size(); i++) {
        visitExpr(rhs->elements.at(i));
        popToRegister(std::to_string(-i * getByteSizeOfType(rhs->type)) + "(%rcx)");
      }
      // push the base of the array
      pushRegister("%rcx");
      return;
    }
  }
  // I think that's it??!!!
};

void codegen::nullExpr(Node::Expr *expr) {
  // Has implicit value of 0.
  pushRegister("$0");
  return;  
}