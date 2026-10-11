#include "anim/animator.hpp"

#include <algorithm>
#include <glm/gtc/quaternion.hpp>

namespace cube {
Animator::Animator(float moveDuration) : moveDuration_(moveDuration) {}

void Animator::push(const Move& move) {
    if (queue_.size() < 16) {
        queue_.push_back(move);
    }
    (void)move;
}

void Animator::update(State& state, float dt) {
    if (!animating_ && !queue_.empty()) {
        current_ = queue_.front();
        queue_.pop_front();
        elapsed_ = 0.0f;
        animating_ = true;
    }
    if (animating_) {
        elapsed_ += dt;
    }
    if (animating_ && elapsed_ >= moveDuration_) {
        cube::applyMove(state, current_);
        state.history.push_back(current_);
        animating_ = false;
    }
}

glm::mat4 Animator::rotationMatrix() const {
    if (!animating_ || moveDuration_ <= 0.0f)
        return glm::mat4(1.0f);
    float t = std::clamp(elapsed_ / moveDuration_, 0.0f, 1.0f);
    float e = Animator::ease(t);
    glm::vec3 axis(0.0f);
    axis[current_.axis] = 1.0f;
    float totalAngle = static_cast<float>(current_.dir) * glm::half_pi<float>() *
                       static_cast<float>(current_.turns);
    float currentAngle = e * totalAngle;
    return glm::mat4_cast(glm::angleAxis(currentAngle, axis));
}

constexpr float Animator::ease(float t) {
    float e = t * t * (3 - 2 * t);
    return e;
}
}  // namespace cube
