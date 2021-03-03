<% from idl_gen import MetaType, PRIMITIVE_TYPES %>\
/* --- Generated code --- */

#pragma once

## % for imp in imports:
## #include "${imp}"
## % endfor
% for imp in msg.includes:
#include "${imp}"
% endfor

#include <cassert>  // Some types need it, some types don't
#include <cstring>  // Some types need it, some types don't
#include <string>   // Some types need it, some types don't
#include <vector>   // Some types need it, some types don't

## Namespaces
% for ns in namespace.split('::'):
namespace ${ns} {
% endfor

## Class definition
% if msg.metatype == MetaType.ENUM:
enum class ${msg.name} {
    % for member in msg.members:
    ${member.name} = ${member.value},
    % endfor
};

% else:
/*
 * ${msg.name} (${msg.fullname})
 *
 * Serialization mode of this type: ${msg.source_type.serialization_mode.name}
 */
class ${msg.name} {
  public:
    ##
    ## Define members
    ##
    // Member definitions
    % for member in msg.members:
    <%
    t = f'std::vector<{member.type.fullname}>' if member.is_vec else member.type.fullname
    n = member.name
    v = member.value if member.value is not None else ''
    %>
    ${t} ${n}{${v}};
    % endfor

};  // class ${msg.name.split('::')[-1]}

## The component root is special. In order to prevent making copies of all the components
## when we serialize, we create a data structure that holds references to the components
## instead. This way, we make fewer allocations, but also, we can reuse all the
## serialization logic even though the C++ type we are serializing internally holds
## non-copyable types (e.g. std::unique_ptr<T>).
% if msg.metatype == MetaType.COMPONENT_ROOT:
/*
 * ${msg.name}_Ref (${msg.fullname}) (holds references only)
 *
 * Serialization mode of this type: ${msg.source_type.serialization_mode.name}
 *
 * Note: this is a special type that only holds reference for more efficient serialization
 *       and some other optimizations (for more information, see the template that was used
 *       to generate this code).
 */
class ${msg.name}_Ref {
  public:
    ##
    ## Define members
    ##
    // Member definitions
    % for member in msg.members:
    <%
    t = f'std::vector<std::reference_wrapper<{member.type.fullname}>>' if member.is_vec else \
        f'std::reference_wrapper<{member.type.fullname}'
    n = member.name
    v = member.value if member.value is not None else ''
    %>
    ${t} ${n}{${v}};
    % endfor

};  // class ${msg.name.split('::')[-1]}_Ref
% endif  ## msg.metatype == MetaType.COMPONENT_ROOT
% endif

## Namespaces
% for ns in reversed(namespace.split('::')):
}  // namespace ${ns}
% endfor

## enum magic
% if msg.metatype == MetaType.ENUM:

namespace sim {
// Define a lookup for the enum's name
inline const char* enum_name(${msg.fullname} x) {
    % for member in msg.members:
    if (x == ${msg.fullname}::${member.name}) { return "${member.name}"; }
    % endfor
    return "";
}

// Define a lookup for the enum's value
inline auto enum_value(${msg.fullname} x) {
    return static_cast<std::underlying_type_t<${msg.fullname}>>(x);
}

// String to enum conversion
// Note: SFINAE on return type because multiple overloads for this function will exist
// that can only be disambiguated by the return type. (Also see overload below)
template <typename T>
std::enable_if_t<std::is_same_v<T, ${msg.fullname}>, ${msg.fullname}> enum_from_string(const char* s) {
    % for member in msg.members:
    if (strcmp(s, "${member.name}") == 0) { return ${msg.fullname}::${member.name}; }
    % endfor

    // If value does not exist, it's a runtime assertion
    std::cerr << "Invalid enum value '" << s << "' for type ${msg.fullname}."
              << "Valid values: \n";
    % for member in msg.members:
    std::cerr << "${member.name} ";
    % endfor
    std::cerr << std::endl;
    assert(false && "Invalid enum value for type");
};

template <typename T>
std::enable_if_t<std::is_same_v<T, ${msg.fullname}>, ${msg.fullname}> enum_from_string(const std::string &s) { return enum_from_string<${msg.fullname}>(s.c_str()); }

} // namespace sim
% endif