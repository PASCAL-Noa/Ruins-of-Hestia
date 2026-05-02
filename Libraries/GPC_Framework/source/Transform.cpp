#include "Transform.h"

#include "../../GPC_Engine/include/TransformComponents.h"

namespace GPC {
    glm::quat Transform::YawPitchRoll_To_Quaternion(Radians yaw, Radians pitch, Radians roll) {
        glm::quat q = glm::angleAxis(yaw, glm::vec3{0, 1, 0});

        glm::vec3 right = q * glm::vec3(1, 0, 0);
        q = glm::normalize(glm::angleAxis(pitch, right) * q);

        glm::vec3 forward = q * glm::vec3(0, 0, 1);
        return glm::normalize(glm::angleAxis(roll, forward) * q);
    }

    glm::quat Transform::Euler_To_Quaternion(Radians X, Radians Y, Radians Z, EulerRotation order) {
        const Radians angles[4] = { X, Y, 0, Z };
        static constexpr glm::vec3 axis[4] = { {1, 0, 0}, {0, 1, 0}, {0, 0, 0}, {0, 0, 1} };

        uint16_t i1 = (order & 0b111) - 1;
        uint16_t i2 = ((order >> 3) & 0b111) - 1;
        uint16_t i3 = ((order >> 6) & 0b111) - 1;

        glm::quat qRotation1 = glm::angleAxis(angles[i1], axis[i1]);
        glm::quat qRotation2 = glm::angleAxis(angles[i2], axis[i2]);
        glm::quat qRotation3 = glm::angleAxis(angles[i3], axis[i3]);

        return qRotation3 * qRotation2 * qRotation1;
    }

    void Transform::ForceUpdateWorld() {
        auto T = glm::mat4x4(1.0f);
        T[3].x = m_Position.x;
        T[3].y = m_Position.y;
        T[3].z = m_Position.z;

        auto S = glm::mat4x4(1.0f);
        S[0][0] = m_Scale.x;
        S[1][1] = m_Scale.y;
        S[2][2] = m_Scale.z;

        if (m_IsDirty[DIRTY_FLAG_INDEX_ROTATION]) ForceUpdateRotationMatrix();
        m_Local = T * m_RotationMatrix * S;
        m_IsDirty[DIRTY_FLAG_INDEX_LOCAL] = false;
    }

    void Transform::ForceUpdateInverseWorld() {
        if (m_IsDirty[DIRTY_FLAG_INDEX_LOCAL]) ForceUpdateWorld();
        m_InverseLocal = glm::inverse(m_Local);
        m_IsDirty[DIRTY_FLAG_INDEX_INV_LOCAL] = false;
    }

    void Transform::ForceUpdateRotationMatrix() {
        m_RotationMatrix = glm::mat4_cast(m_Rotation);
        m_Right     = {m_RotationMatrix[0][0], m_RotationMatrix[0][1], m_RotationMatrix[0][2]};
        m_Up        = {m_RotationMatrix[1][0], m_RotationMatrix[1][1], m_RotationMatrix[1][2]};
        m_Forward   = {m_RotationMatrix[2][0], m_RotationMatrix[2][1], m_RotationMatrix[2][2]};
        m_IsDirty[DIRTY_FLAG_INDEX_ROTATION] = false;
    }

    void Transform::ForceUpdateInverseRotationMatrix() {
        if (m_IsDirty[DIRTY_FLAG_INDEX_ROTATION]) ForceUpdateWorld();
        m_InverseRotationMatrix = glm::inverse(m_RotationMatrix);
        m_IsDirty[DIRTY_FLAG_INDEX_INV_ROTATION] = false;
    }

    Transform::Transform() :
        m_RotationMatrix(1.0f),
        m_InverseRotationMatrix(1.0f),
        m_Local(1.0f),
        m_InverseLocal(1.0f),
        m_Rotation(1.0f, 0.0f, 0.0f, 0.0f),
        m_Right(1.0f, 0.0f, 0.0f),
        m_Up(0.0f, 1.0f, 0.0f),
        m_Forward(0.0f, 0.0f, 1.0f),
        m_Position(0.0f, 0.0f, 0.0f),
        m_Scale(1.0f, 1.0f, 1.0f),
        m_IsDirty(0xFFFFFFFF)
    {}

    void Transform::InitForComponent(Transform3D *pTransform) {
        mp_TransformComponent = pTransform;
    }

    const glm::vec3 & Transform::GetPosition() const {
        return m_Position;
    }

    const glm::vec3 & Transform::GetScale() const {
        return m_Scale;
    }

    const glm::quat & Transform::GetRotation() const {
        return m_Rotation;
    }

    const glm::vec3 & Transform::GetRight() {
        if (m_IsDirty[DIRTY_FLAG_INDEX_ROTATION]) ForceUpdateRotationMatrix();
        return m_Right;
    }

    const glm::vec3 & Transform::GetUp() {
        if (m_IsDirty[DIRTY_FLAG_INDEX_ROTATION]) ForceUpdateRotationMatrix();
        return m_Up;
    }

    const glm::vec3 & Transform::GetForward() {
        if (m_IsDirty[DIRTY_FLAG_INDEX_ROTATION]) ForceUpdateRotationMatrix();
        return m_Forward;
    }

    const glm::mat4x4 & Transform::GetLocal() {
        if (m_IsDirty[DIRTY_FLAG_INDEX_LOCAL]) ForceUpdateWorld();
        return m_Local;
    }

    const glm::mat4x4 & Transform::GetInverseLocal() {
        if (m_IsDirty[DIRTY_FLAG_INDEX_INV_LOCAL]) ForceUpdateInverseWorld();
        return m_InverseLocal;
    }

