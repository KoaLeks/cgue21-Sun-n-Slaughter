#pragma once


#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Utils.h"
#include <vector>

class Plane {

public:
	Plane::Plane() {}
	Plane::~Plane() {}

	glm::vec3 _norm;
	float _A, _B, _C, _D;
	float distance(glm::vec3 point);
	void setPoints(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d);
};

// http://cgvr.informatik.uni-bremen.de/teaching/cg_literatur/lighthouse3d_view_frustum_culling/index.html
class FrustumG {

private:

	enum {
		TOP = 0, BOTTOM, LEFT,
		RIGHT, NEARP, FARP
	};

public:

	static enum { OUTSIDE, INTERSECT, INSIDE };

	Plane pl[6];

	glm::vec3 ntl, ntr, nbl, nbr, ftl, ftr, fbl, fbr;
	float _nearD, _farD, _ratio, _angle, _tang;
	float nw, nh, fw, fh;
	bool doCheck = true;

	FrustumG::FrustumG() {}
	FrustumG::~FrustumG() {}

	void setCamInternals(float angle, float ratio, float nearD, float farD);
	void setCamDef(glm::vec3 &p, glm::vec3 &l, glm::vec3 &u);
	int boxInFrustum(std::shared_ptr<std::vector<glm::vec3>> boundingBox);

};