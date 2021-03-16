#include "raytracer_bvh.h"

namespace P3 {

bool BVHNode::IsLeaf() {
	return holds_alternative<vector<BVHNode*>>(holding);
}

BVHNode::BVHNode(Geometry* geo) {
	holding = geo;
	bb = geo->GetBoundingBox();
}

BVHNode::BVHNode(vector<BVHNode*> nodes) {
	holding = nodes;
}

void BVHNode::AddGeometry(Geometry* geo) {
	if (IsLeaf()) {
		BVHNode* node1 = new BVHNode(get<Geometry*>(holding));
		BVHNode* node2 = new BVHNode(geo);
		holding = vector<BVHNode*>{node1, node2};
		bb = bb.Union(node2->bb);
	} else {
		BoundingBox gbb = geo->GetBoundingBox();
		vector<BVHNode*> intersecting;
		vector<BVHNode*>& children = get<vector<BVHNode*>>(holding);
		for (BVHNode* node : children) {
			if (gbb.Intersects(node->bb))
				intersecting.push_back(node);
		}
		// If it intersects with all or none, it is proper to be a leaf
		if (intersecting.size() == 0 || intersecting.size() == children.size()) {
			children.push_back(new BVHNode(geo));
		}

		// We should be a child of the node we intersect (and call recursively)
		if (intersecting.size() == 1) {
			intersecting.at(0)->AddGeometry(geo);
		}

		// We should make a new node, and move all the intersecting nodes into that.
		for (BVHNode* np : intersecting) {
			remove(children.begin(), children.end(), np);
		}
	}
}

void BVHNode::UpdateBB() {
	if (IsLeaf()) throw runtime_error("UpdateBB called on Leaf node, not supported");

	for (BVHNode* node : get<vector<BVHNode*>>(holding)) {
		bb = bb.Union(node->bb);
	}
}

}  //namespace P3
