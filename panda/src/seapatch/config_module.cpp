// Filename: config_module.cpp
////////////////////////////////////////////////////////////////////

#include "config_module.h"
#include "seaPatchRoot.h"
#include "seaPatchNode.h"

#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PIRATES)
  #error Buildsystem error: BUILDING_PIRATES not defined
#endif

Configure(config_libpirates);
NotifyCategoryDef(seapatch, "");

ConfigureFn(config_libpirates) {
  init_libpirates();
}

void init_libpirates() {
  static bool initialized = false;

  if (initialized) {
    return;
  }

  initialized = true;

  // Init SeaPatch types.
  SeaPatchRoot::init_type();
  SeaPatchNode::init_type();

  return;
}

