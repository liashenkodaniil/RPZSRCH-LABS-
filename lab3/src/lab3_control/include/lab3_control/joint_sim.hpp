// Модель приводу суглоба з обмеженням прискорення та затримками контуру
// (спільна для офлайн-симулятора delay_sim і апаратного плагіна ros2_control)
#ifndef LAB3_CONTROL__JOINT_SIM_HPP_
#define LAB3_CONTROL__JOINT_SIM_HPP_

#include <algorithm>
#include <cmath>
#include <deque>

namespace lab3_control
{

struct JointState
{
  double t{0.0};
  double x{0.0};  // положення, рад
  double v{0.0};  // швидкість, рад/с
};

// Об'єкт керування: сервопривід відпрацьовує задану швидкість u,
// але не швидше, ніж дозволяє обмеження прискорення a_max.
//  - команда застосовується приводом через actuation_delay після видачі;
//  - вимірювання (енкодер) надходить у регулятор із запізненням feedback_delay.
class DelayedJointPlant
{
public:
  DelayedJointPlant(
    double a_max = 0.5, double actuation_delay = 0.0, double feedback_delay = 0.0,
    double dt = 0.001)
  : a_max_(a_max), actuation_delay_(actuation_delay), feedback_delay_(feedback_delay), dt_(dt)
  {
    reset();
  }

  void reset(double x0 = 0.0, double v0 = 0.0)
  {
    state_ = JointState{0.0, x0, v0};
    u_applied_ = 0.0;
    commands_.clear();
    history_.clear();
    history_.push_back(state_);
  }

  // Регулятор видав команду u у момент t: привід отримає її в t + actuation_delay
  void command(double t, double u)
  {
    commands_.push_back({t + actuation_delay_, u});
  }

  // Інтегрування руху до моменту t_end з дрібним кроком dt
  void advance_to(double t_end)
  {
    constexpr double eps = 1e-9;
    while (state_.t < t_end - eps) {
      while (!commands_.empty() && commands_.front().first <= state_.t + eps) {
        u_applied_ = commands_.front().second;
        commands_.pop_front();
      }
      const double dv = std::clamp(u_applied_ - state_.v, -a_max_ * dt_, a_max_ * dt_);
      state_.v += dv;
      state_.x += state_.v * dt_;
      state_.t += dt_;
      history_.push_back(state_);
    }
    // Історію тримаємо лише на глибину затримки фідбеку (+ запас)
    while (history_.size() > 2 && history_[1].t < state_.t - feedback_delay_ - 1.0) {
      history_.pop_front();
    }
  }

  // Те, що "бачить" регулятор у момент t: стан приводу в момент t - feedback_delay
  JointState measure(double t) const
  {
    constexpr double eps = 1e-9;
    const double t_meas = t - feedback_delay_;
    JointState result = history_.front();
    for (const auto & s : history_) {
      if (s.t <= t_meas + eps) {
        result = s;
      } else {
        break;
      }
    }
    return result;
  }

  const JointState & true_state() const {return state_;}
  double applied_command() const {return u_applied_;}

private:
  double a_max_;
  double actuation_delay_;
  double feedback_delay_;
  double dt_;
  JointState state_;
  double u_applied_{0.0};
  std::deque<std::pair<double, double>> commands_;  // (час застосування, u)
  std::deque<JointState> history_;
};

// П-регулятор положення з необов'язковим лінійним предиктором затримки:
//   e      = x_target - x_meas
//   x_pred = x_meas + v_meas * (dt_actuation + dt_feedback)
//   e_comp = x_target - x_pred
//   U      = Kp * e   (або Kp * e_comp)
struct PController
{
  double kp{5.0};
  double target{0.05};
  bool compensate{false};
  double loop_delay{0.0};  // dt_actuation + dt_feedback, с

  double predict(double x_meas, double v_meas) const
  {
    return compensate ? x_meas + v_meas * loop_delay : x_meas;
  }

  double compute(double x_meas, double v_meas) const
  {
    return kp * (target - predict(x_meas, v_meas));
  }
};

}  // namespace lab3_control

#endif  // LAB3_CONTROL__JOINT_SIM_HPP_
