// Filename: cLerpSeaPatchInterval.h
////////////////////////////////////////////////////////////////////

#ifndef CLERPSEAPATCHINTERVAL_H
#define CLERPSEAPATCHINTERVAL_H

#include "piratesbase.h"
#include "seaPatchRoot.h"
#include "cLerpInterval.h"

class EXPCL_PIRATES CLerpSeaPatchInterval : public CLerpInterval {
PUBLISHED:
  CLerpSeaPatchInterval(const std::string &name, double duration,
                        CLerpInterval::BlendType blend_type,
                        SeaPatchRoot *patch,
                        SeaPatchRoot *start_patch,
                        SeaPatchRoot *end_patch);

  virtual void priv_step(double t);

private:
  PT(SeaPatchRoot) _patch;
  PT(SeaPatchRoot) _start_patch;
  PT(SeaPatchRoot) _end_patch;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CLerpInterval::init_type();
    register_type(_type_handle, "CLerpSeaPatchInterval",
                  CLerpInterval::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "cLerpSeaPatchInterval.I"

#endif

