/*
* Copyright (C) 2017 by Godlike
* This code is licensed under the MIT license (MIT)
* (http://opensource.org/licenses/MIT)
*/

#include <unicorn/video/Primitives.hpp>
#include <unicorn/utility/Logger.hpp>

#include <glm/gtc/constants.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <cassert>

namespace unicorn
{
namespace video
{
Mesh& Primitives::Box(Mesh& mesh)
{
    std::vector<Vertex> vertices{{
        //front
        {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f}},
        {{0.5f, -0.5f, 0.5f}, {1.0f, 1.0f}},
        {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}},

        //right
        {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f}},
        {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}},
        {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f}},

        //back
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},
        {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}},
        {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f}},
        {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f}},

        //left
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},
        {{-0.5f, -0.5f, 0.5f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},
        {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f}},

        //upper
        {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, -0.5f}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f}},

        //bottom
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},
        {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}},
        {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f}},
        {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}},
    }};

    mesh.SetMeshData(vertices, {
                         0, 1, 2, 0, 2, 3,
                         4, 5, 6, 4, 6, 7,
                         8, 9, 10, 8, 10, 11,
                         12, 13, 14, 12, 14, 15,
                         16, 17, 18, 16, 18, 19,
                         20, 21, 22, 20, 22, 23});

    return mesh;
}

Mesh& Primitives::Quad(Mesh& mesh)
{
    std::vector<Vertex> vertices{{
        {{-0.5f, -0.5f, 0.0f},{0.0f, 1.0f}} ,
        {{0.5f, -0.5f, 0.0f},{1.0f, 1.0f}} ,
        {{0.5f, 0.5f, 0.0f},{1.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.0f},{0.0f, 0.0f}},
    }};

    mesh.SetMeshData(vertices, {0, 1, 2, 2, 3, 0});

    return mesh;
}

Mesh& Primitives::Sphere(Mesh& mesh, float radius, uint32_t rings, uint32_t sectors)
{
    assert(radius > 0);
    assert(rings > 4 || sectors > 4);

    std::vector<uint32_t> indices;
    std::vector<Vertex> vertices;

    uint32_t vectorSize = rings * sectors;

    assert(vectorSize < vertices.max_size());

    vertices.resize(vectorSize);

    {
        float const R = 1.f / static_cast<float>(rings - 1);
        float const S = 1.f / static_cast<float>(sectors - 1);

        auto vert_iter = vertices.begin();
        for(uint32_t r = 0; r < rings; ++r)
        {
            for(uint32_t s = 0; s < sectors; s++)
            {
                float const y = glm::sin(-glm::half_pi<float>() + glm::pi<float>() * r * R);
                float const x = glm::cos(2 * glm::pi<float>() * s * S) * glm::sin(glm::pi<float>() * r * R);
                float const z = glm::sin(2 * glm::pi<float>() * s * S) * glm::sin(glm::pi<float>() * r * R);
                *vert_iter++ = {{x * radius, y * radius, z * radius}, {(sectors - s) * S, (rings - r) * R}};
            }
        }
    }

    indices.resize(sectors * rings * 6);
    auto indicesIt = indices.begin();
    for(uint32_t x = 0; x < sectors; ++x)
    {
        uint32_t const right = (x + 1) % sectors;
        for(uint32_t y = 0; y < rings; ++y)
        {
            uint32_t bottom = (y + 1) % rings;
            *indicesIt++ = x + y * sectors;
            *indicesIt++ = x + bottom * sectors;
            *indicesIt++ = right + y * sectors;
            *indicesIt++ = right + y * sectors;
            *indicesIt++ = x + bottom * sectors;
            *indicesIt++ = right + bottom * sectors;
        }
    }

    mesh.SetMeshData(vertices, indices);

    return mesh;
}

Mesh& Primitives::LoadMeshFromFile(Mesh& mesh, std::string const& path)
{
    Assimp::Importer importer;

    std::vector<uint32_t> indices;
    std::vector<Vertex> vertices;

    aiScene const* scene = importer.ReadFile(path,
        aiProcess_CalcTangentSpace       |
        aiProcess_Triangulate            |
        aiProcess_JoinIdenticalVertices  |
        aiProcess_SortByPType);

    if( !scene)
    {
        LOG_ERROR("Assimp import error %s", importer.GetErrorString());
        //return false;
    }

    aiMesh* assimpMesh = scene->mMeshes[scene->mRootNode->mChildren[0]->mMeshes[0]];

    for(unsigned int i = 0; i < assimpMesh->mNumVertices; i++)
    {
        Vertex vertex;

        glm::vec3 vector;
        vector.x = assimpMesh->mVertices[i].x;
        vector.y = assimpMesh->mVertices[i].y;
        vector.z = assimpMesh->mVertices[i].z;
        vertex.pos = vector;

        if(assimpMesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            vec.x = assimpMesh->mTextureCoords[0][i].x;
            vec.y = assimpMesh->mTextureCoords[0][i].y;
            vertex.tc = vec;
        }
        else
        {
            vertex.tc = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    for(unsigned int i = 0; i < assimpMesh->mNumFaces; i++)
    {
        aiFace face = assimpMesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    mesh.SetMeshData(vertices, indices);

    return mesh;
}

}
}
