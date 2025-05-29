#include "gen.hpp"
#include "../typeChecker/type.hpp"
#include <limits>

std::string codegen::dwarf::generateAbbreviations() {
  std::string abbreviations = "";
  for (DIEAbbrev a : dieAbbrevsUsed) {
    switch (a) {
      case DIEAbbrev::FunctionNoParams: {
        abbreviations += "\n.uleb128 " + std::to_string((int)a) +
                         "\n.uleb128 0x2e # TAG_subprogram - FunctionNoParams, non-void"
                         "\n.byte 0x1 # Has children"
                         "\n.uleb128 0x3f # AT_external"
                         "\n.uleb128 0xc # FORM_flag_present"
                         "\n.uleb128 0x3 # AT_name"
                         "\n.uleb128 0xe # FORM_strp"
                         "\n.uleb128 0x3a # AT_decl_file"
                         "\n.uleb128 0xb # FORM_data1"
                         "\n.uleb128 0x3b # AT_decl_line"
                         "\n.uleb128 0xb # FORM_data1"
                         "\n.uleb128 0x39 # AT_decl_column"
                         "\n.uleb128 0xb # FORM_data1"
                         "\n.uleb128 0x49 # AT_type" 
                         "\n.uleb128 0x13 # FORM_ref4"
                         "\n.uleb128 0x11 # AT_low_pc"
                         "\n.uleb128 0x1 # FORM_addr" 
                         "\n.uleb128 0x12 # AT_high_pc"
                         "\n.uleb128 0x7 # FORM_data8"
                         "\n.uleb128 0x40 # AT_frame_base"
                         "\n.uleb128 0x18 # FORM_exprloc"
                         "\n.uleb128 0x7a # AT_call_all_calls"
                         "\n.uleb128 0x19 # FORM_flag_present"
                         "\n";
        break;
      }
      case DIEAbbrev::FunctionNoParamsVoid: {
        abbreviations += "\n.uleb128 " + std::to_string((int)a) +
                         "\n.uleb128 0x2e # TAG_subprogram - FunctionNoParams, non-void"
                         "\n.byte 0x1 # Has children"
                         "\n.uleb128 0x3f # AT_external"
                         "\n.uleb128 0xc # FORM_flag_present"
                         "\n.uleb128 0x3 # AT_name"
                         "\n.uleb128 0xe # FORM_strp"
                         "\n.uleb128 0x3a # AT_decl_file"
                         "\n.uleb128 0xb # FORM_data1"
                         "\n.uleb128 0x3b # AT_decl_line"
                         "\n.uleb128 0xb # FORM_data1"
                         "\n.uleb128 0x39 # AT_decl_column"
                         "\n.uleb128 0xb # FORM_data1"
                         "\n.uleb128 0x11 # AT_low_pc"
                         "\n.uleb128 0x1 # FORM_addr" 
                         "\n.uleb128 0x12 # AT_high_pc"
                         "\n.uleb128 0x7 # FORM_data8"
                         "\n.uleb128 0x40 # AT_frame_base"
                         "\n.uleb128 0x18 # FORM_exprloc"
                         "\n.uleb128 0x7a # AT_call_all_calls"
                         "\n.uleb128 0x19 # FORM_flag_present"
                         "\n";
        break;
      };
      case DIEAbbrev::FunctionWithParams: {
        abbreviations += "\n.uleb128 " + std::to_string((int)a) +
                         "\n.uleb128 0x2e # TAG_subprogram - FunctionNoParams, non-void"
                         "\n.byte 0x1 # Has children"
                         "\n.uleb128 0x3f # AT_external"
                         "\n.uleb128 0xc # FORM_flag_present"
                         "\n.uleb128 0x3 # AT_name"
                         "\n.uleb128 0xe # FORM_strp"
                         "\n.uleb128 0x27 # AT_prototyped"
                         "\n.uleb128 0x19 # FORM_flag_present"
                         "\n.uleb128 0x3a # AT_decl_file"
                         "\n.uleb128 0xb # FORM_data1"
                         "\n.uleb128 0x3b # AT_decl_line"
                         "\n.uleb128 0xb # FORM_data1"
                         "\n.uleb128 0x39 # AT_decl_column"
                         "\n.uleb128 0xb # FORM_data1"
                         "\n.uleb128 0x49 # AT_type" 
                         "\n.uleb128 0x13 # FORM_ref4"
                         "\n.uleb128 0x11 # AT_low_pc"
                         "\n.uleb128 0x1 # FORM_addr" 
                         "\n.uleb128 0x12 # AT_high_pc"
                         "\n.uleb128 0x7 # FORM_data8"
                         "\n.uleb128 0x40 # AT_frame_base"
                         "\n.uleb128 0x18 # FORM_exprloc"
                         "\n.uleb128 0x7a # AT_call_all_calls"
                         "\n.uleb128 0x19 # FORM_flag_present"
                         "\n";
        break;
      }
      case DIEAbbrev::FunctionWithParamsVoid: {
        abbreviations += "\n.uleb128 " + std::to_string((int)a) +
                         "\n.uleb128 0x2e # TAG_subprogram - FunctionNoParams, non-void"
                         "\n.byte 0x1 # Has children"
                         "\n.uleb128 0x3f # AT_external"
                         "\n.uleb128 0xc # FORM_flag_present"
                         "\n.uleb128 0x3 # AT_name"
                         "\n.uleb128 0xe # FORM_strp"
                         "\n.uleb128 0x27 # AT_prototyped"
                         "\n.uleb128 0x19 # FORM_flag_present"
                         "\n.uleb128 0x3a # AT_decl_file"
                         "\n.uleb128 0xb # FORM_data1"
                         "\n.uleb128 0x3b # AT_decl_line"
                         "\n.uleb128 0xb # FORM_data1"
                         "\n.uleb128 0x39 # AT_decl_column"
                         "\n.uleb128 0xb # FORM_data1"
                         "\n.uleb128 0x11 # AT_low_pc"
                         "\n.uleb128 0x1 # FORM_addr" 
                         "\n.uleb128 0x12 # AT_high_pc"
                         "\n.uleb128 0x7 # FORM_data8"
                         "\n.uleb128 0x40 # AT_frame_base"
                         "\n.uleb128 0x18 # FORM_exprloc"
                         "\n.uleb128 0x7a # AT_call_all_calls"
                         "\n.uleb128 0x19 # FORM_flag_present"
                         "\n";
        break;
      }
      case DIEAbbrev::Variable: {
        abbreviations += 	"\n.uleb128 " + std::to_string((int)a) +
                          "\n.uleb128 0x34 # TAG_variable"
                          "\n.byte	0 # No chidlren"
                          "\n.uleb128 0x3 # Name"
                          "\n.uleb128 0xe # FORM_strp"
                          "\n.uleb128 0x3a # AT_decl_file"
                          "\n.uleb128 0xb # FORM_data1"
                          "\n.uleb128 0x3b # AT_decl_line"
                          "\n.uleb128 0xb # FORM_data1"
                          "\n.uleb128 0x39 # AT_decl_column"
                          "\n.uleb128 0xb # FORM_data1"
                          "\n.uleb128 0x49 # AT_type"
                          "\n.uleb128 0x13 # FORM_ref4"
                          "\n.uleb128 0x2 # AT_location"
                          "\n.uleb128 0x18 # FORM_exprloc"
                          "\n";
        break;
      }
      case DIEAbbrev::Type: {
        abbreviations += "\n.uleb128 " + std::to_string((int)a) +
                         "\n.uleb128 0x24 # TAG_base_type"
                         "\n.byte	0 # no children"
                         "\n.uleb128 0xb # AT_byte_size"
                         "\n.uleb128 0xb # FORM_data1"
                         "\n.uleb128 0x3e # AT_encoding"
                         "\n.uleb128 0xb # FORM_data1"
                         "\n.uleb128 0x3 # AT_name"
                         "\n.uleb128 0x8 # FORM_string"
                         "\n";
        break;
      }
      case DIEAbbrev::PointerType: {
        abbreviations += "\n.uleb128 " + std::to_string((int)a) +
                         "\n.uleb128 0xF # TAG_pointer_type"
                         "\n.byte 0 # No children"
                         "\n.uleb128 0xB # AT_byte_size"
                         "\n.uleb128 0xb # FORM_data1"
                         "\n.uleb128 0x3 # AT_name"
                         "\n.uleb128 0xE # FORM_strp"
                         "\n.uleb128 0x49 #  AT_type"
                         "\n.uleb128 0x13 #  FORM_ref4";
        break;
      }
      case DIEAbbrev::CompileUnit: {
        abbreviations += "\n.uleb128 " + std::to_string((int)DIEAbbrev::CompileUnit) + // Index label
                         "\n.uleb128 0x11 # TAG_compile_unit"
                         "\n.byte	0x1 # No children"
                         "\n.uleb128 0x25 # AT_producer"
                         "\n.uleb128 0xe # FORM_strp"
                         "\n.uleb128 0x13 # AT_language"
                         "\n.uleb128 0x05 # FORM_data2" // Language code -- custom ones start at 0x8000, which needs the range of a short or larger
                         "\n.uleb128 0x3 # AT_name"
                         "\n.uleb128 0x1f # FORM_line_strp"
                         "\n.uleb128 0x1b # AT_comp_dir"
                         "\n.uleb128 0x1f # FORM_line_strp"
                         "\n.uleb128 0x11 # AT_low_pc"
                         "\n.uleb128 0x1 # FORM_addr"
                         "\n.uleb128 0x12 # AT_high_pc"
                         "\n.uleb128 0x7 # FORM_data8"
                         "\n.uleb128 0x10 # AT_stmt_list"
                         "\n.uleb128 0x17 # FORM_sec_offset";
        break;
      }
      case DIEAbbrev::FunctionParam: {
        abbreviations += "\n.uleb128 " + std::to_string((int)a) +
                         "\n.byte 0x05 # TAG_formal_parameter"
                         "\n.byte 0 # No children"
                         "\n.byte 0x3a # AT_decl_file"
                         "\n.byte 0x0b # FORM_data1"
                         "\n.byte 0x3b # AT_decl_line"
                         "\n.byte 0x0b # FORM_data1"
                         "\n.byte 0x39 # AT_decl_column"
                         "\n.byte 0x0b # FORM_data1"
                         "\n.byte 0x49 # AT_type"
                         "\n.byte 0x13 # FORM_ref4"
                         "\n.byte 0x2 # AT_location"
                         "\n.byte 0x18 # FORM_exprloc"
                         "\n";

        break;
      };
      case DIEAbbrev::LexicalBlock: {
        abbreviations += "\n.uleb128 " + std::to_string((int)a) +
                         "\n.uleb128 0x0b # TAG_lexical_block"
                          "\n.byte 1 # Yes, absolutely children!"
                          "\n.uleb128 0x3a # AT_decl_file"
                          "\n.uleb128 0xb # FORM_data1"
                          "\n.uleb128 0x3b # AT_decl_line"
                          "\n.uleb128 0xb # FORM_data1"
                          "\n.uleb128 0x39 # AT_decl_column"
                          "\n.uleb128 0xb # FORM_data1"
                          "\n.uleb128 0x11 # AT_low_pc"
                          "\n.uleb128 0x1 # FORM_addr"
                          "\n.uleb128 0x12 # AT_high_pc"
                          "\n.uleb128 0x7 # FORM_data8"
                         "\n";
        break;
      }
      case DIEAbbrev::StructType: {
        // DW_TAG_structure_type
        abbreviations += "\n.uleb128 " + std::to_string((int)a) +
                         "\n.uleb128 0x13 # TAG_structure_type"
                         "\n.byte 1 # children"
                         "\n.uleb128 0x3 # AT_name"
                         "\n.uleb128 0xe # FORM_strp"
                         "\n.uleb128 0x3a # AT_decl_file"
                         "\n.uleb128 0xb # FORM_data1"
                         "\n.uleb128 0x3b # AT_decl_line"
                         "\n.uleb128 0xb # FORM_data1"
                         "\n.uleb128 0x39 # AT_decl_column"
                         "\n.uleb128 0xb # FORM_data1"
                         "\n.uleb128 0xb # AT_byte_size"
                         "\n.uleb128 0x05 # FORM_data2"
                         "\n";
        break;
      }
      case DIEAbbrev::StructMember: {
        abbreviations += "\n.uleb128 " + std::to_string((int)a) +
                         "\n.uleb128 0x0d # TAG_member"
                         "\n.byte 0 # No children"
                         "\n.uleb128 0x3 # AT_name"
                         "\n.uleb128 0xe # FORM_strp"
                         "\n.uleb128 0x49 # AT_type"
                         "\n.uleb128 0x13 # FORM_ref4"
                         "\n.uleb128 0x38 # AT_data_member_location"
                         "\n.uleb128 0x0d # FORM_sdata"
                         "\n";
        break;
      }
      case DIEAbbrev::EnumType: {
        abbreviations += "\n.uleb128 " + std::to_string((int)a) +
                         "\n.uleb128 0x04 # TAG_enumeration_type"
                         "\n.byte 1 # children"
                         "\n.uleb128 0x3 # AT_name"
                         "\n.uleb128 0xe # FORM_strp"
                         "\n.uleb128 0x3a # AT_decl_file"
                         "\n.uleb128 0xb # FORM_data1"
                         "\n.uleb128 0x3b # AT_decl_line"
                         "\n.uleb128 0xb # FORM_data1"
                         "\n.uleb128 0x39 # AT_decl_column"
                         "\n.uleb128 0xb # FORM_data1"
                         // For some reason, C includes BOTH a "type" AND "encoding" attribute.
                         // Since both the 'type', and the 'encoding'-'byte-size' pair are included in C,
                         // i will just make them both the same constant- unsigned long (32-bits, although this is too much.)
                         "\n.uleb128 0xb # AT_byte_size"
                         "\n.uleb128 0xb # FORM_data1"
                         "\n.uleb128 0x3e # AT_encoding"
                         "\n.uleb128 0xb # FORM_data1"
                         "\n.uleb128 0x49 # AT_type"
                         "\n.uleb128 0x13 # FORM_ref4"
                         "\n";
        break;
      }
      case DIEAbbrev::EnumMember: {
        abbreviations += "\n.uleb128 " + std::to_string((int)a) +
                         "\n.uleb128 0x28 # TAG_enumerator"
                         "\n.byte 0 # No children"
                         "\n.uleb128 0x3 # AT_name"
                         "\n.uleb128 0xe # FORM_strp"
                         "\n.uleb128 0x1c # AT_const_value"
                         "\n.uleb128 0xb # FORM_data1"
                         "\n";
        break;
      }
      case DIEAbbrev::ArrayType: {
        abbreviations += "\n.uleb128 " + std::to_string((int)a) +
                         "\n.uleb128 0x01 # TAG_array_type"
                         "\n.byte 1 # children"
                         "\n.uleb128 0x3 # AT_name"
                         "\n.uleb128 0xe # FORM_strp"
                         "\n.uleb128 0x49 # AT_type"
                         "\n.uleb128 0x13 # FORM_ref4"
                         "\n";
        useAbbrev(DIEAbbrev::ArraySubrange); // Add the array subrange - arraytypes are useless without these
        break;
      }
      case DIEAbbrev::ArraySubrange: {
        abbreviations += "\n.uleb128 " + std::to_string((int)a) +
                         "\n.uleb128 0x21 # TAG_subrange_type"
                         "\n.byte 0 # No children"
                         "\n.uleb128 0x49 # AT_type" // typically an int - arr[1] 1 is int
                         "\n.uleb128 0x13 # FORM_ref4"
                         "\n.uleb128 0x2f # AT_upper_bound" // the max size of the array - int arr[10] - 9 is the upper bound (because they start at 0)
                         "\n.uleb128 0x05 # FORM_data2"
                         "\n";
        break;
      }
      default:
        abbreviations += "ERROR HERE! UNIMPLEMENTED ABBREV: " + std::to_string((int)a) + "\n";
        break;
    };
    abbreviations += "\n.byte 0\n.byte 0\n"; // Add padding
  }
  abbreviations += ".byte 0\n.byte 0\n"; // End of all children of CU, end of all tags, yata yata shut up its the end of the abbevations.
  return abbreviations;
};

