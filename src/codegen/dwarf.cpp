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
        abbreviations += "\n.uleb128 " + std::to_string((int)a) +
                         "\n.uleb128 0x34 # TAG_variable"
                         "\n.byte 0 # No children"
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
                         "\n.byte 0x18 # FORM_exprloc";
                         "\n";

        break;
      };
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
  return dieAbbrevsUsed.contains(a);
};