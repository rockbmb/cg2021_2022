#ifndef PROJ_PARSING_H
#define PROJ_PARSING_H

#include <vector>

void operations_load_xml(const char *filename, std::vector<float> *operations);

enum {
  TRANSLATE = 1,
  ROTATE,
  SCALE,
  LOAD_MODEL,
  BEGIN_GROUP,
  END_GROUP,
  EXTENDED_TRANSLATE,
  EXTENDED_ROTATE
};
#endif //PROJ_PARSING_H
