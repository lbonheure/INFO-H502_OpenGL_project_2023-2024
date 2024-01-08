// This code is heavily inspired by that of Etay Meiri in its tutorial: https://github.com/emeiri/ogldev/tree/master

#ifndef WORLD_TRANSFORM_H
#define WORLD_TRANSFORM_H

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale

class WorldTrans {
 public:
    WorldTrans() {}

    void SetScale(float scale)
    {
        m_scale = scale;
    }

    void SetRotation(float x, float y, float z)
    {
        m_rotation.x = x;
        m_rotation.y = y;
        m_rotation.z = z;
    }

    void SetRotation(const glm::vec3& rotation)
    {
        m_rotation = rotation;
    }
    void SetPosition(float x, float y, float z)
    {
        m_pos.x = x;
        m_pos.y = y;
        m_pos.z = z;
    }

    void SetPosition(const glm::vec3& WorldPos)
    {
        m_pos = WorldPos;
    }

    void Rotate(float x, float y, float z)
    {
        m_rotation.x += x;
        m_rotation.y += y;
        m_rotation.z += z;
    }

    glm::mat4 GetMatrix() const
    {
        glm::mat4 WorldTransformation = glm::mat4(1.0);
        WorldTransformation = glm::scale(WorldTransformation, glm::vec3(m_scale));

        WorldTransformation = glm::rotate(WorldTransformation, glm::radians(m_rotation.x), glm::vec3(1,0,0));
        WorldTransformation = glm::rotate(WorldTransformation, glm::radians(m_rotation.y), glm::vec3(0,1,0));
        WorldTransformation = glm::rotate(WorldTransformation, glm::radians(m_rotation.z), glm::vec3(0,0,1));

        WorldTransformation = glm::translate(WorldTransformation, m_pos);

        return WorldTransformation;
    }

    glm::vec3 WorldPosToLocalPos(const glm::vec3& WorldPos) const
    {
        glm::mat4 WorldToLocalTranslation = GetReversedTranslationMatrix();
        glm::mat4 WorldToLocalRotation = GetReversedRotationMatrix();
        glm::mat4 WorldToLocalTransformation = WorldToLocalRotation * WorldToLocalTranslation;
        glm::vec4 WorldPos4f = glm::vec4(WorldPos, 1.0f);
        glm::vec4 LocalPos4f = WorldToLocalTransformation * WorldPos4f;
        glm::vec3 LocalPos3f(LocalPos4f);
        return LocalPos3f;
    }

    glm::vec3 WorldDirToLocalDir(const glm::vec3& WorldDirection) const
    {
        glm::mat3 World3f(GetMatrix());  // Initialize using the top left corner

        // Inverse local-to-world transformation using transpose
        // (assuming uniform scaling)
        glm::mat3 WorldToLocal = glm::transpose(World3f);

        glm::vec3 LocalDirection = WorldToLocal * WorldDirection;

        LocalDirection = glm::normalize(LocalDirection);

        return LocalDirection;
    }

    glm::mat4 GetReversedTranslationMatrix() const
    {
        glm::mat4 ReversedTranslation = glm::mat4(1.0);
        ReversedTranslation =  glm::translate(ReversedTranslation, -m_pos);
        return ReversedTranslation;
    }

    glm::mat4 GetReversedRotationMatrix() const
    {
        glm::mat4 ReversedRotation = glm::mat4(1.0);

        ReversedRotation = glm::rotate(ReversedRotation, glm::radians(-m_rotation.x), glm::vec3(1,0,0));
        ReversedRotation = glm::rotate(ReversedRotation, glm::radians(-m_rotation.y), glm::vec3(0,1,0));
        ReversedRotation = glm::rotate(ReversedRotation, glm::radians(-m_rotation.z), glm::vec3(0,0,1));
        return ReversedRotation;
    }

    float GetScale() const { return m_scale; }
    glm::vec3 GetPos() const { return m_pos; }
    glm::vec3 GetRotation() const { return m_rotation; }

 private:
    float    m_scale    = 1.0f;
    glm::vec3 m_rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 m_pos      = glm::vec3(0.0f, 0.0f, 0.0f);
};


#endif