    const glm::mat4x4 & Transform::GetRotationMatrix() {
        if (m_IsDirty[DIRTY_FLAG_INDEX_ROTATION]) ForceUpdateRotationMatrix();
        return m_RotationMatrix;
    }

    const glm::mat4x4 & Transform::GetInverseRotationMatrix() {
        if (m_IsDirty[DIRTY_FLAG_INDEX_INV_ROTATION]) ForceUpdateInverseRotationMatrix();
        return m_InverseRotationMatrix;
    }

    void Transform::SetPosition(const glm::vec3 &position) {
        m_Position = position;
        m_IsDirty[DIRTY_FLAG_INDEX_LOCAL] = true;
        m_IsDirty[DIRTY_FLAG_INDEX_INV_LOCAL] = true;
        mp_TransformComponent->SetDirty();
    }

    void Transform::SetPosition(float x, float y, float z) {
        m_Position = {x, y, z};
        m_IsDirty[DIRTY_FLAG_INDEX_LOCAL] = true;
        m_IsDirty[DIRTY_FLAG_INDEX_INV_LOCAL] = true;
        mp_TransformComponent->SetDirty();
    }

    void Transform::SetScale(const glm::vec3 &scale) {
        m_Scale = scale;
        m_IsDirty[DIRTY_FLAG_INDEX_LOCAL] = true;
        m_IsDirty[DIRTY_FLAG_INDEX_INV_LOCAL] = true;
        mp_TransformComponent->SetDirty();
    }

    void Transform::SetScale(float x, float y, float z) {
        m_Scale = {x, y, z};
        m_IsDirty[DIRTY_FLAG_INDEX_LOCAL] = true;
        m_IsDirty[DIRTY_FLAG_INDEX_INV_LOCAL] = true;
        mp_TransformComponent->SetDirty();
    }

    void Transform::SetRotation(const glm::quat &quaternion) {
        m_Rotation = quaternion;
        m_IsDirty = 0xFFFFFFFF;
        mp_TransformComponent->SetDirty();
    }

    void Transform::SetRotationYawPitchRoll(const glm::vec3 &yaw_pitch_roll) {
        m_Rotation = YawPitchRoll_To_Quaternion(yaw_pitch_roll.x, yaw_pitch_roll.y, yaw_pitch_roll.z);
        m_IsDirty = 0xFFFFFFFF;
        mp_TransformComponent->SetDirty();
    }

    void Transform::SetRotationYawPitchRoll(Radians yaw, Radians pitch, Radians roll) {
        m_Rotation = YawPitchRoll_To_Quaternion(yaw, pitch, roll);
        m_IsDirty = 0xFFFFFFFF;
        mp_TransformComponent->SetDirty();
    }

    void Transform::SetRotationEuler(Radians X, Radians Y, Radians Z, EulerRotation order) {
        m_Rotation = Euler_To_Quaternion(X, Y, Z, order);
        m_IsDirty = 0xFFFFFFFF;
        mp_TransformComponent->SetDirty();
    }

    void Transform::AddPosition(const glm::vec3 &position) {
        m_Position += position;
        m_IsDirty[DIRTY_FLAG_INDEX_LOCAL] = true;
        m_IsDirty[DIRTY_FLAG_INDEX_INV_LOCAL] = true;
        mp_TransformComponent->SetDirty();
    }

    void Transform::AddPosition(float x, float y, float z) {
        m_Position.x += x;
        m_Position.y += y;
        m_Position.z += z;
        m_IsDirty[DIRTY_FLAG_INDEX_LOCAL] = true;
        m_IsDirty[DIRTY_FLAG_INDEX_INV_LOCAL] = true;
        mp_TransformComponent->SetDirty();
    }

    void Transform::AddScale(const glm::vec3 &scale) {
        m_Scale += scale;
        m_IsDirty[DIRTY_FLAG_INDEX_LOCAL] = true;
        m_IsDirty[DIRTY_FLAG_INDEX_INV_LOCAL] = true;
        mp_TransformComponent->SetDirty();
    }

    void Transform::AddScale(float x, float y, float z) {
        m_Scale.x += x;
        m_Scale.y += y;
        m_Scale.z += z;
        m_IsDirty[DIRTY_FLAG_INDEX_LOCAL] = true;
        m_IsDirty[DIRTY_FLAG_INDEX_INV_LOCAL] = true;
        mp_TransformComponent->SetDirty();
    }

    void Transform::Rotate(const glm::quat &quaternion) {
        m_Rotation = glm::normalize(quaternion * m_Rotation);
        m_IsDirty = 0xFFFFFFFF;
        mp_TransformComponent->SetDirty();
    }

    void Transform::RotateYawPitchRoll(const glm::vec3 &yaw_pitch_roll) {
        m_Rotation = glm::normalize(YawPitchRoll_To_Quaternion(yaw_pitch_roll.x, yaw_pitch_roll.y, yaw_pitch_roll.z) * m_Rotation);
        m_IsDirty = 0xFFFFFFFF;
        mp_TransformComponent->SetDirty();
    }

    void Transform::RotateYawPitchRoll(Radians yaw, Radians pitch, Radians roll) {
         m_Rotation = glm::normalize(YawPitchRoll_To_Quaternion(yaw, pitch, roll) * m_Rotation);
        m_IsDirty = 0xFFFFFFFF;
        mp_TransformComponent->SetDirty();
    }

    void Transform::RotateEuler(Radians X, Radians Y, Radians Z, EulerRotation order) {
        m_Rotation = glm::normalize(Euler_To_Quaternion(X, Y, Z, order) * m_Rotation);
        m_IsDirty = 0xFFFFFFFF;
        mp_TransformComponent->SetDirty();
    }

} // GPC