#include "Mesh.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb_image.h>
#include <filesystem>

#include "Logger.hpp"

EMLogger m_logger("Meshes");

namespace fs = std::filesystem;

std::vector<std::shared_ptr<EMMesh>> EMMesh::load(const char* filePath)
{
    std::vector<std::shared_ptr<EMMesh>> meshes;
    fs::path directory = fs::path(filePath).parent_path();
    fs::path fileName = fs::path(filePath).stem();

    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate);

    if(!scene)
    {
        m_logger.errorf("Cannot load \"%s\"", filePath);
        return meshes;
    }

    for(unsigned int i = 0; i < scene->mNumMeshes; i++)
    {
        std::shared_ptr<EMMesh> emmesh = std::make_shared<EMMesh>();
        meshes.push_back(emmesh);
        const aiMesh* mesh = scene->mMeshes[i];

        if(mesh->mName.length) emmesh->m_name = mesh->mName.data;
        else emmesh->m_name = fileName.u8string();

        m_logger.infof("Loading mesh \"%s\"", emmesh->m_name.c_str());
        
        emmesh->m_numVerticies = mesh->mNumVertices;

        // Get Position data
        for(unsigned int j = 0; j < mesh->mNumVertices; j++)
        {
            aiVector3D pos = mesh->mVertices[j];
            emmesh->m_positions.emplace_back(pos.x , pos.y, pos.z);
        }

        // Get UV data
        if(mesh->mTextureCoords[0])
            for(unsigned int j = 0; j < mesh->mNumVertices; j++)
            {
                aiVector3D uv = mesh->mTextureCoords[0][j];
                emmesh->m_uvs.emplace_back(uv.x, uv.y);
            }

        // Get Normal data
        for(unsigned int j = 0; j < mesh->mNumVertices; j++)
        {
            aiVector3D norm = mesh->mNormals[j];
            emmesh->m_normals.emplace_back(norm.x, norm.y, norm.z);
        }

        // Get Color data
        if(mesh->mColors[0])
            for(unsigned int j = 0; j < mesh->mNumVertices; j++)
            {
                aiColor4D color = mesh->mColors[0][j];
                emmesh->m_colors.emplace_back(color.r, color.g, color.b, color.a);
            }

        // Get Index data
        emmesh->m_numIndicies = mesh->mNumFaces * 3;
        for(unsigned int j = 0; j < mesh->mNumFaces; j++)
        {
            uint32_t* faceIndicies = mesh->mFaces[j].mIndices;
            emmesh->m_indicies.emplace_back(faceIndicies[0]);
            emmesh->m_indicies.emplace_back(faceIndicies[1]);
            emmesh->m_indicies.emplace_back(faceIndicies[2]);
        }

        // Get texture (if any)
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        stbi_uc* image = NULL;
        int width, height, channels;
        if(material && aiGetMaterialTextureCount(material, aiTextureType_DIFFUSE))
        {
            aiString textureName;
            aiGetMaterialTexture(material, aiTextureType_DIFFUSE, 0, &textureName);
            std::string texturePath = (directory / fs::path(textureName.data)).u8string();

            stbi_set_flip_vertically_on_load(true);
            image = stbi_load(texturePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

            if(!image)
            {
                m_logger.submodule(emmesh->getName()).errorf("Failed to load texture \"%s\"", texturePath.c_str());
                return meshes;
            }
        }

        if(image)
        {
            glGenTextures(1, &emmesh->m_glTexture);
            glBindTexture(GL_TEXTURE_2D, emmesh->m_glTexture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
            stbi_image_free(image);
        }

        emmesh->m_positions.shrink_to_fit();
        emmesh->m_uvs.shrink_to_fit();
        emmesh->m_normals.shrink_to_fit();
        emmesh->m_colors.shrink_to_fit();
    }

    return meshes;
}

EMMesh::EMMesh() :
    m_numVerticies(0),
    m_numIndicies(0),
    m_isRenderable(false),
    m_advised(false),
    m_glVAO(0),
    m_glVBO(0),
    m_glEBO(0),
    m_glTexture(0)
{

}

EMMesh::~EMMesh()
{
    if(m_isRenderable)
    {
        glDeleteVertexArrays(1, &m_glVAO);
        glDeleteBuffers(1, &m_glVBO);
        glDeleteBuffers(1, &m_glEBO);
    }

    if(m_glTexture)
    {
        glDeleteTextures(1, &m_glTexture);
    }
}

uint32_t EMMesh::getTextureHandle() const
{
    return m_glTexture;
}

void EMMesh::putMeshArrays(EMMeshBuilder& meshBuilder) const
{
    const EMVertexFormat& vtxFmt = meshBuilder.getVertexFormat();
    
    if(m_positions.empty() || m_indicies.empty()) return;

    for(size_t i = 0; i < m_numIndicies; i++)
    {
        uint32_t index = m_indicies[i];

        putVertex(meshBuilder, vtxFmt, index);
    }
}

void EMMesh::putMeshElements(EMMeshBuilder& meshBuilder) const
{
    const EMVertexFormat& vtxFmt = meshBuilder.getVertexFormat();

    if(m_positions.empty() || m_indicies.empty()) return;

    meshBuilder.indexv(m_numIndicies, m_indicies.data());

    for(size_t i = 0; i < m_numVerticies; i++)
    {
        putVertex(meshBuilder, vtxFmt, (uint32_t) i);
    }
}

void EMMesh::makeRenderable(EMVertexFormat vtxFmt)
{
    if(m_isRenderable) return;

    EMMeshBuilder meshBuilder(vtxFmt);

    putMeshElements(meshBuilder);

    size_t vertexBufferSize, indexBufferSize;

    const uint8_t* vertexData = meshBuilder.getVertexBuffer(&vertexBufferSize);
    const uint32_t* indexData = meshBuilder.getIndexBuffer(&indexBufferSize);

    glGenVertexArrays(1, &m_glVAO);
    glGenBuffers(1, &m_glVBO);
    glGenBuffers(1, &m_glEBO);

    glBindVertexArray(m_glVAO);
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_glVBO);
        glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, vertexData, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, indexData, GL_STATIC_DRAW);

        vtxFmt.apply();
    }
    glBindVertexArray(0);

    m_isRenderable = true;
}

