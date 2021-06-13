#include "FrustumG.h"

void FrustumG::setCamInternals(float angle, float ratio, float nearD, float farD) {
	_ratio = ratio;
	_angle = angle;
	_nearD = nearD;
	_farD = farD;


	_tang = (float)tan(glm::radians(_angle) * 0.5);
	nh = _nearD * _tang;
	nw = nh * ratio;
	fh = _farD * _tang;
	fw = fh * _ratio;
}

void FrustumG::setCamDef(glm::vec3 &p, glm::vec3 &l, glm::vec3 &u) {

	glm::vec3 dir, nc, fc, X, Y, Z;

	// compute the Z axis of camera
	// this axis points in the opposite direction from 
	// the looking direction
	Z = glm::normalize(l);

	// the real "up" vector is the cross product of Z and X
	Y = glm::normalize(u);

	// X axis of camera with given "up" vector and Z axis
	X = glm::cross(Y, Z);

	// compute the centers of the near and far planes
	nc = p - Z * _nearD;
	fc = p - Z * _farD;

	// compute the 4 corners of the frustum on the near plane
	ntl = nc + Y * nh - X * nw;
	ntr = nc + Y * nh + X * nw;
	nbl = nc - Y * nh - X * nw;
	nbr = nc - Y * nh + X * nw;

	// compute the 4 corners of the frustum on the far plane
	ftl = fc + Y * fh - X * fw;
	ftr = fc + Y * fh + X * fw;
	fbl = fc - Y * fh - X * fw;
	fbr = fc - Y * fh + X * fw;

	// compute the six planes
	pl[TOP].setPoints(ntr, ntl, ftl, ftr);
	pl[BOTTOM].setPoints(nbl, nbr, fbr, fbl);
	pl[LEFT].setPoints(ntl, nbl, fbl, ftl);
	pl[RIGHT].setPoints(nbr, ntr, ftr, fbr);
	pl[NEARP].setPoints(ntl, ntr, nbr, nbl);
	pl[FARP].setPoints(ftr, ftl, fbl, fbr);

}

int FrustumG::boxInFrustum(std::shared_ptr<std::vector<glm::vec3>> boundingBox) {
	if (!doCheck) {
		return INSIDE;
	}

	int result = INSIDE;
	int out = 0;
	int in = 0;

	// for each plane do ...
	for (int i = 0; i < 6; i++) {
		out = 0;
		in = 0;

		for (int k = 0; k < boundingBox->size() && (in == 0 || out == 0); k++) {

			// is the corner outside or inside
			if (pl[i].distance(boundingBox->at(k)) < 0) {
				out++;
			} else {
				in++;
			}
		}

		//if all corners are out
		if (!in) {
			return (OUTSIDE);
		} else if (out) {
			result = INTERSECT;
		}
	}

	return result;
}


float Plane::distance(glm::vec3 point) {
	return glm::dot(_norm, point) + _D;
}

void Plane::setPoints(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d) {
	glm::vec3 v = b - a;
	glm::vec3 u = c - a;
	_norm = glm::normalize(glm::cross(v, u));
	_A = _norm.x;
	_B = _norm.y;
	_C = _norm.z;
	_D = glm::dot(-_norm, a);
}