#include "raylib.h"
#include <cmath>
#include <utility>
#include <assert.h>
#include <float.h>
#include <vector>

const int screenWidth = 1200;
const int screenHeight = 800;

inline Color RandomColor() {
	return { (unsigned char)GetRandomValue(0,255), (unsigned char)GetRandomValue(0,255) , (unsigned char)GetRandomValue(0,255), 255 };
}

struct Square {
	Vector2 min;
	Vector2 max;
	Color color;

	Square() : min{}, max{}, color{RandomColor()} {}
	Square(const Square& copy) : min(copy.min), max(copy.max), color(copy.color) {}

	Rectangle GetRect() const {
		Rectangle ret;
		ret.width = std::abs(min.x - max.x);
		ret.height = std::abs(min.y - max.y);
		ret.x = std::fminf(min.x, max.x);
		ret.y = std::fminf(min.y, max.y);
		return ret;
	}

	inline Vector2 GetCenter() {
		return { (min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f };
	}

	void Fix() {
		Vector2 mincopy = min;
		Vector2 maxcopy = max;
		min.x = fminf(mincopy.x, maxcopy.x);
		min.y = fminf(mincopy.y, maxcopy.y);
		max.x = fmaxf(mincopy.x, maxcopy.x);
		max.y = fmaxf(mincopy.y, maxcopy.y);
	}

	void Fit(const Square& sq1, const Square& sq2) {
		min.x = fminf(sq1.min.x, sq2.min.x);
		min.y = fminf(sq1.min.y, sq2.min.y);
		max.x = fmaxf(sq1.max.x, sq2.max.x);
		max.y = fmaxf(sq1.max.y, sq2.max.y);
	}

	Vector2 GetSize() const {
		return { max.x - min.x, max.y - min.y };
	}

	void DrawFill() const {
		Rectangle rect = GetRect();
		DrawRectangleRec(rect, color);
	}

	void DrawLine(float thickness = 3.0f) const {
		Rectangle rect = GetRect();
		rect.x -= thickness;
		rect.y -= thickness;
		rect.width += thickness * 2.0f;
		rect.height += thickness * 2.0f;
		DrawRectangleLinesEx(rect, thickness, color);
	}

	inline bool HasPoint(const Vector2& vec) const {
		return (min.x <= vec.x && vec.x <= max.x
			&& min.y <= vec.y && vec.y <= max.y);
	}
};

//struct SquareHash {
//	inline std::size_t operator()(const Square& v) const {
//		std::hash<Shape*> ptr_hasher;
//		return ptr_hasher(v.first) ^ ptr_hasher(v.second);
//	}
//};

bool IsSquareBoxFullyContained(const Square& inside, const Square& outside) {
	const Vector2 imin = inside.min;
	const Vector2 imax = inside.max;
	const Vector2 omin = outside.min;
	const Vector2 omax = outside.max;

	return omin.x <= imin.x && imax.x <= omax.x
		&& omin.y <= imin.y && imax.y <= omax.y;
}

//struct BVHNode {
//	Square bounding;
//	BVHNode* parent;
//	BVHNode* child[2];
//
//	BVHNode() : bounding{}, parent(nullptr), child{ nullptr, nullptr } {}
//	BVHNode(Square square) : bounding(square), parent(nullptr), child{ nullptr, nullptr } {}
//
//	inline bool IsLeaf() const {
//		return !child[0] && !child[1];
//	}
//
//	void FitChildren() {
//		// not checking children validity
//		bounding.Fit(child[0]->bounding, child[1]->bounding);
//	}
//
//	inline float MidPosComp(int comp) {
//		return (*(&bounding.min.x + comp) + *(&bounding.max.x + comp)) * 0.5f;
//	}
//
//	void DrawParentLines(int depth) const {
//		if (parent && depth > 0) {
//			parent->bounding.DrawLine();
//			parent->DrawParentLines(depth - 1);
//		}
//	}
//};
//
//void SwapIfGreaterMidPosComp(BVHNode*& node1, BVHNode*& node2, int comp) {
//	if (node1->MidPosComp(comp) > node2->MidPosComp(comp)) {
//		std::swap(node1, node2);
//	}
//}
//
//class BVH {
//public:
//	BVHNode root;
//
//	void GroupToLeaf(BVHNode*& leaf, Square& square) {
//		// leaf MUST be a leaf
//		assert(leaf->IsLeaf() && "BVH logic error : grouping must be done to a leaf");
//
//		BVHNode* newsub = new BVHNode();
//		newsub->parent = leaf->parent;
//		leaf->parent = newsub;
//		newsub->child[0] = leaf;
//		newsub->child[1] = new BVHNode(square);
//		newsub->child[1]->parent = newsub;
//		newsub->FitChildren();
//		leaf = newsub;
//	}
//
//	void InsertRecursive(BVHNode& node, Square& square) {
//		assert(node.child[0] && "BVH logic error : first child must always exist");
//
//		// at this point we're guaranteed to already fit inside the node
//		assert(IsSquareBoxFullyContained(square, node.bounding) && "BVH logic error : the new square should be contained in this node");
//
//		// First Case : can fit in one of the children
//		for (auto& child : node.child) {
//			if (child && IsSquareBoxFullyContained(square, child->bounding)) {
//				if (child->IsLeaf()) {
//					GroupToLeaf(child, square);
//				}
//				else {
//					InsertRecursive(*child, square);
//				}
//				return;
//			}
//		}
//
//		// Case : child slot available
//		if (!node.child[1]) {
//			node.child[1] = new BVHNode(square);
//			node.child[1]->parent = &node;
//			node.FitChildren();
//			return;
//		}
//
//		// Case : one of the nodes is a leaf, group them
//		bool leaf0 = node.child[0]->IsLeaf();
//		bool leaf1 = node.child[1]->IsLeaf();
//		if (leaf0 != leaf1)
//		{
//			BVHNode*& leaf = leaf0 ? node.child[0] : node.child[1];
//			GroupToLeaf(leaf, square);
//			return;
//		}
//
//		// Final case : calculate and sort all three nodes
//		Vector2 size = node.bounding.GetSize();
//		int axis = size.x >= size.y ? 0 : 1;
//
//		BVHNode* insertedone = new BVHNode(square);
//		BVHNode* newnodes[3] = { node.child[0], node.child[1], insertedone };
//		SwapIfGreaterMidPosComp(newnodes[0], newnodes[1], axis);
//		SwapIfGreaterMidPosComp(newnodes[1], newnodes[2], axis);
//		SwapIfGreaterMidPosComp(newnodes[0], newnodes[1], axis);
//
//		BVHNode* newsub = new BVHNode();
//		newsub->parent = &node;
//		newsub->child[0] = newnodes[0];
//		newsub->child[1] = newnodes[1];
//		newnodes[0]->parent = newsub;
//		newnodes[1]->parent = newsub;
//		newsub->FitChildren();
//
//		node.child[0] = newsub;
//
//		// Edge case : the third alone node is our new one, and can fit inside the new first child
//		//if (insertedone == newnodes[2] && IsSquareBoxFullyContained(insertedone->bounding, newsub->bounding)) {
//		//	node.child[1] = nullptr;
//		//	delete insertedone;
//		//	InsertRecursive(*node.child[0], square);
//		//	return;
//		//}
//
//		node.child[1] = newnodes[2];
//		node.child[1]->parent = &node;
//	}
//
//	void Insert(Square& square) {
//		square.Fix();
//
//		// root exception : entire BVH first node
//		if (!root.child[0]) {
//			root.bounding = square;
//			root.bounding.color = RandomColor();
//			root.child[0] = new BVHNode(square);
//			root.child[0]->parent = &root;
//			return;
//		}
//
//		// root exception : making the new square always fit the root
//		root.bounding.Fit(root.bounding, square);
//
//		InsertRecursive(root, square);
//	}
//
//	void RecursiveDraw(const BVHNode& node, bool lines) const {
//		if (node.IsLeaf()) {
//			node.bounding.DrawFill();
//		}
//		else {
//			for (auto& child : node.child) {
//				if (child) {
//					RecursiveDraw(*child, lines);
//				}
//			}
//			if (lines) {
//				node.bounding.DrawLine();
//			}
//		}
//	}
//
//	void Draw(bool lines) {
//		RecursiveDraw(root, lines);
//	}
//
//	void ClearRecursive(BVHNode& node) {
//		for (auto& child : node.child) {
//			if (child) {
//				ClearRecursive(*child);
//				delete child;
//				child = nullptr;
//			}
//		}
//	}
//
//	void Clear() {
//		ClearRecursive(root);
//	}
//
//	const BVHNode* PickNodeRecursive(const BVHNode& node, const Vector2& vec, int* depth) const {
//		if (depth) {
//			++ *depth;
//		}
//		const Square& bb = node.bounding;
//		if (bb.min.x <= vec.x && vec.x <= bb.max.x
//			&& bb.min.y <= vec.y && vec.y <= bb.max.y) {
//			if (node.IsLeaf()) {
//				return &node;
//			}
//			for (const auto& child : node.child) {
//				const BVHNode* ret = PickNodeRecursive(*child, vec, depth);
//				if (ret) {
//					return ret;
//				}
//			}
//		}
//		if (depth) {
//			-- *depth;
//		}
//		return nullptr;
//	}
//
//	const BVHNode* PickNode(const Vector2& vec, int* depth=nullptr) const {
//		if (depth) {
//			*depth = 0;
//		}
//		return PickNodeRecursive(root, vec, depth);
//	}
//};

inline float& Vec2Comp(Vector2& v, int comp) {
	return *(&v.x + comp);
}

#define COMP_X 0
#define COMP_Y 1
#define COMP_MAX 2

struct QTNode {
	Square bounding;
	QTNode* parent;
	QTNode* child[2];
	std::vector<Square> entities; // use unordered_set when I'm not lazy to implement a hash

