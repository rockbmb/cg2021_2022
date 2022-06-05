#include "util.h"
#include <string>
#include <filesystem>
#include <iostream>

using std::string, std::filesystem::exists, std::cerr, std::endl;

void crash_if_file_does_not_exist (const string &filename, const string &additional_info = "")
{
  if (!exists(filename))
    {
      cerr << "Failed loading " << filename << " " << additional_info << endl;
      exit(EXIT_FAILURE);
    }
}