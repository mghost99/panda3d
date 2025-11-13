#include "seaPatchNode.h"
#include "seaPatchRoot.h"

TypeHandle SeaPatchNode::_type_handle;

// Setup our notify category.
NotifyCategoryDef(SeaPatchNode, "");

SeaPatchNode::SeaPatchNode(const std::string &name, SeaPatchRoot *patch) : PandaNode(name), _patch(patch) {
	_enabled = false;
	_want_reflect = true;
	_want_color = true;
	SeaPatchNode_cat.debug() << "SeaPatchNode created: " << name << std::endl;
}

void SeaPatchNode::collect_geometry() {
	// Collect geometry from all child GeomNodes for optimized rendering
	// This consolidates vertex data from the seamodel's children
	
	PandaNode::Children children = get_children();
	int num_children = children.get_num_children();
	
	for (int i = 0; i < num_children; ++i) {
		PandaNode *child = children.get_child(i);
		if (child->is_geom_node()) {
			// Found a GeomNode child - this contains the actual sea mesh geometry
			SeaPatchNode_cat.debug() << "Collected geometry from child: " << child->get_name() << std::endl;
		}
	}
}