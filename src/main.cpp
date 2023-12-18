#include <cstring>
#include <iostream>

#include "../inc/colorize.hpp"
#include "common.hpp"
#include "helper/flags.hpp"

using namespace std;

int main(int argc, char **argv) {
  if ((argc == 2 && strcmp(argv[1], "--help") == 0) || argc == 1) {
    cout << "Usage: " << argv[0] << " [options]" << endl;
    cout << "Options:" << endl;
    cout << "  --help\t\t\tPrints this help message" << endl;
    cout << "  --version\t\t\tPrints the version of the compiler" << endl;
    cout << "  --license\t\t\tPrints the license of the Zura Lang" << endl;
    cout << "  --update\t\t\tUpdates the Zura compiler" << endl;
    cout << "Compiler:" << endl;
    cout << termcolor::red << "   -s" << termcolor::reset
         << ", \t\tsave the generated c file and the out.o file" << endl;
    cout << termcolor::red << "   -o" << termcolor::reset
         << ", \t\tOutput the transpiled file to <file>" << endl;
    cout << termcolor::red << "   -r" << termcolor::reset
         << ", \t\tRun the exacutable file. (Not yet implemented)" << endl;
    cout << termcolor::red << "   -c" << termcolor::reset
         << ", \t\tDelete the exacutable file and c file if it is there."
         << endl;
    Exit(ExitValue::FLAGS_PRINTED);
  }

  // version
  if (argc == 2 && strcmp(argv[1], "--version") == 0) {
    cout << "Zura Lang " << ZuraVersion << endl;
    Exit(ExitValue::FLAGS_PRINTED);
  }

  // update
  if (argc == 2 && strcmp(argv[1], "--update") == 0) {
    system("python scripts/update.py");
    Exit(ExitValue::UPDATED);
  }

  // license
  if (argc == 2 && strcmp(argv[1], "--license") == 0) {
    cout << "Zura uses a license under GPL-3.0" << endl;
    cout << "You can find the license here:" << endl;
    cout << termcolor::blue << "https://www.gnu.org/licenses/gpl-3.0.en.html"
         << termcolor::reset << endl;
    Exit(ExitValue::FLAGS_PRINTED);
  }

  // (delete)
  if (argc == 3 && strcmp(argv[1], "-c") == 0)
    Flags::compilerDelete(argv);
  // (transpile)
  if (argc == 4 && strcmp(argv[2], "-o") == 0) {
    char *outName = argv[3];
    Flags::runFile(argv[1], outName, false);
  }
  // (transpile and save)
  if (argc == 5 && strcmp(argv[2], "-o") == 0 && strcmp(argv[4], "-s") == 0) {
    char *outName = argv[3];
    Flags::runFile(argv[1], outName, true);
  }

  return 0;
}