void codegen::dwarf::useAbbrev(codegen::dwarf::DIEAbbrev a) {
  if (!isUsed(a)) dieAbbrevsUsed.insert(a);
};

bool codegen::dwarf::isUsed(codegen::dwarf::DIEAbbrev a) {
  return dieAbbrevsUsed.find(a) != dieAbbrevsUsed.end();
};

void codegen::dwarf::useType(Node::Type *type) {
  // Equivalent of 'useAbbrev' but on dienames
  std::string dieName = type_to_diename(type);
  if (dieNamesUsed.find(dieName) != dieNamesUsed.end()) return;
  dieNamesUsed.insert(dieName);
  useAbbrev(DIEAbbrev::Type);
  if (type->kind == NodeKind::ND_POINTER_TYPE) {
    // Create a PointerType
    useAbbrev(DIEAbbrev::PointerType);
    PointerType *p = static_cast<PointerType *>(type);
    useType(p->underlying);
    std::string underlyingName = type_to_diename(p->underlying);
    // strp
    push(Instr{.var = Label{.name = ".L" + dieName + "_string"}, .type = InstrType::Label}, Section::DIEString);
    pushLinker(".string \"" + TypeChecker::type_to_string(type) + "\"\n", Section::DIEString);
    push(Instr{.var = Label{.name = ".L" + dieName + "_debug_type"}, .type = InstrType::Label}, Section::DIETypes);
    pushLinker(".uleb128 " + std::to_string((int)DIEAbbrev::PointerType) +
               "\n.byte 8 # AT_byte_size (all pointers are 8 bytes)"
               "\n.long .L" + dieName + "_string # FORM_strp"
               "\n.long .L" + dieName + "_debug_type # FORM_ref4"
               , Section::DIETypes);
  } else if (type->kind == NodeKind::ND_ARRAY_TYPE) {
    // Create an ArrayType
    useAbbrev(DIEAbbrev::ArrayType);
    useAbbrev(DIEAbbrev::ArraySubrange);
    useType(static_cast<ArrayType *>(type)->underlying);
    useType(new SymbolType("int", SymbolType::Signedness::UNSIGNED));
    ArrayType *a = static_cast<ArrayType *>(type);
    push(Instr{.var = Label{.name = ".L" + dieName + "_debug_type"}, .type = InstrType::Label}, Section::DIETypes);
    // string section
    push(Instr{.var = Label{.name = ".L" + dieName + "_string"}, .type = InstrType::Label}, Section::DIEString);
    pushLinker(".string \"" + TypeChecker::type_to_string(type) + "\"\n", Section::DIEString);
    pushLinker(".uleb128 " + std::to_string((int)DIEAbbrev::ArrayType) +
               "\n.long .L" + dieName + "_string # FORM_strp"
               "\n.long .L" + type_to_diename(a->underlying) + "_debug_type\n"
               , Section::DIETypes);
    pushLinker(".uleb128 " + std::to_string((int)DIEAbbrev::ArraySubrange) +
               "\n.long .Lint_u_debug_type" // This is gonna be the type of the index used in arrays - arr[1] - 1 is an int 
               "\n.short " + (a->constSize <= 0 ? std::to_string(std::numeric_limits<short signed int>::max()) : std::to_string(a->constSize - 1)) +
               "\n.byte 0\n" // end of the arrayType
               , Section::DIETypes);
  } else {
    // Nothing can be done if it is a basic type.
    // If it is a struct type, then that will already be handled by the structDecl function
    // This area here is useless!
    if (type->kind == ND_SYMBOL_TYPE) {
      if (static_cast<SymbolType *>(type)->name == "str") {
        useType(new SymbolType("char", SymbolType::Signedness::UNSIGNED));
      }
    }
  }
};

