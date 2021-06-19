#include "FrustumG.h"
#include "MeshMaterial.h"
#include "Mesh.h"

void FrustumG::setCamInternals(float fov, float ratio, float nearD, float farD) {
	_ratio = ratio;
	_fov = fov;
	_nearD = nearD;
	_farD = farD/10;
	_tang = (float)tan(glm::radians(_fov) * 0.5);
	nh = _nearD * _tang;
	nw = nh * ratio;
	fh = _farD * _tang;
	fw = fh * _ratio;
}

void FrustumG::draw(Mesh& mesh, glm::vec3 ftl, glm::vec3 ftr, glm::vec3 fbr, glm::vec3 fbl, glm::vec3 ntl, glm::vec3 ntr, glm::vec3 nbr, glm::vec3 nbl) {
	// far top left
	mesh.resetModelMatrix();
	mesh.transform(glm::translate(glm::mat4(1), ftl));
	mesh.draw();
	// far top right
	mesh.resetModelMatrix();
	mesh.transform(glm::translate(glm::mat4(1), ftr));
	mesh.draw();
	// far bottom right
	mesh.resetModelMatrix();
	mesh.transform(glm::translate(glm::mat4(1), fbr));
	mesh.draw();
	// far bottom left
	mesh.resetModelMatrix();
	mesh.transform(glm::translate(glm::mat4(1), fbl));
	mesh.draw();
	//// near top left
	//mesh.resetModelMatrix();
	//mesh.transform(glm::translate(glm::mat4(1), ntl));
	//mesh.draw();
	//// near top right
	//mesh.resetModelMatrix();
	//mesh.transform(glm::translate(glm::mat4(1), ntr));
	//mesh.draw();
	//// near bottom right
	//mesh.resetModelMatrix();
	//mesh.transform(glm::translate(glm::mat4(1), nbr));
	//mesh.draw();
	//// near bottom left
	//mesh.resetModelMatrix();
	//mesh.transform(glm::translate(glm::mat4(1), nbl));
	//mesh.draw();
}

void FrustumG::setCamDef2(glm::vec3& p, glm::vec3& l, glm::vec3& u, Mesh& mesh) {
	glm::vec3 dir, nc, fc, X, Y, Z;

	Z = glm::normalize(l); 
	X = glm::normalize(glm::cross(glm::vec3(0, 1, 0), Z));
	Y = glm::normalize(glm::cross(X, Z));

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

	//std::cout << "frustum top left x=" << fbr.x << ", y=" << fbr.y << ", z=" << fbr.z << std::endl;

	pl[TOP].setPoints(ntr, ntl, ftl);
	pl[BOTTOM].setPoints(nbl, nbr, fbr);
	pl[LEFT].setPoints(ntl, nbl, fbl);
	pl[RIGHT].setPoints(nbr, ntr, fbr);
	pl[NEARP].setPoints(ntl, ntr, nbr);
	pl[FARP].setPoints(ftr, ftl, fbl);

	draw(mesh, ftl, ftr, fbr, fbl, ntl, ntr, nbr, nbl);
	
}

void FrustumG::setCamDef(glm::vec3 &p, glm::vec3 &l, glm::vec3 &u) {

	glm::vec3 dir, nc, fc, X, Y, Z;

	// compute the Z axis of camera
	// this axis points in the opposite direction from 
	// the looking direction
	Z = glm::normalize(l);

	// X axis of camera with given "up" vector and Z axis
	X = glm::normalize(glm::cross(glm::vec3(0, 1, 0), Z));

	// the real "up" vector is the cross product of Z and X
	Y = glm::normalize(u);


	// compute the centers of the near and far planes
	nc = p - Z * _nearD;
	fc = p - Z * _farD;

	pl[NEARP].setNormalAndPoint(-Z, nc);
	pl[FARP].setNormalAndPoint(Z, fc);

	glm::vec3 aux, normal;

	aux = (nc + Y * nh) - p;
	normal = aux * X;
	pl[TOP].setNormalAndPoint(normal, nc + Y * nh);

	aux = (nc - Y * nh) - p;
	aux = glm::normalize(aux);
	normal = X * aux;
	pl[BOTTOM].setNormalAndPoint(normal, nc - Y * nh);

	aux = (nc - X * nw) - p;
	aux = glm::normalize(aux);
	normal = aux * Y;
	pl[LEFT].setNormalAndPoint(normal, nc - X * nw);

	aux = (nc + X * nw) - p;
	aux = glm::normalize(aux);
	normal = Y * aux;
	pl[RIGHT].setNormalAndPoint(normal, nc + X * nw);
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


void Plane::setPoints(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) {
	glm::vec3 aux1, aux2;
	aux1 = v1 - v2;
	aux2 = v3 - v2;
	_norm = glm::normalize(glm::cross(aux2, aux1));
	_D = -(glm::dot(_norm, v2));
}

float Plane::distance(glm::vec3 point) {
	return glm::dot(_norm, point) + _D;
}

void Plane::setNormalAndPoint(glm::vec3 normal, glm::vec3 point) {
	_norm = glm::normalize(normal);
	_D = -glm::dot(_norm, point);
}