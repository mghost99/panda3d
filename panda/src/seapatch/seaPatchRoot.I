INLINE void SeaPatchRoot::enable(){
  _enabled = true;
}

INLINE void SeaPatchRoot::disable(){
  _enabled = false;
}

INLINE bool SeaPatchRoot::is_enabled() const {
  return _enabled;
}

INLINE void SeaPatchRoot::set_center(NodePath center) {
	// Just use the position of the node directly, don't try to get bounds
	// (cameras and other nodes may have infinite bounds)
	_center = center.get_pos();
}

INLINE LPoint3f SeaPatchRoot::get_center() const {
  return _center;
}

INLINE void SeaPatchRoot::set_anchor(NodePath anchor) {
	_anchor = anchor;
}

INLINE NodePath SeaPatchRoot::get_anchor() const {
	return _anchor;
}

INLINE void SeaPatchRoot::set_sea_level(float level) {
	_seaLevel = level;
}

INLINE float SeaPatchRoot::get_sea_level() const {
	return _seaLevel;
}

INLINE void SeaPatchRoot::set_high_color(LColorf color) {
	_colorHigh = color;
}

INLINE LColorf SeaPatchRoot::get_high_color() const {
	return _colorHigh;
}

INLINE void SeaPatchRoot::set_low_color(LColorf color) {
	_colorLow = color;
}

INLINE LColorf SeaPatchRoot::get_low_color() const {
	return _colorLow;
}

INLINE void SeaPatchRoot::set_overall_speed(float speed) {
	_overallSpeed = speed;
}

INLINE float SeaPatchRoot::get_overall_speed() const {
	return _overallSpeed;
}

INLINE void SeaPatchRoot::set_uv_speed(LVecBase2f speed) {
	_uvSpeed = speed;
}

INLINE LVecBase2f SeaPatchRoot::get_uv_speed() const {
	return _uvSpeed;
}

INLINE void SeaPatchRoot::set_uv_scale(LVecBase2f scale) {
	_uvScale = scale;
}

INLINE LVecBase2f SeaPatchRoot::get_uv_scale() const {
	return _uvScale;
}

INLINE void SeaPatchRoot::set_threshold(float threshold) {
	_threshold = threshold;
}

INLINE float SeaPatchRoot::get_threshold() const {
	return _threshold;
}

INLINE void SeaPatchRoot::set_radius(float radius) {
	_radius = radius;
}

INLINE float SeaPatchRoot::get_radius() const {
	return _radius;
}

// Wave management
INLINE void SeaPatchRoot::enable_wave(int waveId) {
	if (waveId >= 0 && waveId < MAX_WAVES) {
		_waves[waveId].enabled = true;
	}
}

INLINE void SeaPatchRoot::disable_wave(int waveId) {
	if (waveId >= 0 && waveId < MAX_WAVES) {
		_waves[waveId].enabled = false;
	}
}

INLINE bool SeaPatchRoot::is_wave_enabled(int waveId) const {
	if (waveId >= 0 && waveId < MAX_WAVES) {
		return _waves[waveId].enabled;
	}
	return false;
}

INLINE int SeaPatchRoot::get_num_waves() const {
	return _num_waves;
}

// Wave properties
INLINE void SeaPatchRoot::set_wave_amplitude(int waveId, float amplitude) {
	if (waveId >= 0 && waveId < MAX_WAVES) {
		_waves[waveId].amplitude = amplitude;
	}
}

INLINE float SeaPatchRoot::get_wave_amplitude(int waveId) const {
	if (waveId >= 0 && waveId < MAX_WAVES) {
		return _waves[waveId].amplitude;
	}
	return 0.0f;
}

INLINE void SeaPatchRoot::set_wave_length(int waveId, float length) {
	if (waveId >= 0 && waveId < MAX_WAVES) {
		_waves[waveId].length = length;
	}
}

INLINE float SeaPatchRoot::get_wave_length(int waveId) const {
	if (waveId >= 0 && waveId < MAX_WAVES) {
		return _waves[waveId].length;
	}
	return 1.0f;
}

INLINE void SeaPatchRoot::set_wave_speed(int waveId, float speed) {
	if (waveId >= 0 && waveId < MAX_WAVES) {
		_waves[waveId].speed = speed;
	}
}

INLINE float SeaPatchRoot::get_wave_speed(int waveId) const {
	if (waveId >= 0 && waveId < MAX_WAVES) {
		return _waves[waveId].speed;
	}
	return 1.0f;
}

INLINE void SeaPatchRoot::set_wave_direction(int waveId, LVecBase2f dir) {
	if (waveId >= 0 && waveId < MAX_WAVES) {
		_waves[waveId].direction = dir;
	}
}

INLINE LVecBase2f SeaPatchRoot::get_wave_direction(int waveId) const {
	if (waveId >= 0 && waveId < MAX_WAVES) {
		return _waves[waveId].direction;
	}
	return LVecBase2f(1, 0);
}

INLINE void SeaPatchRoot::set_choppy_k(int waveId, int choppy) {
	if (waveId >= 0 && waveId < MAX_WAVES) {
		_waves[waveId].choppy_k = choppy;
	}
}

INLINE int SeaPatchRoot::get_choppy_k(int waveId) const {
	if (waveId >= 0 && waveId < MAX_WAVES) {
		return _waves[waveId].choppy_k;
	}
	return 0;
}

INLINE void SeaPatchRoot::set_wave_target(int waveId, int target) {
	if (waveId >= 0 && waveId < MAX_WAVES) {
		_waves[waveId].target = target;
	}
}

INLINE int SeaPatchRoot::get_wave_target(int waveId) const {
	if (waveId >= 0 && waveId < MAX_WAVES) {
		return _waves[waveId].target;
	}
	return WTZ;
}

INLINE void SeaPatchRoot::set_wave_func(int waveId, int func) {
	if (waveId >= 0 && waveId < MAX_WAVES) {
		_waves[waveId].func = func;
	}
}

INLINE int SeaPatchRoot::get_wave_func(int waveId) const {
	if (waveId >= 0 && waveId < MAX_WAVES) {
		return _waves[waveId].func;
	}
	return WFSin;
}

// Animation
INLINE void SeaPatchRoot::animate_height(bool flag) {
	_animate_height = flag;
}

INLINE void SeaPatchRoot::animate_uv(bool flag) {
	_animate_uv = flag;
}