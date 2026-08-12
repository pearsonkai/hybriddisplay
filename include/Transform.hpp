#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include "Vec3.hpp"

namespace hybriddisplay::math {

class Transform {
private:
    Vec3 position;
    Vec3 rotation;
    Vec3 scale;
public:
    
    Transform(const Vec3& position = Vec3(0, 0, 0), const Vec3& rotation = Vec3(0, 0, 0), const Vec3& scale = Vec3(1, 1, 1));

    void setPosition(const Vec3& position);
    void setRotation(const Vec3& rotation);
    void setScale(const Vec3& scale);

    const Vec3 getPosition() const;
    const Vec3 getRotation() const;
    const Vec3 getScale() const;

    Vec3 applyPosition(const Vec3& point) const;
    Vec3 applyNormal(const Vec3& normal) const;
};

};

#endif