#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include "Mesh.hpp"

namespace hybriddisplay::math {

class Transform {
private:
    Vec3 position;
    Vec3 rotation;
    Vec3 scale;

    struct TrigCache {
        float sin;
        float cos;
    };
    TrigCache x, y, z;
public:
    
    Transform(const Vec3& position = Vec3(0, 0, 0), const Vec3& rotation = Vec3(0, 0, 0), const Vec3& scale = Vec3(1, 1, 1));

    void setPosition(const Vec3& position);
    void setRotation(const Vec3& rotation);
    void setScale(const Vec3& scale);
    void rotate(const Vec3& rotation);
    void rotate(const float pitch, const float yaw);
    void rotatePitch(const float degrees);
    void rotateYaw(const float degrees);
    void rotateRoll(const float degrees);

    const Vec3& getPosition() const;
    const Vec3& getRotation() const;
    const Vec3& getScale() const;

    void updateCache();
    Vec3 applyRotation(const Vec3& point) const;
    Vec3 applyInverseRotation(const Vec3& point) const;
    Vec3 applyPosition(const Vec3& point) const;
    Vec3 applyNormal(const Vec3& normal) const;

    geometry::Vertex apply(const geometry::Vertex& vertex);
};

};

#endif