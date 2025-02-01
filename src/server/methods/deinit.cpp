#include "../../common.hpp"
#include "../logging.hpp"
#include "../json.hpp"
#include "../lsp.hpp"

nlohmann::ordered_json lsp::methods::shutdown(nlohmann::json& request) {
  // Deinitialize, free memory, etc...
  // logging::log("Shutting down...\n");
  // logging::close();
  return nullptr; // Empty brackets is converted to "null" upon being stringified
}

nlohmann::ordered_json lsp::methods::exit(nlohmann::json& request) {
  Exit(ExitValue::SUCCESS);
};

nlohmann::ordered_json lsp::methods::cancelRequest(nlohmann::json& request) {
  // We don't care about what is getting cancelled, but we must respond to these events anyway
  return nullptr;
}