#include "gen.hpp"
#include "optimize.hpp"
#include <sys/cdefs.h>

void codegen::visitStmt(Node::Stmt *stmt) {
	auto handler = lookup(stmtHandlers, stmt->kind);
	if (handler) {
		handler(stmt);
	}
}

void codegen::expr(Node::Stmt *stmt) {
	auto expr = static_cast<ExprStmt *>(stmt);
	visitExpr(expr->expr);
}

void codegen::program(Node::Stmt *stmt) {
	auto program = static_cast<ProgramStmt *>(stmt);
	
	for (auto &s : program->stmt) {
		visitStmt(s);
	}
}

void codegen::constDecl(Node::Stmt *stmt) {
	auto constDecl = static_cast<ConstStmt *>(stmt);
	visitStmt(constDecl->value);
}

void codegen::funcDecl(Node::Stmt *stmt) {
	auto funcDecl = static_cast<fnStmt *>(stmt);
	// Declare the type of this function (important)

	std::string funcName = "usr_" + funcDecl->name;
	int preStackSize = stackSize;
	push(Instr{.var = LinkerDirective{.value=".type " + funcName + ", @function\n"}, .type = InstrType::Linker}, Section::Main);
	push(Instr{.var = Label{.name = funcName}, .type = InstrType::Label},
				Section::Main);

	// push arguments to the stack
	for (auto &args : funcDecl->params) {
		stackTable.insert({args.first, stackSize});
	}

	push(Instr{.var = LinkerDirective{.value=".cfi_startproc\n\t"}, .type = InstrType::Linker}, Section::Main);
	push(Instr{.var=PushInstr{ .what = "%rbp", .whatSize = DataSize::Qword }, .type = InstrType::Mov}, Section::Main);
	stackSize++;
	push(Instr{.var=MovInstr{ .dest = "%rbp", .src = "%rsp", .destSize = DataSize::Qword, .srcSize = DataSize::Qword }, .type = InstrType::Mov}, Section::Main);
	visitStmt(funcDecl->block);
	stackSize = preStackSize;

	// for (auto &arg : funcDecl->params) {
	// }

	// Yes, AFTER the ret! and in the main function as well!
	push(Instr{.var = LinkerDirective{.value=".cfi_endproc\n\t"}, .type = InstrType::Linker}, Section::Main);
	push(Instr{.var = LinkerDirective{.value=".size " + funcName + ", .-" + funcName + "\n\t"}, .type = InstrType::Linker}, Section::Main);
	// return in rax
}

void codegen::varDecl(Node::Stmt *stmt) {
	auto varDecl = static_cast<VarStmt *>(stmt);

	push(Instr{.var =
								 Comment{.comment = "define variable '" + varDecl->name + "'"},
						 .type = InstrType::Comment},
			 Section::Main);

	visitExpr(static_cast<ExprStmt *>(varDecl->expr)->expr);

	// add variable to the stack
	stackTable.insert({varDecl->name, stackSize});
}

void codegen::block(Node::Stmt *stmt) {
	auto block = static_cast<BlockStmt *>(stmt);
	stackSizesForScopes.push_back(stackSize);
	size_t scopeCount = stackSizesForScopes.size();
	for (auto &s : block->stmts) {
		visitStmt(s);
	}
	// If difference is zero, it will skip!
	if (stackSizesForScopes.size() < scopeCount)
		return; // A function must have returned, meaning rsp and stuff will already be handled
	if (int diff = stackSize - stackSizesForScopes.at(stackSizesForScopes.size() - 1)) {
		push(Instr{ .var = AddInstr { .lhs = "%rsp", .rhs = "$" + std::to_string(8 * diff) }, .type = InstrType::Add }, Section::Main);

		stackSize = stackSizesForScopes.at(stackSizesForScopes.size() - 1);
	}
}

void codegen::print(Node::Stmt *stmt) {
	auto print = static_cast<PrintStmt *>(stmt);

	push(Instr{.var = Comment{.comment = "print stmt"}}, Section::Main);
	nativeFunctionsUsed[NativeASMFunc::strlen] = true;
	for (auto &arg : print->args) {
		visitExpr(arg);
		// assume type-checker worked properly and a string is passed in
		push(Instr{.var = PopInstr({.where = "%rsi", .whereSize = DataSize::Qword}), .type = InstrType::Pop},
				 Section::Main);
		stackSize--;

		// calculate string length using native function
		push(Instr{.var = MovInstr{.dest = "%rdi", .src = "%rsi", .destSize = DataSize::Qword, .srcSize = DataSize::Qword},
							 .type = InstrType::Mov},
				 Section::Main);
		push(Instr{.var = CallInstr{.name = "native_strlen"}}, Section::Main);

		auto str = static_cast<StringExpr *>(arg);
		push(Instr{.var = MovInstr({.dest = "%rdx", .src = "%rax", .destSize = DataSize::Qword, .srcSize = DataSize::Qword}),
							 .type = InstrType::Mov},
				 Section::Main);

		// syscall id for write on x86 is 1
		push(Instr{.var = MovInstr({.dest = "%rax", .src = "$1", .destSize = DataSize::Qword, .srcSize = DataSize::Qword}),
							 .type = InstrType::Mov},
				 Section::Main);

		// set rdi to 1 (file descriptor for stdout)
		push(Instr{.var = MovInstr({.dest = "%rdi", .src = "$1", .destSize = DataSize::Qword, .srcSize = DataSize::Qword}),
							 .type = InstrType::Mov},
				 Section::Main);

		// create call
		push(Instr{.var = Syscall({.name = "SYS_WRITE"}),
							 .type = InstrType::Syscall},
				 Section::Main);
	}
}

