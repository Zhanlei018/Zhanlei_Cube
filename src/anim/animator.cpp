#include "anim/animator.hpp"

#include <algorithm>
#include <glm/gtc/quaternion.hpp>

namespace cube {
Animator::Animator(float moveDuration) : moveDuration_(moveDuration) {}

void Animator::beginMove(const Move& move, float startProgress) {
    current_ = move;
    const float p = std::clamp(startProgress, 0.0f, 1.0f);
    startElapsed_ = p * moveDuration_;
    elapsed_ = startElapsed_;
    animating_ = true;
}

void Animator::endMove(State& state) {
    cube::applyMove(state, current_);
    if (recording_) {
        state.history.push_back(current_);
    }
    animating_ = false;
}

void Animator::clearHistory(State& state) {
    state.history.clear();
}

void Animator::push(const Move& move, float startProgress) {
    if (queue_.size() < 16) {
        enqueue(move);
        if (!animating_ && queue_.size() == 1) {
            beginMove(move, startProgress);
            queue_.pop_front();
        }
    }
}

void Animator::update(State& state, float dt) {
    if (!animating_ && !queue_.empty()) {
        current_ = queue_.front();
        queue_.pop_front();
        startElapsed_ = 0.0f;
        elapsed_ = 0.0f;
        animating_ = true;
    }
    if (animating_) {
        elapsed_ += dt;
    }
    if (animating_ && elapsed_ >= moveDuration_) {
        endMove(state);
    }
}

glm::mat4 Animator::rotationMatrix() const {
    if (!animating_ || moveDuration_ <= 0.0f)
        return glm::mat4(1.0f);
    const float t = std::clamp(elapsed_ / moveDuration_, 0.0f, 1.0f);
    const float e = Animator::ease(t);
    glm::vec3 axis(0.0f);
    axis[current_.axis] = 1.0f;
    const float totalAngle = static_cast<float>(current_.dir) * glm::half_pi<float>() *
                             static_cast<float>(current_.turns);
    return glm::mat4_cast(glm::angleAxis(e * totalAngle, axis));
}

constexpr float Animator::ease(float t) {
    return t * t * (3.0f - 2.0f * t);
}

void Animator::pushSequence(const std::vector<Move>& moves) {
    for (const Move& m : moves) enqueue(m);
}
}  // namespace cube
