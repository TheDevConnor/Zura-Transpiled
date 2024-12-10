#include "gen.hpp"
#include "optimizer/instr.hpp"
#include "optimizer/compiler.hpp"
#include <string>

void codegen::visitExpr(Node::Expr *expr) {
  // Optimize the expression before we handle it!
  // Node::Expr *realExpr = CompileOptimizer::optimizeExpr(expr);
  Node::Expr *realExpr = expr;
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
  }
  case NodeKind::ND_IDENT: {
    IdentExpr *e = static_cast<IdentExpr *>(expr);
    std::string res = variableTable[e->name];

    push(Instr{.var = Comment{.comment = "Retrieve identifier: '" + e->name + "' located at " + res},
               .type = InstrType::Comment},
         Section::Main);

    pushDebug(e->line, expr->file_id, e->pos);

    // Push the result
    // Check for struct
    if (getUnderlying(e->asmType).find("struct") == 0) {
      // It is a struct type!
      // We MUST use lea here.
      // Like seriously, this is its JOB.
      push(Instr{.var = LeaInstr { .size = DataSize::Qword, .dest = "%rcx", .src = res }, .type = InstrType::Lea}, Section::Main);
      pushRegister("%rcx");
    } else {
      pushRegister(res);
    }
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
  } 
  else if (returnType->name == "float") {
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

void codegen::cast(Node::Expr *expr) {
  CastExpr *e = static_cast<CastExpr *>(expr);
  SymbolType *toType = static_cast<SymbolType *>(e->castee_type);
  SymbolType *fromType = static_cast<SymbolType *>(e->castee->asmType);

  if (fromType->name == "str") {
    std::cerr << "Explicitly casting from string is not allowed" << std::endl;
    exit(-1);
  }
  pushDebug(e->line, expr->file_id, e->pos);
  visitExpr(e->castee);
  if (fromType->name == "unknown") {
    return; // Assume the dev knew what they were doing and push the original value
  }
  if (toType->name == "enum") {
    // Enums are truly just ints under-the-hood
    if (fromType->name != "int") {
      std::cerr << "Cannot cast non-int to enum" << std::endl;
      exit(-1);
    }
    return; // Do nothing (essentially just a void cast)
  }
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
    if (fromType->name == "int" || fromType->name == "enum") {
      // Do nothing, it's already an int
      // Enums are explicitly ints under-the-hood as well
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
  GroupExpr *e = static_cast<GroupExpr *>(expr);
  // Visit the expression inside the grouping
  pushDebug(e->line, expr->file_id, e->pos);
  visitExpr(e->expr);
}

void codegen::call(Node::Expr *expr) {
  CallExpr *e = static_cast<CallExpr *>(expr);
  IdentExpr *n = static_cast<IdentExpr *>(e->callee);
  pushDebug(e->line, expr->file_id, e->pos);
  // Push each argument one by one.
  if (e->args.size() > argOrder.size()) {
    std::cerr << "Too many arguments in call - consider reducing them or moving them to a globally defined space." << std::endl;
    exit(-1);
  }
  for (size_t i = 0; i < e->args.size(); i++) {
    // evaluate them
    codegen::visitExpr(e->args.at(i));
    popToRegister(argOrder[i]);
  }
  // Call the function
  push(Instr{.var = CallInstr{.name = "usr_" + n->name},
             .type = InstrType::Call,
             .optimize = false},
       Section::Main);
  pushRegister("%rax");
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

void codegen::externalCall(Node::Expr *expr) {
  // Basically like a normal function call
  // ... Minus the "usr_" prefix
  ExternalCall *e = static_cast<ExternalCall *>(expr);
  pushDebug(e->line, expr->file_id, e->pos);
  // Push each argument one by one.
  if (e->args.size() > argOrder.size()) {
    std::cerr << "Too many arguments in call - consider reducing them or moving them to a globally defined space." << std::endl;
    exit(-1);
  }
  for (size_t i = 0; i < e->args.size(); i++) {
    // evaluate them
    codegen::visitExpr(e->args.at(i));
    popToRegister(argOrder[i]);
  }
  // Call the function
  push(Instr{.var = CallInstr{.name = e->name + "@PLT"},
             .type = InstrType::Call},
       Section::Main);
  pushRegister("%rax");
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
  ArrayExpr *array = static_cast<ArrayExpr *>(e->lhs);
  if (e->rhs->kind == ND_INT) {
    // It's a constant index
    IntExpr *index = static_cast<IntExpr *>(e->rhs);
    Node::Type *underlying = static_cast<ArrayType *>(array->type)->underlying;
    if (e->lhs->kind == ND_IDENT) {
      std::string whereBytes = variableTable[static_cast<IdentExpr *>(e->lhs)->name];
      int offset = std::stoi(whereBytes.substr(0, whereBytes.find("("))); // this is the base of the array - the first byte of the first element
      offset -= (index->value * getByteSizeOfType(underlying));
      pushRegister(std::to_string(offset) + "(%rbp)");
    } else {
      visitExpr(e->lhs);
      popToRegister("%rcx");
      int byteSize = getByteSizeOfType(underlying);
      int offset = -index->value * byteSize;
      if (offset == 0)
        pushRegister("(%rcx)");
      else
        pushRegister(std::to_string(offset) + "(%rcx)");
    }
  } else {
    // This is a little more intricate.
    // We have to evaluate the index and multiply it by the size of the type
    visitExpr(e->lhs);
    popToRegister("%rcx");
    // %rcx contains the base of the array
    visitExpr(e->rhs);
    popToRegister("%rax");
    // %rax contains the index
    // Negate rax
    pushLinker("negq %rax", Section::Main);
    // Multiply it by the size of the type
    ArrayExpr *array = static_cast<ArrayExpr *>(e->lhs);
    int byteSize = getByteSizeOfType(array->type);
    switch (byteSize) {
      case 1: {
        // No need to multiply
        pushRegister("(%rcx, %rax)");
        break;
      }
      case 2:
      case 4:
      case 8: {
        // Multiply by the size of the type
        pushRegister("(%rcx, %rax, " + std::to_string(byteSize) + ")");
        break;
      }

      default: {
        // Sad, we can't rely on little syntactical sugar of the assembler to cheat our way out :(
        push(Instr{.var = BinaryInstr{.op = "imul", .src = "$" + std::to_string(byteSize), .dst = "%rax"},
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
  auto e = static_cast<ArrayExpr *>(expr);
  pushDebug(e->line, expr->file_id, e->pos); // k ping me on discord if you need help, i am going to be studying for a bit

}

void codegen::memberExpr(Node::Expr *expr) {
  MemberExpr *e = static_cast<MemberExpr *>(expr);
  pushDebug(e->line, expr->file_id, e->pos);
  // If lhs is enum
  std::string lhsName = static_cast<SymbolType *>(e->lhs->asmType)->name;
  if (lhsName == "enum") {
    IdentExpr *lhs = static_cast<IdentExpr *>(e->lhs);
    IdentExpr *rhs = static_cast<IdentExpr *>(e->rhs);
    pushRegister("$enum_" + lhs->name + "_" + rhs->name);
    return;
  }

  // Check if starts with "struct-"
  if (lhsName.find("struct-") == 0) {
    // It's a struct
    std::string structName = lhsName.substr(7);
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
    for (int i = 0; i < fields.size(); i++) {
      if (fields[i].first == fieldName) {
        // Check if the type of the member is another struct
        Node::Type *fieldType = fields[i].second.first;
        if (fieldType->kind == ND_SYMBOL_TYPE && (structByteSizes.find(getUnderlying(fieldType)) != structByteSizes.end())) {
          // It was a struct!
          // We have to return the ADDRESS of this node here!
          int offset = 0;
          if (i != 0) for (int j = 0; j < i; j++) {
            offset += fields[i].second.second;
          }
          push(Instr{.var=LeaInstr{.size=DataSize::Qword,.dest="%rcx",.src=std::to_string(offset)+"(%rcx)"}, .type=InstrType::Lea},Section::Main);
          pushRegister("%rcx");
          return;
        }
        // push the value at this offset
        int offset = 0;
        if (i != 0) for (int j = 0; j < i; j++) {
          offset += fields[i].second.second;
        }
        if (offset == 0)
          pushRegister("(%rcx)");
        else
          pushRegister(std::to_string(offset) + "(%rcx)");
        break;
      }
    }
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
  std::cerr << "Lonely struct expression just dangling here. That's not allowed!" << std::endl;
  exit(-1);
  return;
};

void codegen::addressExpr(Node::Expr *expr) {
  AddressExpr *e = static_cast<AddressExpr *>(expr);
  if (getByteSizeOfType(e->right->asmType) <= 8) {
    // It is small enough to fit in a register or stack
    // so it can be passed by value (pointing to it is useless)

    // NOTE: This also makes pointer pointers (**int)s useless and optimizable!
    visitExpr(e->right);
    return;
  }

  // It was a struct or something like that
  // Get the address of it. The address is the return type (becuase it's a pointer)
  visitExpr(e->right);
  // Good enough
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
    std::string structTypeName = asmName.substr(7);
    int size = structByteSizes[structTypeName].first;
    std::vector<StructMember> fields = structByteSizes[structTypeName].second;
    // Find the position of the field
    int offset = 0;
    for (int i = 0; i < fields.size(); i++) {
      if (fields[i].first == rhs->name) {
        offset = fields[i].second.second;
        break;
      }
    }
    // If it's a pointer, it's very simple.
    // This is if it's NOT simple...
    if (member->asmType->kind == ND_STRUCT
      && getUnderlying(member->asmType).find("struct") == 0) {
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
        std::string popExpr = std::to_string(baseBytes + offset + (-8 + fields.at(i).second.second)) + "(%rcx)";
        // optimizer bug fsr
        popToRegister(popExpr);
      }
      push(Instr{.var=LeaInstr{.size=DataSize::Qword,.dest="%rcx",.src=std::to_string(8 - (size + offset)) + "(%rbp)"},.type=InstrType::Lea},Section::Main);
      pushRegister("%rcx");
      return;
    } else {
      // We are setting a memberexpr to a ptr
      // ex. human.pet = dog;
      // Get 'dog' value (the location of the struct)
      visitExpr(e->rhs);
      // Pop the value into a register
      int baseBytes = std::stoi(variableTable[lhs->name].substr(0, variableTable[lhs->name].find('(')));
      std::string popExpr = std::to_string(baseBytes + 8 - offset) + "(%rbp)";
      // optimizer bug fsr
      popToRegister(popExpr);
    }
    // Push what's in the field
    int baseBytes = std::stoi(variableTable[lhs->name].substr(0, variableTable[lhs->name].find('(')));
    pushRegister(std::to_string(baseBytes + 8 - (size + offset)) + "(%rbp)");
    return;
  } else if (e->rhs->kind == ND_STRUCT) {
    // Just evaluate the struct expression but use the base of the already
    // defined struct as the base of the new one
    StructExpr *s = static_cast<StructExpr *>(e->rhs);
    std::string structTypeName = static_cast<SymbolType *>(s->asmType)->name.substr(7);
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
    for (int i = 0; i < orderedFields.size(); i++) {
      std::pair<std::string, Node::Expr *> field = orderedFields.at(i);
      // Evaluate the expression
      visitExpr(field.second);
      // Pop the value into a register
      std::string popExpr = std::to_string(baseBytes + (-8 + fields.at(i).second.second)) + "(%rcx)";
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