void codegen::ifStmt(Node::Stmt *stmt) {
	auto ifstmt = static_cast<IfStmt *>(stmt);
	push(Instr{.var = Comment{.comment = "if statment"},
						 .type = InstrType::Comment},
			 Section::Main);

	std::string preConditionalCount = std::to_string(++conditionalCount);

	// visit the expr, jump if not zero
	visitExpr(ifstmt->condition);

	// pop value somewhere relatively unused, that is unlikely to be overriden
	// somewhere else
	push(Instr{.var = PopInstr{.where = "%rcx"}, .type = InstrType::Pop},
			 Section::Main);
	stackSize--;
	// NOTE: This is not a directive! Test Instr does not exist, so pushing plaintext is easier
	push(Instr{.var = LinkerDirective{.value = "test %rcx, %rcx\n\t"}, .type = InstrType::Cmp},
			 Section::Main);
	push(Instr{.var = JumpInstr{.op = JumpCondition::NotZero,
															.label = ("conditional" + preConditionalCount)},
						 .type = InstrType::Jmp},
			 Section::Main);
	// Evaluate else 
	if (ifstmt->elseStmt != nullptr) {
		visitStmt(ifstmt->elseStmt);
	}
	push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned,
															.label = "main" + preConditionalCount},
						 .type = InstrType::Jmp},
			 Section::Main);

	push(Instr{.var = Label{.name = "conditional" + preConditionalCount},
						 .type = InstrType::Label},
			 Section::Main);
	visitStmt(ifstmt->thenStmt);
	push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned,
															.label = "main" + preConditionalCount},
						 .type = InstrType::Jmp},
			 Section::Main);

	push(Instr{.var = Label{.name = "main" + preConditionalCount},
						 .type = InstrType::Label},
			 Section::Main);
}

void codegen::_return(Node::Stmt *stmt) {
	auto returnStmt = static_cast<ReturnStmt *>(stmt);
	// the "main" function is handled as a user function- we dont need to make any exceptions
	// check if we have an if return
	if (returnStmt->stmt != nullptr) {
		visitStmt(returnStmt->stmt);

		// pop the expression we just visited
		push(Instr{.var = PopInstr{.where = "%rax", .whereSize = DataSize::Qword}, .type = InstrType::Pop},
				 Section::Main);
		stackSize--;
		stackSizesForScopes.pop_back();
		push(Instr{.var = MovInstr { .dest = "%rsp", .src = "%rbp", .destSize = DataSize::Qword, .srcSize = DataSize::Qword }, .type = InstrType::Mov}, Section::Main);
		push(Instr{.var = PopInstr { .where = "%rbp", .whereSize = DataSize::Qword }, .type = InstrType::Pop}, Section::Main);
		stackSize--;
		push(Instr{.var = Ret{}, .type = InstrType::Ret}, Section::Main);
		return;
	}

	visitExpr(returnStmt->expr);
	push(Instr{.var = PopInstr{.where = "%rax", .whereSize = DataSize::Qword}, .type = InstrType::Pop},
			 Section::Main);
	stackSize--;
	stackSizesForScopes.pop_back();
	push(Instr{.var = MovInstr { .dest = "%rsp", .src = "%rbp", .destSize = DataSize::Qword, .srcSize = DataSize::Qword }, .type = InstrType::Mov}, Section::Main);
	push(Instr{.var = PopInstr { .where = "%rbp", .whereSize = DataSize::Qword }, .type = InstrType::Pop}, Section::Main);
	stackSize--;
	push(Instr{.var = Ret{}, .type = InstrType::Ret}, Section::Main);
}

void codegen::whileLoop(Node::Stmt *stmt) {
	WhileStmt *loop = static_cast<WhileStmt *>(stmt);

	// Generate unique labels for the loop
	std::string startLabel = "while" + std::to_string(loopCount);
	std::string endLabel = "endwhile" + std::to_string(loopCount);

	// Create the start label for the loop
	push(Instr{.var = Label{.name = startLabel}, .type = InstrType::Label},
			 Section::Main);

	// Evaluate the loop condition again after increment
	visitExpr(loop->condition);

	// Pop the result of the condition into rax
	push(Instr{.var = PopInstr{.where = "%rax", .whereSize = DataSize::Qword}, .type = InstrType::Pop},
			 Section::Main);
	stackSize--;

	// Compare rax to 0 (to decide whether to continue or break the loop)
	push(Instr{.var = CmpInstr{.lhs = "%rax", .rhs = "0"}, .type = InstrType::Cmp},
			 Section::Main);

	// Jump to end if the condition is false
	push(Instr{.var = JumpInstr{.op = JumpCondition::Equal, .label = endLabel},
						 .type = InstrType::Jmp},
			 Section::Main);

	// check if we have an optional condition
	if (loop->optional != nullptr) {
		visitExpr(loop->optional);
	}

	// Visit the loop body
	visitStmt(loop->block);

	// Unconditionally jump back to the start of the loop
	push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned,
															.label = startLabel},
						 .type = InstrType::Jmp},
			 Section::Main);

	// Create the end label for the loop
	push(Instr{.var = Label{.name = endLabel}, .type = InstrType::Label},
			 Section::Main);

	// Increment the loop count for future loops
	loopCount++;
}