#pragma once

#include <cstdint>
#include <cmath>
#include <string>
#include <vector>

## Namespaces
% for ns in namespace:
namespace ${ns} {
% endfor

## enums
% for e in enums:
enum ${e.name} {
% for v in e.values:
  % if v.init == "":
    ${v.name},
  % else:
    ${v.name} = ${v.init},
  % endif
% endfor
};

% endfor

## structs
% for s in structs:
struct ${s.name} {
% for m in s.members:
  % if m.init == "":
    ${m.type} ${m.name};
  % else:
    ${m.type} ${m.name} = ${m.init};
  % endif
% endfor
};

% endfor

## Namespaces
% for ns in reversed(namespace):
} // namespace ${ns}
% endfor
