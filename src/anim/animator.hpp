#pragma once

#include <deque>
#include <glm/mat4x4.hpp>
#include "cube/moves.hpp"
#include "cube/state.hpp"

namespace cube {
class Animator {
public:
    explicit Animator(float moveDuration = 0.20f);

    void push(const Move& move, float startProgress = 0.0f);

    void beginMove(const Move& move, float startProgress = 0.0f);
    void endMove(State& state);
    void clearHistory(State& state);
    void applySilent(State& state, const Move& move);

    void update(State& state, float dt);
    glm::mat4 rotationMatrix() const;
    const Move& currentMove() const { return current_; }
    bool isAnimating() const { return animating_; }
    std::size_t pendingCount() const { return queue_.size(); }
    constexpr static float ease(float t);
    void pushSequence(const std::vector<Move>& moves);

private:
    std::deque<Move> queue_;
    Move current_;
    float elapsed_ = 0.0f;
    float startElapsed_ = 0.0f;
    bool animating_ = false;
    float moveDuration_ = 0.20f;
    void enqueue(const Move& move) { queue_.push_back(move); }
};
}  // namespace cube

