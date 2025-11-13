#ifndef SEAPATCHROOT_H
#define SEAPATCHROOT_H

#include "piratesbase.h"

#include "notifyCategoryProxy.h"
#include "typedReferenceCount.h"
#include "nodePath.h"
#include "luse.h"
#include "geometricBoundingVolume.h"

// Declare our notify category.
NotifyCategoryDecl(SeaPatchRoot, EXPCL_PIRATES, EXPTP_PIRATES);

class EXPCL_PIRATES SeaPatchRoot : public TypedReferenceCount {

PUBLISHED:

	SeaPatchRoot();

	INLINE void enable();
	INLINE void disable();
	INLINE bool is_enabled() const;

	INLINE void set_center(NodePath center);
	INLINE LPoint3f get_center() const;

	INLINE void set_anchor(NodePath anchor);
	INLINE NodePath get_anchor() const;

	INLINE void set_sea_level(float level);
	INLINE float get_sea_level() const;

	// LColorf -> typedef for LVecBase4f.
	INLINE void set_high_color(LColorf color);
	INLINE LColorf get_high_color() const;

	INLINE void set_low_color(LColorf color);
	INLINE LColorf get_low_color() const;

	INLINE void set_overall_speed(float speed);
	INLINE float get_overall_speed() const;

	INLINE void set_uv_speed(LVecBase2f speed);
	INLINE LVecBase2f get_uv_speed() const;

	INLINE void set_uv_scale(LVecBase2f scale);
	INLINE LVecBase2f get_uv_scale() const;

	INLINE void set_threshold(float threshold);
	INLINE float get_threshold() const;

	INLINE void set_radius(float radius);
	INLINE float get_radius() const;

	// Wave management
	INLINE void enable_wave(int waveId);
	INLINE void disable_wave(int waveId);
	INLINE bool is_wave_enabled(int waveId) const;
	INLINE int get_num_waves() const;

	// Wave properties
	INLINE void set_wave_amplitude(int waveId, float amplitude);
	INLINE float get_wave_amplitude(int waveId) const;
	
	INLINE void set_wave_length(int waveId, float length);
	INLINE float get_wave_length(int waveId) const;
	
	INLINE void set_wave_speed(int waveId, float speed);
	INLINE float get_wave_speed(int waveId) const;
	
	INLINE void set_wave_direction(int waveId, LVecBase2f dir);
	INLINE LVecBase2f get_wave_direction(int waveId) const;
	
	INLINE void set_choppy_k(int waveId, int choppy);
	INLINE int get_choppy_k(int waveId) const;
	
	INLINE void set_wave_target(int waveId, int target);
	INLINE int get_wave_target(int waveId) const;
	
	INLINE void set_wave_func(int waveId, int func);
	INLINE int get_wave_func(int waveId) const;

	// Utilities
	void reset_properties();
	void assign_environment_from(SeaPatchRoot *other);
	
	// Animation
	INLINE void animate_height(bool flag);
	INLINE void animate_uv(bool flag);
	
	// Height calculation
	float calc_height(float x, float y, float dist2) const;
	float calc_filtered_height(float x, float y, float minWaveLength, float dist2) const;
	float calc_height_for_mass(float x, float y, float dist2, float mass, float area) const;
	LVector3f calc_normal(float height, float x, float y, float dist2) const;
	LVector3f calc_normal_for_mass(float height, float x, float y, float dist2, float mass, float area) const;
	float calc_flat_well_scale(float x, float y) const;

	// Wave enum types
	enum WaveTarget {
		WTZ = 0,
		WTU = 1,
		WTV = 2
	};
	
	enum WaveFunc {
		WFSin = 0,
		WFNoise = 1
	};

private:
	bool _enabled;
	LPoint3f _center;
	NodePath _anchor;
	float _seaLevel;
	LColorf _colorHigh;
	LColorf _colorLow;
	float _overallSpeed;
	LVecBase2f _uvSpeed;
	LVecBase2f _uvScale;
	float _threshold;
	float _radius;
	bool _animate_height;
	bool _animate_uv;
	
	// Wave data (support up to 16 waves)
	static const int MAX_WAVES = 16;
	struct WaveData {
		bool enabled;
		float amplitude;
		float length;
		float speed;
		LVecBase2f direction;
		int choppy_k;
		int target;
		int func;
	};
	WaveData _waves[MAX_WAVES];
	int _num_waves;

public:
	void allocate_wave(int waveId);

public:
	static TypeHandle get_class_type() {
		return _type_handle;
	}
	static void init_type() {
		TypedReferenceCount::init_type();
		register_type(_type_handle, "SeaPatchRoot",
			TypedReferenceCount::get_class_type());
	}
	virtual TypeHandle get_type() const {
		return get_class_type();
	}
	virtual TypeHandle force_init_type() { init_type(); return get_class_type(); }

private:
	static TypeHandle _type_handle;
};

#include "seaPatchRoot.I"

#endif