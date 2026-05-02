#ifndef GPC_TRANSFORM_H
#define GPC_TRANSFORM_H

#include <bitset>

#include "GLM_Include.h"

namespace GPC {
    struct Transform3D;

    using Radians = float;
    using Degree = float;

    enum Axis : uint16_t
    {
        X = 1,
        Y = 2,
        Z = 4
    };

    enum EulerRotation : uint16_t {
        XYZ = X + (Y << 3) + (Z << 6),
        XZY = X + (Z << 3) + (Y << 6),
        YXZ = Y + (X << 3) + (Z << 6),
        YZX = Y + (Z << 3) + (X << 6),
        ZXY = Z + (X << 3) + (Y << 6),
        ZYX = Z + (Y << 3) + (X << 6)
    };

    class Transform {

        glm::mat4x4 m_RotationMatrix;
        glm::mat4x4 m_InverseRotationMatrix;

        glm::mat4x4 m_Local;
        glm::mat4x4 m_InverseLocal;

        glm::quat m_Rotation;

        glm::vec3 m_Right;
        glm::vec3 m_Up;
        glm::vec3 m_Forward;

        glm::vec3 m_Position;
        glm::vec3 m_Scale;

        std::bitset<4> m_IsDirty;

        Transform3D* mp_TransformComponent = nullptr;

        void ForceUpdateWorld();
        void ForceUpdateInverseWorld();
        void ForceUpdateRotationMatrix();
        void ForceUpdateInverseRotationMatrix();

        static constexpr uint64_t DIRTY_FLAG_INDEX_LOCAL = 0;
        static constexpr uint64_t DIRTY_FLAG_INDEX_INV_LOCAL = 1;
        static constexpr uint64_t DIRTY_FLAG_INDEX_ROTATION = 2;
        static constexpr uint64_t DIRTY_FLAG_INDEX_INV_ROTATION = 3;
    public:
        Transform();
        ~Transform() = default;

        void InitForComponent(Transform3D* pTransform);

        const glm::vec3& GetPosition() const;
        const glm::vec3& GetScale() const;
        const glm::quat& GetRotation() const;

        const glm::vec3& GetRight();
        const glm::vec3& GetUp();
        const glm::vec3& GetForward();

        const glm::mat4x4& GetLocal();
        const glm::mat4x4& GetInverseLocal();
        const glm::mat4x4& GetRotationMatrix();
        const glm::mat4x4& GetInverseRotationMatrix();

        void SetPosition(const glm::vec3& position);
        void SetPosition(float x, float y, float z);
        void SetScale(const glm::vec3& scale);
        void SetScale(float x, float y, float z);
        void SetRotation(const glm::quat& quaternion);
        void SetRotationYawPitchRoll(const glm::vec3& yaw_pitch_roll);
        void SetRotationYawPitchRoll(Radians yaw, Radians pitch, Radians roll);
        void SetRotationEuler(Radians X, Radians Y, Radians Z, EulerRotation order);

        void AddPosition(const glm::vec3& position);
        void AddPosition(float x, float y, float z);
        void AddScale(const glm::vec3& scale);
        void AddScale(float x, float y, float z);

        void Rotate(const glm::quat& quaternion);
        void RotateYawPitchRoll(const glm::vec3& yaw_pitch_roll);
        void RotateYawPitchRoll(Radians yaw, Radians pitch, Radians roll);
        void RotateEuler(Radians X, Radians Y, Radians Z, EulerRotation order);

        bool IsDirty() const { return m_IsDirty[DIRTY_FLAG_INDEX_LOCAL] | m_IsDirty[DIRTY_FLAG_INDEX_INV_LOCAL]; }

        static glm::quat YawPitchRoll_To_Quaternion(Radians yaw, Radians pitch, Radians roll);
        static glm::quat Euler_To_Quaternion(Radians X, Radians Y, Radians Z, EulerRotation order);

        friend struct Transform3D;
    };

} // GPC

#endif // GPC_TRANSFORM_H
