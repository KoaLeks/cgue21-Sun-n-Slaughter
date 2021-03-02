#pragma once
#include <vector>
#include <glm\detail\type_vec.hpp>
#include <glm\detail\type_mat.hpp>
#include <memory>
#include <GL\glew.h>
#include "MeshMaterial.h"

/*!
 * Stores all data for a Mesh object
 */
struct MeshData {
	/*!
	 * Vertex positions
	 */
	std::vector<glm::vec3> positions;
	/*!
	 * Mesh indices
	 */
	std::vector<unsigned int> indices;
	/*!
	 * Vertex normals
	 */
	std::vector<glm::vec3> normals;

	std::vector<glm::vec2> uv_coord;
};


class Mesh
{
protected:

	GLuint heightmap;
	const char* path;

	void loadHeightMap();

	/*!
	 * Vertex array object
	 */
	GLuint _vao;
	/*!
	 * Vertex buffer object that stores the vertex positions
	 */
	GLuint _vboPositions;
	/*!
	 * Vertex buffer object that stores the vertex normals
	 */
	GLuint _vboNormals;
	/*!
	 * Vertex buffer object that stores the indices
	 */
	GLuint _vboIndices;

	GLuint _vboUV;

	/*!
	 * Number of elements to be rendered
	 */
	unsigned int _elements;

	/*!
	 * Material of the Mesh object
	 */
	 //std::shared_ptr<Material> _material;
	std::shared_ptr<MeshMaterial> _material;

	/*!
	 * Model matrix of the object
	 */
	glm::mat4 _modelMatrix;

public:
	/*!
	 * Mesh object constructor
	 * Creates VAO and VBOs and binds them
	 * @param modelMatrix: model matrix of the object
	 * @param data: data for the Mesh object
	 * @param material: material of the Mesh object
	 */
	Mesh(glm::mat4 modelMatrix, MeshData& data, std::shared_ptr<MeshMaterial> material, const char* _path);
	Mesh(glm::mat4 modelMatrix, MeshData& data, std::shared_ptr<MeshMaterial> material);
	~Mesh();

	/*!
	 * Draws the object
	 * Uses the shader, sets the uniform and issues a draw call
	 */
	void draw();

	/*!
	 * Transforms the object, i.e. updates the model matrix
	 * @param transformation: the transformation matrix to be applied to the object
	 */
	void transform(glm::mat4 transformation);

	/*!
	 * Resets the model matrix to the identity matrix
	 */
	void resetModelMatrix();


	/*!
	 * Creates a plane Mesh
	 * @param width: width of the plane
	 * @param depth: length of the plane
	 * @return all cube data
	 */
	static MeshData createPlaneMesh(int dimensions);

	/*!
	 * Creates a cube Mesh
	 * @param width: width of the cube
	 * @param height: height of the cube
	 * @param depth: depth of the cube
	 * @return all cube data
	 */
	static MeshData createCubeMesh(float width, float height, float depth);

	/*!
	 * Creates a cube Mesh
	 * @param width: width of the cube
	 * @param height: height of the cube
	 * @param depth: depth of the cube
	 * @return all cube data
	 */
	static MeshData createSkyboxMesh(float width, float height, float depth);

	/*!
	 * Creates a cylinder Mesh
	 * @param segments: number of segments of the cylinder
	 * @param height: height of the cylinder
	 * @param radius: radius of the cylinder
	 * @return all cylinder data
	 */
	static MeshData createCylinderMesh(unsigned int segments, float height, float radius);
	/*!
	 * Creates a sphere Mesh
	 * @param longitudeSegments: number of longitude segments of the sphere
	 * @param latitudeSegments: number of latitude segments of the sphere
	 * @param radius: radius of the sphere
	 * @return all sphere data
	 */
	static MeshData createSphereMesh(unsigned int longitudeSegments, unsigned int latitudeSegments, float radius);
};