#include "gen.hpp"

std::string codegen::dwarf::generateAbbreviations() {
  std::string abbreviations = "";
  for (DIEAbbrev a : dieAbbrevsUsed) {
    switch (a) {
      case DIEAbbrev::FunctionNoParams: {
        abbreviations += "\n.uleb128 " + std::to_string((int)a) +
                         "\n.uleb128 0x2e # TAG_subprogram - FunctionNoParams, non-void"
                         "\n.byte 0x1 # Has children"
                         "\n.uleb128 0x3f # AT_external"
                         "\n.uleb128 0x19 # FORM_flag_present"
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
                         "\n.uleb128 0x19 # FORM_flag_present"
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
                         "\n.uleb128 0x19 # FORM_flag_present"
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
                         "\n.uleb128 0x19 # FORM_flag_present"
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
                         "\n.uleb128 0xb # FORM_data1"
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
                         "\n.uleb128 0xb # FORM_data1"
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
                         "\n.uleb128 0xb # FORM_data1"
                         "\n";
        break;
      }
      default:
        abbreviations += "ERROR HERE! UNIMPLEMENTED";
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
    push(Instr{.var = Label{.name = ".L" + dieName + "_debug_type"}, .type = InstrType::Label}, Section::DIE);
    pushLinker(".uleb128 " + std::to_string((int)DIEAbbrev::PointerType) +
               "\n.byte 8 # AT_byte_size (all pointers are 8 bytes)"
               "\n.long .L" + underlyingName + "_debug_type # FORM_ref4"
               , Section::DIE);
  } else if (type->kind == NodeKind::ND_ARRAY_TYPE) {
    // Create an ArrayType
    // useAbbrev(DIEAbbrev::ArrayType);

  } else {
    // Nothing can be done if it is a basic type.
    // If it is a struct type, then that will already be handled by the structDecl function
    // This area here is useless!
  }
};