void EMMesh::render(int mode) const
{
    if(!m_isRenderable && !m_advised)
    {
        m_advised = true;
        m_logger.submodule(getName()).warnf("Mesh has not yet been made renderable. Call makeRenderable first.");
    }

    if(m_glTexture)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_glTexture);
    }

    glBindVertexArray(m_glVAO);
    {
        glDrawElements(mode, m_numIndicies, GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(0);
}

const glm::vec3* EMMesh::getPositions() const
{
    if(m_positions.empty()) return NULL;
    return m_positions.data();
}

const glm::vec2* EMMesh::getUVs() const
{
    if(m_uvs.empty()) return NULL;
    return m_uvs.data();
}

const glm::vec3* EMMesh::getNormals() const
{
    if(m_normals.empty()) return NULL;
    return m_normals.data();
}

const glm::vec4* EMMesh::getColors() const
{
    if(m_colors.empty()) return NULL;
    return m_colors.data();
}

size_t EMMesh::numVerticies() const
{
    return m_numVerticies;
}

size_t EMMesh::numIndicies() const
{
    return m_numIndicies;
}

const char* EMMesh::getName() const
{
    return m_name.c_str();
}

void EMMesh::putVertex(EMMeshBuilder& meshBuilder, const EMVertexFormat& vtxFmt, uint32_t vertexID) const
{
    bool hasUVs = !m_uvs.empty();
    bool hasNormals = !m_normals.empty();
    bool hasColors = !m_colors.empty();

    for(int attrb = 0; attrb < vtxFmt.size; attrb++)
    {
        uint32_t attrbUsage = vtxFmt.attributes[attrb].getUsage();

        glm::vec2 vec2;
        glm::vec3 vec3;
        glm::vec4 vec4;

        switch (attrbUsage)
        {
        case EMVF_ATTRB_USAGE_POS:
            vec3 = m_positions[vertexID];
            meshBuilder.position(vec3.x, vec3.y, vec3.z);
            break;
        case EMVF_ATTRB_USAGE_UV:
            if(hasUVs)
            {
                vec2 = m_uvs[vertexID];
                meshBuilder.uv(vec2.s, vec2.t);
            }
            else meshBuilder.uvDefault();
            break;
        case EMVF_ATTRB_USAGE_NORMAL:
            if(hasNormals)
            {
                vec3 = m_normals[vertexID];
                meshBuilder.normal(vec3.x, vec3.y, vec3.z);
            }
            else meshBuilder.normalDefault();
            break;
        case EMVF_ATTRB_USAGE_COLOR:
            if(hasColors)
            {
                vec4 = m_colors[vertexID];
                meshBuilder.colorRGBA(vec4.r, vec4.g, vec4.b, vec4.a);
            }
            else meshBuilder.colorDefault();
            break;
        case EMVF_ATTRB_USAGE_TEXID:
            meshBuilder.texid(0);
            break;
        }
    }
}