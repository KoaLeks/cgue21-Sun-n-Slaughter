#include "FrustumG.h"
#include "MeshMaterial.h"
#include "Mesh.h"

void FrustumG::setCamInternals(float fov, float ratio, float nearD, float farD) {
	_ratio = ratio;
	_fov = fov;
	_nearD = nearD;
	_farD = farD / 10;
	_tang = (float)tan(glm::radians(_fov) * 0.5);
	nh = _nearD * _tang;
	nw = nh * ratio;
	fh = _farD * _tang;
	fw = fh * _ratio;
}

void FrustumG::updateFOV(float fov) {
	_fov = fov;
	_tang = (float)tan(glm::radians(_fov) * 0.5);
	nh = _nearD * _tang;
	nw = nh * _ratio;
	fh = _farD * _tang;
	fw = fh * _ratio;
}

void FrustumG::setCamDef(glm::vec3& p, glm::vec3& l, glm::vec3& u) {
	glm::vec3 dir, nc, fc, X, Y, Z;

	Z = glm::normalize(l); 
	X = glm::normalize(glm::cross(glm::vec3(0, 1, 0), Z));
	Y = glm::normalize(glm::cross(Z, X));

	// compute the centers of the near and far planes
	fc = p - Z * _farD;
	nc = p - Z * _nearD;

	// compute the 4 corners of the frustum on the far plane
	ftl = fc + (Y * fh) - (X * fw);
	ftr = fc + (Y * fh) + (X * fw);
	fbl = fc - (Y * fh) - (X * fw);
	fbr = fc - (Y * fh) + (X * fw);

	// compute the 4 corners of the frustum on the near plane
	ntl = nc + (Y * nh) - (X * nw);
	ntr = nc + (Y * nh) + (X * nw);
	nbl = nc - (Y * nh) - (X * nw);
	nbr = nc - (Y * nh) + (X * nw);

	pl[TOP].setPoints(ntr, ntl, ftl);
	pl[BOTTOM].setPoints(nbl, nbr, fbr);
	pl[LEFT].setPoints(ntl, nbl, fbl);
	pl[RIGHT].setPoints(nbr, ntr, fbr);
	pl[NEARP].setPoints(ntl, ntr, nbr);
	pl[FARP].setPoints(ftr, ftl, fbl);
}

void FrustumG::drawBoundingBox(glm::vec3 pos) {
	debug->resetModelMatrix();
	debug->transform(glm::translate(glm::mat4(1), pos));
	debug->draw();
}

void FrustumG::setDebugMesh(Mesh& mesh) {
	debug = &mesh;
}

int FrustumG::boxInFrustumDebug(std::shared_ptr<std::vector<glm::vec3>> boundingBox, std::string enemy) {
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

			if (!enemy.compare("mob_enemy")) {
				drawBoundingBox(boundingBox->at(k));
			}
			// is the corner outside or inside
			float dist = pl[i].distance(boundingBox->at(k));
			if (dist < 0) {
				out++;
			} else {
				in++;
			}
			
		}

		if (!in) {
			return (OUTSIDE);
		} else if (out) {
			return (INTERSECT);
		}
	}

	return result;
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
			float dist = pl[i].distance(boundingBox->at(k));
			if (dist < 0) {
				out++;
			}
			else {
				in++;
			}

		}

		if (!in) {
			return (OUTSIDE);
		}
		else if (out) {
			return (INTERSECT);
		}
	}

	return result;
}


glm::vec3 Plane::setPoints(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) {
	glm::vec3 aux1, aux2;
	aux1 = v1 - v2;
	aux2 = v3 - v2;
	_norm = glm::normalize(glm::cross(aux2, aux1));
	_D = -(glm::dot(_norm, v2));
	return _norm;
}

float Plane::distance(glm::vec3 point) {
	float dist = _D + glm::dot(_norm, point);
	return dist;
}