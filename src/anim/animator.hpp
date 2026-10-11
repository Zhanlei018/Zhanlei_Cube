#pragma once

#include <deque>
#include <glm/mat4x4.hpp>
#include "cube/moves.hpp"
#include "cube/state.hpp"

namespace cube {
class Animator {
public:
    explicit Animator(float moveDuration = 0.20f);
    void push(const Move& move);
    void update(State& state, float dt);
    glm::mat4 rotationMatrix() const;
    const Move& currentMove() const { return current_; }
    bool isAnimating() const { return animating_; }
    std::size_t pendingCount() const { return queue_.size(); }
    constexpr static float ease(float t);
    void pushSequence(const std::vector<Move>& moves);

private:
    std::deque<Move> queue_;   // 等着播的动作
    Move current_;             // 正在播的动作
    float elapsed_ = 0.0f;     // 当前动作已播了多久秒
    bool animating_ = false;   // 现在有没有在播
    float moveDuration_ = 0.20f;
    void enqueue(const Move& move) { queue_.push_back(move); }
};
}  // namespace cube
