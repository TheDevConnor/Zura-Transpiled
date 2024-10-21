#include "gen.hpp"
#include "optimize.hpp"

void codegen::visitExpr(Node::Expr *expr) {
  auto handler = lookup(exprHandlers, expr->kind);
  if (handler) {
    handler(expr);
  }
}

// A basic "fallback" type of expression that covers all 1-D, basic expr's
void codegen::primary(Node::Expr *expr) {
  // TODO: Implement the primary expression
  switch (expr->kind) {
  case NodeKind::ND_INT: {
    auto e = static_cast<IntExpr *>(expr);
    pushRegister("$" + std::to_string(e->value));
    break;
  }
  case NodeKind::ND_IDENT: {
    auto e = static_cast<IdentExpr *>(expr);
    int offset = variableTable[e->name];

    auto res = std::to_string(offset * -8) + "(%rbp)";

    push(Instr{.var = Comment{.comment = "Clone variable '" + e->name +
                                         "' for use as expr (offset: " +
                                         std::to_string(offset) + ")"},
               .type = InstrType::Comment},
         Section::Main);

    // Push the result (the retrieved data)
    pushRegister(res);
    break;
  }
  case ND_STRING: {
    auto string = static_cast<StringExpr *>(expr);
    std::string label = "string" + std::to_string(stringCount++);

    // Push the label onto the stack
    pushRegister("$" + label);

    // define the string in the data section
    push(Instr{.var = Label{.name = label}, .type = InstrType::Label},
         Section::Data);

    // Push the string onto the data section
    push(Instr{.var = AscizInstr{.what=string->value},
               .type = InstrType::Asciz},
         Section::Data);
    break;
  }
  case ND_FLOAT: {
    auto floating = static_cast<FloatExpr *>(expr);
    std::string label = "float" + std::to_string(floatCount++);

    // Push the label onto the stack
    moveRegister("%xmm0", label + "(%rip)", DataSize::SS, DataSize::SS);
    push(Instr{.var=PushInstr{.what="%xmm0",.whatSize=DataSize::SS},.type=InstrType::Push},Section::Main);

    // define the string in the data section
    push(Instr{.var = Label{.name = label}, .type = InstrType::Label},
         Section::Data);

    // Push the string onto the data section
    push(Instr{.var = DataSectionInstr{.bytesToDefine = DataSize::Dword /* long */, .what=std::to_string(convertFloatToInt(floating->value))},
               .type = InstrType::DB},
         Section::Data);
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
  auto e = static_cast<BinaryExpr *>(expr);
  bool isAddition = (e->op == "+" || e->op == "-");

  std::string lhs_reg = isAddition ? "%rbx" : "%rax";
  std::string rhs_reg = isAddition ? "%rcx" : "%rbx";

  visitExpr(e->lhs);
  visitExpr(e->rhs);



  // Perform the binary operation
  std::string op = lookup(opMap, e->op);

  SymbolType *lhsType = static_cast<SymbolType *>(e->lhs->asmType);
  SymbolType *rhsType = static_cast<SymbolType *>(e->rhs->asmType);
  if (lhsType->name == "str" || rhsType->name == "str") {
    std::cerr << "excuse me you cant concat strings :(" << std::endl;
    exit(-1);
  }
  if (lhsType->name == "float" || rhsType->name == "float") {
    lhs_reg = "%xmm0";
    rhs_reg = "%xmm1";
    // Pop the right hand side
    popToRegister(rhs_reg);

    // Pop the left hand side
    popToRegister(lhs_reg);
    // Perform the operation
    if (op == "add") op = "addss";
    if (op == "sub") op = "subss";
    if (op == "mul") op = "mulss";
    if (op == "div") op = "divss";
    // Ignore the others because nobody cares about them HAHAH
    push(Instr{.var = BinaryInstr{.op = op, .src = rhs_reg, .dst = lhs_reg},
               .type = InstrType::Binary},
         Section::Main);
  }
  if (lhsType->name == "int" && rhsType->name == "int") {
      // Pop the right hand side
    popToRegister(rhs_reg);

    // Pop the left hand side
    popToRegister(lhs_reg);
    // Use regular ones! You'll be fine.
    // Modulo (op == mod) is division but push rdx instead
    if (op == "mod") {
      push(Instr{.var = MovInstr{.dest = "%rdx", .src = "%rax"},
                 .type = InstrType::Mov},
           Section::Main);
    } else {
      push(Instr{.var = BinaryInstr{.op = op, .src = rhs_reg, .dst = lhs_reg},
                 .type = InstrType::Binary},
           Section::Main);
    }
  }
  // Push the result
  pushRegister(lhs_reg);
}

void codegen::unary(Node::Expr *expr) {
  auto e = static_cast<UnaryExpr *>(expr);
  std::cerr
      << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)"
      << std::endl;
  std::cerr << "Unary expression not implemented! (Op: " << e->op
            << ", Expr: " << e->expr->kind << ")" << std::endl;
  exit(-1);
}

void codegen::cast(Node::Expr *expr) {
  auto e = static_cast<CastExpr *>(expr);
  SymbolType *toType = static_cast<SymbolType *>(e->castee_type);
  SymbolType *fromType = static_cast<SymbolType *>(e->castee->asmType);
  if (fromType->name == "str") {
    std::cerr << "Explicitly casting from string is not allowed" << std::endl;
    exit(-1);
  }
  visitExpr(e->castee);
  if (toType->name == "float") {
    // We are casting into a float.
    popToRegister("%rax"); // This actually doesnt matter but rax is fast and also our garbage disposal
    // Perform the conversion
    if (fromType->name == "int") {
      push(Instr{.var = ConvertInstr{.convType = ConvertType::SI2SS, .from = "%rax", .to = "%xmm1"},
                .type = InstrType::Convert},
          Section::Main);
      // push xmm0
      push(Instr{.var=PushInstr{.what="%xmm0",.whatSize=DataSize::SS},.type=InstrType::Push},Section::Main);
    }
    if (fromType->name == "float") {
      // Do nothing, it's already a float
      // Optimizer will know that this redundant push/pop is, of course, redundant
      pushRegister("%rax");
    }
  } else if (toType->name == "int") {
    if (fromType->name == "int") {
      // Do nothing, it's already an int
      return;
    }

    if (fromType->name == "float") {
      popToRegister("%xmm0");
      // Perform the conversion (always truncate for now)
      push(Instr{.var = ConvertInstr{.convType = ConvertType::TSS2SI, .from = "%xmm0", .to = "%rax"},
                .type = InstrType::Convert},
          Section::Main);
      // push rax - its ok becasue rax holds an int now! it fits on our int-based stack! woohoo!!!!
      pushRegister("%rax");
    }
  }
}

void codegen::grouping(Node::Expr *expr) {
  auto e = static_cast<GroupExpr *>(expr);
  // Visit the expression inside the grouping
  visitExpr(e->expr);
}

void codegen::call(Node::Expr *expr) {
  auto e = static_cast<CallExpr *>(expr);
  auto n = static_cast<IdentExpr *>(e->callee);
  // Push each argument one by one.
  for (auto p : e->args) {
    // evaluate them
    codegen::visitExpr(p);
  }
  // Call the function
  push(Instr{.var = CallInstr{.name = "usr_" + n->name},
             .type = InstrType::Call,
             .optimize = false},
       Section::Main);
  pushRegister("%rax");
}

void codegen::ternary(Node::Expr *expr) {
  auto e = static_cast<TernaryExpr *>(expr);
  std::cerr
      << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)"
      << std::endl;
  std::cerr << "Ternary expression not implemented!" << std::endl;
  exit(-1);
}

void codegen::assign(Node::Expr *expr) {
  auto e = static_cast<AssignmentExpr *>(expr);
  auto lhs = static_cast<IdentExpr *>(e->assignee);
  visitExpr(e->rhs);
  int offset = variableTable[lhs->name];
  auto res = std::to_string(offset * -8) + "(%rbp)";
  popToRegister("%rax");
}

void codegen::_arrayExpr(Node::Expr *expr) {
  auto e = static_cast<ArrayExpr *>(expr);
  std::cerr
      << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)"
      << std::endl;
  std::cerr << "Array expression not implemented! (Type: NodeKind["
            << (int)e->type->kind << "])" << std::endl;
  exit(-1);
}

void codegen::arrayElem(Node::Expr *expr) {
  auto e = static_cast<IndexExpr *>(expr);
  std::cerr
      << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)"
      << std::endl;
  std::cerr << "Array element not implemented! (LHS: NodeKind["
            << (int)e->lhs->kind << "], RHS: NodeKind[" << (int)e->rhs->kind
            << "])" << std::endl;
  exit(-1);
}