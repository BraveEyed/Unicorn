/*
* Copyright (C) 2017 by Godlike
* This code is licensed under the MIT license (MIT)
* (http://opensource.org/licenses/MIT)
*/

#include <unicorn/video/Mesh.hpp>

namespace unicorn
{
namespace video
{
Mesh::Mesh() :
    m_material(new Material)
    /* Here we initialize with new Material
       When we will call SetMaterial first time in constructor,
       m_material will be nullptr and m_material->DataUpdated.disconnect will crash. */
{
    auto const defaultMaterial = std::make_shared<Material>();

    SetMaterial(defaultMaterial);
}

Mesh::~Mesh()
{
    m_material->DataUpdated.clear();
}

void Mesh::SetMeshData(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
    m_vertices = vertices;
    m_indices = indices;

    VerticesUpdated.emit();
}

void Mesh::SetMaterial(std::shared_ptr<Material> material)
{
    m_material->DataUpdated.disconnect(this, &Mesh::OnMaterialUpdated);

    m_material = material;

    m_material->DataUpdated.connect(this, &Mesh::OnMaterialUpdated);

    MaterialUpdated.emit();
}

std::vector<Vertex> const& Mesh::GetVertices() const
{
    return m_vertices;
}

std::vector<uint32_t> const& Mesh::GetIndices() const
{
    return m_indices;
}

std::shared_ptr<Material> Mesh::GetMaterial() const
{
    return m_material;
}

void Mesh::OnMaterialUpdated()
{
    MaterialUpdated.emit();
}

}
}
