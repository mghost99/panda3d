// Filename: config_seapatch.cxx
////////////////////////////////////////////////////////////////////

#include "config_seapatch.h"
#include "seaPatchRoot.h"
#include "seaPatchNode.h"

#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PIRATES)
  #error Buildsystem error: BUILDING_PIRATES not defined
#endif

Configure(config_seapatch);
NotifyCategoryDef(seapatch, "");

ConfigureFn(config_seapatch) {
  init_libseapatch();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libseapatch
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libseapatch() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  SeaPatchRoot::init_type();
  SeaPatchNode::init_type();
}