	QTNode() : bounding{}, parent(nullptr), child{ nullptr, nullptr } {}
	QTNode(Square square) : bounding(square), parent(nullptr), child{ nullptr, nullptr } {}

	inline bool IsLeaf() const {
		return !child[0] && !child[1];
	}

	void FitChildren() {
		// not checking children validity
		bounding.Fit(child[0]->bounding, child[1]->bounding);
	}

	inline float MidPosComp(int comp) {
		return (*(&bounding.min.x + comp) + *(&bounding.max.x + comp)) * 0.5f;
	}

	void DrawHierarchyLines(int depth) const {
		if (depth > 0) {
			bounding.DrawLine();
			if (parent) {
				parent->DrawHierarchyLines(depth - 1);
			}
		}
	}
};

class QuadTree {
public:
	QTNode root;

	QuadTree() {
		root.bounding.min = { -2048.0f, -2048.0f };
		root.bounding.max = { 2048.0f, 2048.0f };
	}

	void Insert(Square& square) {
		Vector2 c = square.GetCenter();
		int comp = 0;

		QTNode* curnode = &root;

		while (true) {
			// safety check
			assert(IsSquareBoxFullyContained(square, curnode->bounding));

			// calculate left and right child bounding boxes
			Square leftbb = curnode->bounding;
			float halfcompsize = (Vec2Comp(leftbb.max, comp) - Vec2Comp(leftbb.min, comp)) * 0.5f;
			Vec2Comp(leftbb.max, comp) = Vec2Comp(leftbb.min, comp) + halfcompsize;
			Square rightbb = curnode->bounding;
			Vec2Comp(rightbb.min, comp) = Vec2Comp(leftbb.max, comp);

			// check if we can fit in one of them
			int choice = -1;
			if (IsSquareBoxFullyContained(square, leftbb)) {
				choice = 0;
			}
			else if (IsSquareBoxFullyContained(square, rightbb)) {
				choice = 1;
			}

			// if not, we're inside curnode and not further
			if (choice < 0) {
				curnode->entities.push_back(square);
				return;
			}

			// otherwise, re-iterate in the chosen child (and create it if needed)
			QTNode*& child = curnode->child[choice];
			if (!child) {
				child = new QTNode;
				child->bounding = choice == 0 ? leftbb : rightbb;
				child->parent = curnode;
			}
			curnode = child;
			comp = (comp + 1) % COMP_MAX;
		}
	}

