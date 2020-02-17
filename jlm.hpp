#pragma once

#include <iostream>


#include <glm/glm.hpp>



static std::ostream& operator<<(std::ostream& os, const glm::vec3& v) {
  return os << '(' << (v.x) << ", " << (v.y) << ", " << (v.z) << ')';
}

static std::ostream& operator<<(std::ostream& os, const glm::vec2& v) {
  return os << '(' << (v.x) << ", " << (v.y) << ')';
}