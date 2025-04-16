/// @file
/// @brief Definitions to enable compatibility across Tcl 8 → Tcl 9
///
/// Tcl 9 contained multiple breaking changes to the API. To allow compilation
/// under either Tcl 8 or Tcl 9, various shims are needed.
///   • https://core.tcl-lang.org/tcl/wiki?name=Migrating+C+extensions+to+Tcl+9
///   • https://wiki.tcl-lang.org/page/Porting+extensions+to+Tcl+9

#pragma once

#include <limits.h>
#include <tcl.h>

// suggested Tcl 8 compatibility shims from
// https://core.tcl-lang.org/tcl/wiki?name=Migrating+C+extensions+to+Tcl+9
#ifndef TCL_SIZE_MAX
#define Tcl_GetSizeIntFromObj Tcl_GetIntFromObj
#define TCL_SIZE_MAX INT_MAX
#define TCL_SIZE_MODIFIER ""

/// size type for Tcl 8
///
/// https://core.tcl-lang.org/tcl/wiki?name=Migrating+C+extensions+to+Tcl+9
/// says:
///
///   "Tcl_Size" itself doesn't need to be provided as long as you update the
///   extension to the latest tclconfig release. TEA (latest tcl.m4/rules.vc
///   files) takes care of providing an appropriate preprocessor `#define` for
///   `Tcl_Size`.
///
/// but we assume this has not happened.
#ifndef Tcl_Size
#define Tcl_Size int
#endif

#endif