void codegen::dwarf::useStringP(std::string what) {
  if (dieStringsUsed.find(what) != dieStringsUsed.end()) return;
  dieStringsUsed.insert(what);

  // Create the thing :)
  push(Instr{.var = Label{.name = ".L" + what + "_string"}, .type = InstrType::Label}, Section::DIEString);
  pushLinker(".string \"" + what + "\"\n", Section::DIEString);
}

void codegen::dwarf::emitTypes(void) {
  // for each used builtin type, append the DIE to the section
  if (dieNamesUsed.size() == 0) return;
  // int types
  // int (8)
  if (dieNamesUsed.find("int_u") != dieNamesUsed.end()) { // unsigned 8 byte integer type
    push(Instr{.var = Label{.name = ".Lint_u_debug_type"}, .type = InstrType::Label}, Section::DIETypes);
    pushLinker(".uleb128 " + std::to_string((int)DIEAbbrev::Type) +
               "\n.byte 8 # AT_byte_size"
                "\n.uleb128 0x7 # AT_encoding = ATE_unsigned"
                "\n.string \"int!\"\n"
    , Section::DIETypes);
  }
  if (dieNamesUsed.find("int_s") != dieNamesUsed.end() || dieNamesUsed.find("int_i") != dieNamesUsed.end()) { // signed 8 byte integer type
    push(Instr{.var = Label{.name = ".Lint_s_debug_type"}, .type = InstrType::Label}, Section::DIETypes);
    push(Instr{.var = Label{.name = ".Lint_i_debug_type"}, .type = InstrType::Label}, Section::DIETypes);
    pushLinker(".uleb128 " + std::to_string((int)DIEAbbrev::Type) +
               "\n.byte 8 # AT_byte_size"
                "\n.uleb128 0x5 # AT_encoding = ATE_signed"
                "\n.string \"int?\"\n"
    , Section::DIETypes);
  }
  // char
  if (dieNamesUsed.find("char_u") != dieNamesUsed.end()) {
    push(Instr{.var = Label{.name = ".Lchar_u_debug_type"}, .type = InstrType::Label}, Section::DIETypes);
    pushLinker(".uleb128 " + std::to_string((int)DIEAbbrev::Type) +
               "\n.byte 1 # AT_byte_size"
               "\n.uleb128 0x8 # AT_encoding = ATE_unsigned_char"
               "\n.string \"char!\"\n"
    , Section::DIETypes);
  }
  if (dieNamesUsed.find("char_s") != dieNamesUsed.end() || dieNamesUsed.find("char_i") != dieNamesUsed.end()) {
    push(Instr{.var = Label{.name = ".Lchar_s_debug_type"}, .type = InstrType::Label}, Section::DIETypes);
    push(Instr{.var = Label{.name = ".Lchar_i_debug_type"}, .type = InstrType::Label}, Section::DIETypes);
    pushLinker(".uleb128 " + std::to_string((int)DIEAbbrev::Type) +
               "\n.byte 1 # AT_byte_size"
               "\n.uleb128 0x6 # AT_encoding = ATE_signed_char"
               "\n.string \"char?\"\n"
    , Section::DIETypes);
  }
  // short
  if (dieNamesUsed.find("short_u") != dieNamesUsed.end()) {
    push(Instr{.var = Label{.name = ".Lshort_u_debug_type"}, .type = InstrType::Label}, Section::DIETypes);
    pushLinker(".uleb128 " + std::to_string((int)DIEAbbrev::Type) +
               "\n.byte 2 # AT_byte_size"
               "\n.uleb128 0x7 # AT_encoding = ATE_unsigned"
               "\n.string \"short!\"\n"
    , Section::DIETypes);
  }
  if (dieNamesUsed.find("short_s") != dieNamesUsed.end() || dieNamesUsed.find("short_i") != dieNamesUsed.end()) {
    push(Instr{.var = Label{.name = ".Lshort_s_debug_type"}, .type = InstrType::Label}, Section::DIETypes);
    push(Instr{.var = Label{.name = ".Lshort_i_debug_type"}, .type = InstrType::Label}, Section::DIETypes);
    pushLinker(".uleb128 " + std::to_string((int)DIEAbbrev::Type) +
               "\n.byte 2 # AT_byte_size"
               "\n.uleb128 0x5 # AT_encoding = ATE_signed"
               "\n.string \"short?\"\n"
    , Section::DIETypes);
  }
  // long
  if (dieNamesUsed.find("long_u") != dieNamesUsed.end()) {
    push(Instr{.var = Label{.name = ".Llong_u_debug_type"}, .type = InstrType::Label}, Section::DIETypes);
    pushLinker(".uleb128 " + std::to_string((int)DIEAbbrev::Type) +
               "\n.byte 4 # AT_byte_size"
               "\n.uleb128 0x7 # AT_encoding = ATE_unsigned"
               "\n.string \"long!\"\n"
    , Section::DIETypes);
  }
  if (dieNamesUsed.find("long_s") != dieNamesUsed.end() || dieNamesUsed.find("long_i") != dieNamesUsed.end()) {
    push(Instr{.var = Label{.name = ".Llong_s_debug_type"}, .type = InstrType::Label}, Section::DIETypes);
    push(Instr{.var = Label{.name = ".Llong_i_debug_type"}, .type = InstrType::Label}, Section::DIETypes);
    pushLinker(".uleb128 " + std::to_string((int)DIEAbbrev::Type) +
               "\n.byte 4 # AT_byte_size"
               "\n.uleb128 0x5 # AT_encoding = ATE_signed"
               "\n.string \"long?\"\n"
    , Section::DIETypes);
  }
  // non-int types
  // str
  if (dieNamesUsed.find("str_i") != dieNamesUsed.end()) {
    // strp
    push(Instr{.var = Label{.name = ".L_str_string"}, .type = InstrType::Label}, Section::DIEString);
    pushLinker(".string \"str\"\n", Section::DIEString);
    
    // pointer type
    useAbbrev(DIEAbbrev::PointerType);
    push(Instr{.var = Label{.name = ".Lstr_i_debug_type"}, .type = InstrType::Label}, Section::DIETypes);
    pushLinker(".uleb128 " + std::to_string((int)DIEAbbrev::PointerType) +
               "\n.byte 8 # AT_byte_size"
               "\n.long .L_str_string # FORM_strp"
               "\n.long .Lchar_u_debug_type # FORM_ref4"
               , Section::DIETypes);
  }
  // bool (no signedness)
  if (dieNamesUsed.find("bool_i") != dieNamesUsed.end()) {
    push(Instr{.var = Label{.name = ".Lbool_i_debug_type"}, .type = InstrType::Label}, Section::DIETypes);
    pushLinker(".uleb128 " + std::to_string((int)DIEAbbrev::Type) +
               "\n.byte 1 # AT_byte_size"
               "\n.uleb128 0x2 # AT_encoding = ATE_boolean"
               "\n.string \"bool\"\n"
    , Section::DIETypes);
  }
  // float
  if (dieNamesUsed.find("float_i") != dieNamesUsed.end()) {
    push(Instr{.var = Label{.name = ".Lfloat_i_debug_type"}, .type = InstrType::Label}, Section::DIETypes);
    pushLinker(".uleb128 " + std::to_string((int)DIEAbbrev::Type) +
               "\n.byte 4 # AT_byte_size"
               "\n.uleb128 0x4 # AT_encoding = ATE_float"
               "\n.string \"float\"\n"
    , Section::DIETypes);
  }
  // double (float with 8 bytes)
  if (dieNamesUsed.find("double_i") != dieNamesUsed.end()) {
    push(Instr{.var = Label{.name = ".Ldouble_i_debug_type"}, .type = InstrType::Label}, Section::DIETypes);
    pushLinker(".uleb128 " + std::to_string((int)DIEAbbrev::Type) +
               "\n.byte 8 # AT_byte_size"
               "\n.uleb128 0x4 # AT_encoding = ATE_float"
               "\n.string \"double\"\n"
    , Section::DIETypes);
  }

  // TODO: Make custom structs for the @ functions (like sockaddr)
}