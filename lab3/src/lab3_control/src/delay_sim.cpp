// Вузол моделювання перехідного процесу П-регулятора приводу суглоба
// із затримками актуації/фідбеку та лінійним предиктором компенсації.
// Моделювання виконується у віртуальному часі (швидше за реальний),
// результат -- метрики в консолі та CSV-файл для побудови графіків.

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "lab3_control/joint_sim.hpp"

using lab3_control::DelayedJointPlant;
using lab3_control::PController;

struct SimConfig
{
  double kp{5.0};
  double period{0.05};
  double a_max{0.5};
  double target{0.05};
  double actuation_delay{0.0};
  double feedback_delay{0.0};
  bool compensate{false};
  double duration{10.0};
  double sim_dt{0.001};
};

struct SimMetrics
{
  double max_x{0.0};
  double overshoot_pct{0.0};
  double settling_time{-1.0};   // -1: не встановлюється в межах 2 %
  double residual_amplitude{0.0};
  double final_x{0.0};
};

struct Sample
{
  double t, x, v, x_meas, v_meas, x_pred, u_cmd, u_applied;
};

SimMetrics simulate(const SimConfig & c, std::vector<Sample> * log = nullptr)
{
  DelayedJointPlant plant(c.a_max, c.actuation_delay, c.feedback_delay, c.sim_dt);
  PController ctrl{c.kp, c.target, c.compensate, c.actuation_delay + c.feedback_delay};

  SimMetrics m;
  double last_out_of_band = 0.0;
  double tail_min = 1e9, tail_max = -1e9;
  const int steps = static_cast<int>(std::round(c.duration / c.period));
  const int sub = std::max(1, static_cast<int>(std::round(c.period / c.sim_dt)));
  double u = 0.0;

  for (int k = 0; k <= steps; ++k) {
    const double t = k * c.period;
    // 1) Регулятор: вимірювання (із запізненням) -> команда
    const auto meas = plant.measure(t);
    u = ctrl.compute(meas.x, meas.v);
    plant.command(t, u);

    // 2) Об'єкт рухається до наступного такту, фіксуємо траєкторію
    for (int s = 1; s <= sub; ++s) {
      plant.advance_to(t + s * c.sim_dt);
      const auto & st = plant.true_state();
      m.max_x = std::max(m.max_x, st.x);
      if (std::abs(st.x - c.target) > 0.02 * std::abs(c.target)) {
        last_out_of_band = st.t;
      }
      if (st.t >= c.duration - 3.0) {
        tail_min = std::min(tail_min, st.x);
        tail_max = std::max(tail_max, st.x);
      }
      if (log && s % 5 == 0) {
        log->push_back(
          {st.t, st.x, st.v, meas.x, meas.v, ctrl.predict(meas.x, meas.v), u,
            plant.applied_command()});
      }
    }
  }
  m.final_x = plant.true_state().x;
  m.overshoot_pct = std::max(0.0, (m.max_x - c.target) / c.target * 100.0);
  m.residual_amplitude = (tail_max - tail_min) / 2.0;
  m.settling_time = (last_out_of_band < c.duration - 3.0) ? last_out_of_band : -1.0;
  return m;
}

class DelaySimNode : public rclcpp::Node
{
public:
  DelaySimNode()
  : Node("delay_sim")
  {
    cfg_.kp = declare_parameter<double>("kp", 5.0);
    cfg_.period = declare_parameter<double>("period", 0.05);
    cfg_.a_max = declare_parameter<double>("a_max", 0.5);
    cfg_.target = declare_parameter<double>("target", 0.05);
    cfg_.actuation_delay = declare_parameter<double>("actuation_delay", 0.0);
    cfg_.feedback_delay = declare_parameter<double>("feedback_delay", 0.0);
    cfg_.compensate = declare_parameter<bool>("compensate", false);
    cfg_.duration = declare_parameter<double>("duration", 10.0);
    cfg_.sim_dt = declare_parameter<double>("sim_dt", 0.001);
    output_csv_ = declare_parameter<std::string>("output_csv", "");
    sweep_ = declare_parameter<bool>("sweep", false);
  }

  void run()
  {
    if (sweep_) {
      run_sweep();
    } else {
      run_single();
    }
  }

private:
  void report(const SimConfig & c, const SimMetrics & m)
  {
    char settling[32];
    if (m.settling_time >= 0) {
      std::snprintf(settling, sizeof(settling), "%6.2f с", m.settling_time);
    } else {
      std::snprintf(settling, sizeof(settling), "    немає");
    }
    RCLCPP_INFO(
      get_logger(),
      "Kp=%5.1f T=%3.0f мс dt_loop=%3.0f мс comp=%s | x_max=%.4f перерег.=%7.1f %% "
      "t_вст=%s амплітуда_автокол.=%.5f x_кін=%.4f",
      c.kp, c.period * 1000, (c.actuation_delay + c.feedback_delay) * 1000,
      c.compensate ? "так" : "ні ", m.max_x, m.overshoot_pct, settling,
      m.residual_amplitude, m.final_x);
  }

  void run_single()
  {
    std::vector<Sample> log;
    const auto m = simulate(cfg_, &log);
    report(cfg_, m);
    if (!output_csv_.empty()) {
      std::ofstream f(output_csv_);
      f << "t,x,v,x_meas,v_meas,x_pred,u_cmd,u_applied\n";
      for (const auto & s : log) {
        f << s.t << ',' << s.x << ',' << s.v << ',' << s.x_meas << ',' << s.v_meas << ','
          << s.x_pred << ',' << s.u_cmd << ',' << s.u_applied << '\n';
      }
      RCLCPP_INFO(get_logger(), "Траєкторію записано у %s", output_csv_.c_str());
    }
  }

  void run_sweep()
  {
    const std::vector<double> kps{2.0, 5.0, 10.0};
    const std::vector<double> periods{0.05, 0.1};
    const std::vector<std::pair<double, double>> delays{{0.0, 0.0}, {0.1, 0.05}, {0.2, 0.1}};
    for (double kp : kps) {
      for (double period : periods) {
        for (const auto & d : delays) {
          for (bool comp : {false, true}) {
            if (d.first == 0.0 && comp) {continue;}  // без затримки компенсація не змінює нічого
            SimConfig c = cfg_;
            c.kp = kp;
            c.period = period;
            c.actuation_delay = d.first;
            c.feedback_delay = d.second;
            c.compensate = comp;
            c.duration = 20.0;
            report(c, simulate(c));
          }
        }
      }
    }
  }

  SimConfig cfg_;
  std::string output_csv_;
  bool sweep_{false};
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<DelaySimNode>();
  node->run();
  rclcpp::shutdown();
  return 0;
}