	void RecursiveDraw(const QTNode& node, bool lines) const {
		for (auto& entity : node.entities) {
			entity.DrawFill();
		}
		for (auto& child : node.child) {
			if (child) {
				RecursiveDraw(*child, lines);
			}
		}
		if (lines) {
			node.bounding.DrawLine();
		}
	}

	void Draw(bool lines) {
		RecursiveDraw(root, lines);
	}

	void ClearRecursive(QTNode& node) {
		for (auto& child : node.child) {
			if (child) {
				ClearRecursive(*child);
				delete child;
				child = nullptr;
			}
		}
	}

	void Clear() {
		ClearRecursive(root);
	}

	const Square* PickNodeRecursive(const QTNode& node, const Vector2& vec, int* depth, const QTNode** retparent) const {
		if (depth) {
			++ *depth;
		}
		
		// first check current entities
		for (auto& entity : node.entities) {
			if (entity.HasPoint(vec)) {
				if (retparent) {
					*retparent = &node;
				}
				return &entity;
			}
		}

		// check children
		for (auto& child : node.child) {
			if (child && child->bounding.HasPoint(vec)) {
				const Square* ret = PickNodeRecursive(*child, vec, depth, retparent);
				if (ret) {
					return ret;
				}
			}
		}

		if (depth) {
			-- *depth;
		}
		return nullptr;
	}

