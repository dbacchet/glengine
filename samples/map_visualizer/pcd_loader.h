#include "gl_types.h"

#include <vector>

/// load points for the given file, and add them to the vector
bool load_pcd(const char *filename, std::vector<glengine::Vertex> &points);

