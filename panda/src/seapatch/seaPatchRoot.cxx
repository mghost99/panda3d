#include "seaPatchRoot.h"

// Type Handle.
TypeHandle SeaPatchRoot::_type_handle;

// Setup our notify category.
NotifyCategoryDef(SeaPatchRoot, "");

SeaPatchRoot::SeaPatchRoot() {
	_enabled = false;
	_seaLevel = 0.0f;
	_colorHigh = LColorf(1, 1, 1, 1);
	_colorLow = LColorf(0, 0, 0, 1);
	_overallSpeed = 1.0f;
	_uvSpeed = LVecBase2f(0, 0);
	_uvScale = LVecBase2f(1, 1);
	_threshold = 0.0f;
	_radius = 1000.0f;
	_animate_height = true;
	_animate_uv = true;
	_num_waves = MAX_WAVES;
	
	// Initialize waves
	for (int i = 0; i < MAX_WAVES; i++) {
		_waves[i].enabled = false;
		_waves[i].amplitude = 0.0f;
		_waves[i].length = 1.0f;
		_waves[i].speed = 1.0f;
		_waves[i].direction = LVecBase2f(1, 0);
		_waves[i].choppy_k = 0;
		_waves[i].target = WTZ;
		_waves[i].func = WFSin;
	}
	
	SeaPatchRoot_cat.debug() << "SeaPatchRoot created" << std::endl;
}

void SeaPatchRoot::allocate_wave(int waveId) {
	if (waveId >= 0 && waveId < MAX_WAVES) {
		// Wave already allocated in constructor
	}
}

void SeaPatchRoot::reset_properties() {
	// Reset all properties to defaults
	_seaLevel = 0.0f;
	_colorHigh = LColorf(1, 1, 1, 1);
	_colorLow = LColorf(0, 0, 0, 1);
	_overallSpeed = 1.0f;
	_uvSpeed = LVecBase2f(0, 0);
	_uvScale = LVecBase2f(1, 1);
	_threshold = 0.0f;
	_radius = 1000.0f;
	
	for (int i = 0; i < MAX_WAVES; i++) {
		_waves[i].enabled = false;
		_waves[i].amplitude = 0.0f;
		_waves[i].length = 1.0f;
		_waves[i].speed = 1.0f;
		_waves[i].direction = LVecBase2f(1, 0);
		_waves[i].choppy_k = 0;
		_waves[i].target = WTZ;
		_waves[i].func = WFSin;
	}
}

void SeaPatchRoot::assign_environment_from(SeaPatchRoot *other) {
	if (other != nullptr) {
		_center = other->_center;
		_anchor = other->_anchor;
	}
}

float SeaPatchRoot::calc_height(float x, float y, float dist2) const {
	// Calculate wave height by summing contributions from all enabled waves
	if (!_animate_height) {
		return _seaLevel;
	}
	
	float height = _seaLevel;
	float radius2 = _radius * _radius;
	
	// Apply distance-based attenuation if beyond threshold
	float attenuation = 1.0f;
	if (dist2 > radius2) {
		attenuation = 0.0f;
	} else if (_threshold > 0 && dist2 > _threshold * _threshold) {
		float t = (csqrt(dist2) - _threshold) / (_radius - _threshold);
		attenuation = 1.0f - t;
	}
	
	// Sum up wave contributions
	for (int i = 0; i < _num_waves; i++) {
		if (_waves[i].enabled && _waves[i].target == WTZ) {
			// Simple sine wave
			if (_waves[i].func == WFSin && _waves[i].length > 0) {
				float phase = (x * _waves[i].direction[0] + y * _waves[i].direction[1]) / _waves[i].length;
				phase += _waves[i].speed * _overallSpeed;
				height += _waves[i].amplitude * csin(phase * 2.0f * MathNumbers::pi_f) * attenuation;
			}
		}
	}
	
	return height;
}

float SeaPatchRoot::calc_filtered_height(float x, float y, float minWaveLength, float dist2) const {
	// Calculate wave height but only for waves with length >= minWaveLength
	if (!_animate_height) {
		return _seaLevel;
	}
	
	float height = _seaLevel;
	float radius2 = _radius * _radius;
	
	float attenuation = 1.0f;
	if (dist2 > radius2) {
		attenuation = 0.0f;
	} else if (_threshold > 0 && dist2 > _threshold * _threshold) {
		float t = (csqrt(dist2) - _threshold) / (_radius - _threshold);
		attenuation = 1.0f - t;
	}
	
	// Sum up wave contributions for waves >= minWaveLength
	for (int i = 0; i < _num_waves; i++) {
		if (_waves[i].enabled && _waves[i].target == WTZ && _waves[i].length >= minWaveLength) {
			if (_waves[i].func == WFSin && _waves[i].length > 0) {
				float phase = (x * _waves[i].direction[0] + y * _waves[i].direction[1]) / _waves[i].length;
				phase += _waves[i].speed * _overallSpeed;
				height += _waves[i].amplitude * csin(phase * 2.0f * MathNumbers::pi_f) * attenuation;
			}
		}
	}
	
	return height;
}

float SeaPatchRoot::calc_height_for_mass(float x, float y, float dist2, float mass, float area) const {
	// Calculate height adjusted for object mass (for buoyancy)
	// Heavier objects sink deeper into waves
	float base_height = calc_height(x, y, dist2);
	
	// Simple buoyancy: mass/area gives depression depth
	// Negative mass means the object floats higher
	if (area > 0) {
		base_height += mass / area;
	}
	
	return base_height;
}

LVector3f SeaPatchRoot::calc_normal(float height, float x, float y, float dist2) const {
	// Calculate surface normal by computing gradient of height field
	// Sample nearby points to estimate derivatives
	const float delta = 0.1f;
	
	float h_xp = calc_height(x + delta, y, dist2);
	float h_xn = calc_height(x - delta, y, dist2);
	float h_yp = calc_height(x, y + delta, dist2);
	float h_yn = calc_height(x, y - delta, dist2);
	
	// Compute gradients
	float dx = (h_xp - h_xn) / (2.0f * delta);
	float dy = (h_yp - h_yn) / (2.0f * delta);
	
	// Normal is (-dx, -dy, 1) normalized
	LVector3f normal(-dx, -dy, 1.0f);
	normal.normalize();
	
	return normal;
}

LVector3f SeaPatchRoot::calc_normal_for_mass(float height, float x, float y, float dist2, float mass, float area) const {
	// Calculate normal using mass-adjusted heights
	const float delta = 0.1f;
	
	float h_xp = calc_height_for_mass(x + delta, y, dist2, mass, area);
	float h_xn = calc_height_for_mass(x - delta, y, dist2, mass, area);
	float h_yp = calc_height_for_mass(x, y + delta, dist2, mass, area);
	float h_yn = calc_height_for_mass(x, y - delta, dist2, mass, area);
	
	float dx = (h_xp - h_xn) / (2.0f * delta);
	float dy = (h_yp - h_yn) / (2.0f * delta);
	
	LVector3f normal(-dx, -dy, 1.0f);
	normal.normalize();
	
	return normal;
}

float SeaPatchRoot::calc_flat_well_scale(float x, float y) const {
	// Return scaling factor for flat water areas
	// This would typically fade based on distance or other criteria
	return 1.0f;
}