	const Square* PickNode(const Vector2& vec, int* depth=nullptr, const QTNode** retparent=nullptr) const {
		if (depth) {
			*depth = 0;
		}
		return PickNodeRecursive(root, vec, depth, retparent);
	}
};

int main() {

	InitWindow(screenWidth, screenHeight, "BVH Study");


	SetTargetFPS(60);

	Square currentSquare;

	QuadTree bvh;

	while (!WindowShouldClose()) {

		ClearBackground(RAYWHITE);

		if (IsMouseButtonPressed(0)) {
			currentSquare.min = GetMousePosition();
			currentSquare.color = RandomColor();
		}

		bool drawing = IsMouseButtonDown(0);
		if (drawing) {
			currentSquare.max = GetMousePosition();
		}

		if (IsMouseButtonReleased(0)) {
			bvh.Insert(currentSquare);
		}

		if (IsKeyPressed(KEY_SPACE)) {
			bvh.Clear();
		}

		BeginDrawing();

		bool line = IsKeyDown(KEY_L);
		bvh.Draw(!line);

		if (drawing) {
			DrawRectangleRec(currentSquare.GetRect(), currentSquare.color);
		}

		int depth = 0;
		if (line) {
			const QTNode* qtn;
			const Square* pick = bvh.PickNode(GetMousePosition(), &depth, &qtn);
			if (pick) {
				Square s = *pick;
				s.color = RandomColor();
				s.DrawFill();

				static float animline = 0.0f;

				animline += GetFrameTime() * 3.0f;
				if (animline > (float)depth) {
					animline = 0.0f;
				}

				qtn->DrawHierarchyLines((int)animline);
			}
		}

		DrawText(TextFormat("Depth : %i", depth), 20, 20, 10, BLACK);

		EndDrawing();
	}


	CloseWindow();
}