#ifndef SEAPATCHNODE_H
#define SEAPATCHNODE_H

#include "piratesbase.h"
#include "pandaNode.h"
#include "pointerTo.h"
#include "geomNode.h"

class SeaPatchRoot;

// Declare our notify category.
NotifyCategoryDecl(SeaPatchNode, EXPCL_PIRATES, EXPTP_PIRATES);

static const int noise_table_size = 64;

class EXPCL_PIRATES SeaPatchNode : public PandaNode {

PUBLISHED:

	enum Alpha_Type {
		AHIGH,
		ALOW,
		ATOTAL,
		ANONE,
	};

	SeaPatchNode(const std::string &name, SeaPatchRoot *patch);

	INLINE void enable();
	INLINE void disable();
	INLINE bool is_enabled() const;

	INLINE void set_want_reflect(bool flag);
	INLINE bool get_want_reflect() const;

	INLINE void set_want_color(bool flag);
	INLINE bool get_want_color() const;

	void collect_geometry();

private:
	bool _enabled;
	bool _want_reflect;
	bool _want_color;
	PT(SeaPatchRoot) _patch;

public:
	static TypeHandle get_class_type() {
		return _type_handle;
	}
	static void init_type() {
		PandaNode::init_type();
		register_type(_type_handle, "SeaPatchNode",
			PandaNode::get_class_type());
	}
	virtual TypeHandle get_type() const {
		return get_class_type();
	}
	virtual TypeHandle force_init_type() { init_type(); return get_class_type(); }

private:
	static TypeHandle _type_handle;
};

#include "seaPatchNode.I"

#endif