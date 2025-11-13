// Filename: config_seapatch.h
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_SEAPATCH_H
#define CONFIG_SEAPATCH_H

#include "piratesbase.h"
#include "notifyCategoryProxy.h"

NotifyCategoryDecl(seapatch, EXPCL_PIRATES, EXPTP_PIRATES);

extern EXPCL_PIRATES void init_libseapatch();

#endif
