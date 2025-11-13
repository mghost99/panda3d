// Filename: cLerpSeaPatchInterval.cxx
////////////////////////////////////////////////////////////////////

#include "cLerpSeaPatchInterval.h"

TypeHandle CLerpSeaPatchInterval::_type_handle;

CLerpSeaPatchInterval::
CLerpSeaPatchInterval(const std::string &name, double duration,
                      CLerpInterval::BlendType blend_type,
                      SeaPatchRoot *patch,
                      SeaPatchRoot *start_patch,
                      SeaPatchRoot *end_patch) :
  CLerpInterval(name, duration, blend_type),
  _patch(patch),
  _start_patch(start_patch),
  _end_patch(end_patch)
{
}

void CLerpSeaPatchInterval::
priv_step(double t) {
  check_started(get_class_type(), "priv_step");
  _state = S_started;
  double d = compute_delta(t);
  
  if (_start_patch.is_null() || _end_patch.is_null() || _patch.is_null()) {
    return;
  }

  // Lerp the sea level
  PN_stdfloat start_level = _start_patch->get_sea_level();
  PN_stdfloat end_level = _end_patch->get_sea_level();
  _patch->set_sea_level(start_level + d * (end_level - start_level));

  // Lerp the overall speed
  PN_stdfloat start_speed = _start_patch->get_overall_speed();
  PN_stdfloat end_speed = _end_patch->get_overall_speed();
  _patch->set_overall_speed(start_speed + d * (end_speed - start_speed));

  // Lerp the radius
  PN_stdfloat start_radius = _start_patch->get_radius();
  PN_stdfloat end_radius = _end_patch->get_radius();
  _patch->set_radius(start_radius + d * (end_radius - start_radius));

  // Lerp the threshold
  PN_stdfloat start_threshold = _start_patch->get_threshold();
  PN_stdfloat end_threshold = _end_patch->get_threshold();
  _patch->set_threshold(start_threshold + d * (end_threshold - start_threshold));

  // Lerp colors
  LColorf start_high = _start_patch->get_high_color();
  LColorf end_high = _end_patch->get_high_color();
  LColorf high_color = start_high + d * (end_high - start_high);
  _patch->set_high_color(high_color);

  LColorf start_low = _start_patch->get_low_color();
  LColorf end_low = _end_patch->get_low_color();
  LColorf low_color = start_low + d * (end_low - start_low);
  _patch->set_low_color(low_color);

  // Lerp UV speed
  LVecBase2f start_uv_speed = _start_patch->get_uv_speed();
  LVecBase2f end_uv_speed = _end_patch->get_uv_speed();
  LVecBase2f uv_speed = start_uv_speed + d * (end_uv_speed - start_uv_speed);
  _patch->set_uv_speed(uv_speed);

  // Lerp UV scale
  LVecBase2f start_uv_scale = _start_patch->get_uv_scale();
  LVecBase2f end_uv_scale = _end_patch->get_uv_scale();
  LVecBase2f uv_scale = start_uv_scale + d * (end_uv_scale - start_uv_scale);
  _patch->set_uv_scale(uv_scale);
}

