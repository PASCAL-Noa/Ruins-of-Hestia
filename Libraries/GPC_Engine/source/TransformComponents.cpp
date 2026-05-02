#include "TransformComponents.h"

namespace GPC {
    Transform3D::~Transform3D() {
        if (s_IsShuttingDown) {
            return;
        }

        if (mp_Parent != nullptr) {
            for (auto it =  mp_Parent->mp_Childs.begin(); it !=  mp_Parent->mp_Childs.end(); ++it) {
                if (*it == this) {
                    mp_Parent->mp_Childs.erase(it);
                    break;
                }
            }
        }

        for (auto pChild : mp_Childs) {
            pChild->SetParent(nullptr);
        }
    }

    const glm::mat4x4& Transform3D::GetWorldMatrix() {
        UpdateWorldMatrix();
        return m_World;
    }

    glm::mat4x4 Transform3D::GetWorldRotation() {
        glm::mat4x4 m{1.0f};
        for (uint32_t i = 0; i < 3; ++i) {
            glm::vec3 v = glm::normalize(glm::vec3(GetWorldMatrix()[i]));
            m[i] = glm::vec4(v.x, v.y, v.z, 0.0f);
        }
        return m;
    }

    Transform3D * Transform3D::GetParent() {
        return mp_Parent;
    }

    Transform3D * Transform3D::GetChild(uint32_t index) {
        return mp_Childs[index];
    }

    const Transform3D * Transform3D::GetParent() const {
        return mp_Parent;
    }

    const Transform3D * Transform3D::GetChild(uint32_t index) const {
        return mp_Childs[index];
    }

    uint32_t Transform3D::GetChildCount() const {
        return mp_Childs.size();
    }

    void Transform3D::SetParent(Transform3D *pParent) {
        if (pParent == this) {
            GPC_WARNING << "You are trying to create a loop in transform parent" << ENDL;
            return;
        }
        if (mp_Parent != nullptr) {
            RemoveParent();
        }
        mp_Parent = pParent;
        if (mp_Parent != nullptr) {
            mp_Parent->mp_Childs.push_back(this);
        }
        UpdateLocalWithWorldParentAddition();
    }

    void Transform3D::AddChild(Transform3D *pChild) {
        if (pChild == this) {
            GPC_WARNING << "You are trying to create a loop in transform parent" << ENDL;
            return;
        }
        for (auto& child : mp_Childs) {
            if (child == pChild) return;
        }

        if (pChild->mp_Parent != nullptr) {
            pChild->RemoveParent();
        }
        mp_Childs.push_back(pChild);
        pChild->mp_Parent = this;
        pChild->UpdateLocalWithWorldParentAddition();
    }

    void Transform3D::RemoveParent() {
        if (mp_Parent != nullptr) {
            for (auto it = mp_Parent->mp_Childs.begin(); it !=  mp_Parent->mp_Childs.end(); ++it) {
                if (*it == this) {
                    mp_Parent->mp_Childs.erase(it);
                    break;
                }
            }
        }
        mp_Parent = nullptr;
    }

    void Transform3D::RemoveChild(uint32_t index) {
        if (index >= GetChildCount()) return;
        auto it = mp_Childs.begin();
        std::advance(it, index);
        mp_Childs[index]->UpdateLocalWithWorldParentRemoval();
        mp_Childs[index]->mp_Parent = nullptr;
        mp_Childs.erase(it);
    }

    void Transform3D::UpdateLocalWithWorldParentRemoval() {
        if (mp_Parent == nullptr) return;
        auto& world = mp_Parent->GetWorldMatrix();
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(world, scale, rotation, translation, skew, perspective);
        translation += (rotation * LocalTransform.GetPosition()) * scale;;
        rotation = rotation * LocalTransform.GetRotation();
        scale *= LocalTransform.GetScale();
        LocalTransform.SetPosition(translation);
        LocalTransform.SetRotation(rotation);
        LocalTransform.SetScale(scale);
    }

    void Transform3D::UpdateLocalWithWorldParentAddition() {
        if (mp_Parent == nullptr) return;
        auto& world = mp_Parent->GetWorldMatrix();
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew; glm::vec4 perspective;
        glm::decompose(world, scale, rotation, translation, skew, perspective);
        rotation = glm::inverse(rotation);
        translation = (rotation * (LocalTransform.GetPosition() - translation)) / scale;
        rotation = LocalTransform.GetRotation() * rotation;
        scale = LocalTransform.GetScale() / scale;
        LocalTransform.SetPosition(translation);
        LocalTransform.SetRotation(rotation);
        LocalTransform.SetScale(scale);
    }

    glm::vec3 Transform3D::GetWorldPosition() {
        UpdateWorldMatrix();
        return glm::vec3(GetWorldMatrix()[3]);
    }

    void Transform3D::SetDirty() {
        if (m_IsDirty) return;
        m_IsDirty = true;
        for (auto pChild : mp_Childs) {
            pChild->SetDirty();
        }
    }

    void Transform3D::UpdateWorldMatrix() {
        if (m_IsDirty == false) return;
        if (mp_Parent == this) {
            GPC_WARNING << "WTF pq je suis mon propre parent !" << ENDL;
            mp_Parent = nullptr;
        }

        if (mp_Parent == nullptr) {
            m_World = LocalTransform.GetLocal();
        }
        else {
            mp_Parent->UpdateWorldMatrix();
            m_World = mp_Parent->GetWorldMatrix() * LocalTransform.GetLocal();
        }
        m_IsDirty = false;
    }

} // GPC