// Filename: piratessymbols.h
/////////////////////////////////////////////

#ifndef PIRATESSYMBOLS_H
#define PIRATESSYMBOLS_H

/* See dtoolsymbols.h for a rant on the purpose of this file.  */

#ifdef BUILDING_PIRATES
  #define EXPCL_PIRATES EXPORT_CLASS
  #define EXPTP_PIRATES EXPORT_TEMPL
#else
  #define EXPCL_PIRATES IMPORT_CLASS
  #define EXPTP_PIRATES IMPORT_TEMPL
#endif

#endif
