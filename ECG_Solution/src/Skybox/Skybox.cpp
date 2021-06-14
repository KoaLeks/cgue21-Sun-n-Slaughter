#include "Skybox.h"

Skybox::Skybox(Shader* shader) {
    this->shader = shader;
    this->data = Mesh::createSkyboxMesh(50000, 50000, 50000);
    textureFaces[0] = "assets/skybox/right.jpg";
    textureFaces[1] = "assets/skybox/left.jpg";
    textureFaces[2] = "assets/skybox/top.jpg";
    textureFaces[3] = "assets/skybox/bottom.jpg";
    textureFaces[4] = "assets/skybox/front.jpg";
    textureFaces[5] = "assets/skybox/back.jpg";
    this->initBuffer();
    this->loadCubemap();
};

Skybox::~Skybox() {
    glDeleteBuffers(1, &skyboxVbo);
    glDeleteVertexArrays(1, &skyboxVao);
}

void Skybox::draw(PlayerCamera& camera, float brightness) {

    shader->use();
    glDepthMask(GL_FALSE);
    glm::mat4 viewProjMatrix = camera.getViewProjectionMatrix();
    shader->setUniform("viewProjMatrix", viewProjMatrix);
    shader->setUniform("brightness", brightness);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    glBindVertexArray(skyboxVao);
    glDrawArrays(GL_TRIANGLES, 0, data.indices.size());
    glDepthMask(GL_TRUE);
    glBindVertexArray(0);
    shader->unuse();
}

void Skybox::initBuffer() {
    // create VAO
    glGenVertexArrays(1, &skyboxVao);
    glBindVertexArray(skyboxVao);

    glGenBuffers(1, &skyboxVbo);

    glBindBuffer(GL_ARRAY_BUFFER, skyboxVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * data.positions.size(), data.positions.data(), GL_STATIC_DRAW);

    // bind positions to location 0
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Skybox::loadCubemap() {
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    unsigned char* data;
    for (unsigned int i = 0; i < 6; i++)
    {
        data = stbi_load(textureFaces[i], &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << textureFaces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}