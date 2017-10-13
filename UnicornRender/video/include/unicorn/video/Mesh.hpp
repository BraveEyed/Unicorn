/*
* Copyright (C) 2017 by Godlike
* This code is licensed under the MIT license (MIT)
* (http://opensource.org/licenses/MIT)
*/

#ifndef UNICORN_VIDEO_MESH_HPP
#define UNICORN_VIDEO_MESH_HPP

#include <unicorn/utility/SharedMacros.hpp>
#include <unicorn/video/Material.hpp>

#include <wink/signal.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <list>

namespace unicorn
{
namespace video
{

/** @brief Vertex information */
struct Vertex
{
    /** @brief Position of vertex */
    glm::vec3 pos;

    /** @brief Texture coordinates of vertex */
    glm::vec2 tc;
};

/** @brief Mesh data */
class Mesh
{
public:
    UNICORN_EXPORT Mesh(Material const& material);
    UNICORN_EXPORT ~Mesh() = default;

    /**
    * @brief Updates vertices and indices geometry
    * @param [in] vertices vertices data
    * @param [in] indices indices data
    */
    UNICORN_EXPORT void SetMeshData(std::vector<Vertex> const& vertices, std::vector<uint32_t> const& indices);

    /**
    * @brief Sets new material
    * @param [in] material updated material for current mesh
    */
    UNICORN_EXPORT void SetMaterial(Material const& material);

    /**
     * @brief Returns mesh vertices
     * @return Mesh vertices
     */
    UNICORN_EXPORT const std::vector<Vertex>& GetVertices() const;

    /**
    * @brief Returns mesh indices
    * @return Mesh indices
    */
    UNICORN_EXPORT std::vector<uint32_t> const& GetIndices() const;

    /**
    * @brief Returns mesh material
    * @return mesh material data
    */
    UNICORN_EXPORT Material const& GetMaterial() const;

    /** @brief Event triggered when material is changed */
    wink::signal<wink::slot<void()>> MaterialUpdated;

    /** @brief Event triggered when vertices are changed */
    wink::signal<wink::slot<void()>> VerticesUpdated;

    glm::mat4 modelMatrix;
private:
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
    Material m_material;
    
    struct m_transform
    {
        glm::vec3 scale;
        glm::vec3 rotationOrigin;
        glm::vec3 translation;
        glm::vec3 worldTranslation;
        glm::quat rotation;
    };
};
}
}

#endif // UNICORN_VIDEO_MESH_HPP
