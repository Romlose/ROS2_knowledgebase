#include <chrono>
#include <memory>
#include <rclcpp/rclcpp.hpp>

using namespace std::chrono_literals;

class ExperimentNode : public rclcpp::Node {
public:
    ExperimentNode() 
    : Node("experiment_node")
    ,a(0)
     {
        fast_timer_ = this->create_wall_timer(50ms, [this]() {
            auto now = std::chrono::steady_clock::now();
            auto dt = now - last_fast_time_;
            last_fast_time_ = now;
            auto ms = std::chrono::duration_cast<std::chrono::microseconds>(dt).count();
            
            if (ms > 60000) {
                RCLCPP_WARN(this->get_logger(), "HEARTBEAT LAG: %ld us (>60ms)", ms);
            }
            else{
                RCLCPP_INFO(this->get_logger(), "HEARTBEAT LAG: %ld us (<60ms)", ms);

            } 
            {
                std::lock_guard<std::mutex> lock(m);
                int cur = a.load();
                RCLCPP_INFO(this->get_logger(), "Fast timeer: Now %i, then %i", cur, cur+1);
                a += 1;
            }
            
        },
        fast_cb);

        slow_timer_ = this->create_wall_timer(1000ms, [this]() {
            RCLCPP_INFO(this->get_logger(), "Starting HEAVY math...");
            
            std::this_thread::sleep_for(200ms);
            
            RCLCPP_INFO(this->get_logger(), "HEAVY math finished.");
            {
                std::lock_guard<std::mutex> lock(m);
                int cur = a.load();
                RCLCPP_INFO(this->get_logger(), "Slow timer: Now %i, then %i", cur, cur+10);
                a+=10;
            }
            
        },
        slow_cb);

        last_fast_time_ = std::chrono::steady_clock::now();
    }
    
    rclcpp::CallbackGroup::SharedPtr fast_cb = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
    rclcpp::CallbackGroup::SharedPtr slow_cb = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

private:
    rclcpp::TimerBase::SharedPtr fast_timer_;
    rclcpp::TimerBase::SharedPtr slow_timer_;
    std::chrono::steady_clock::time_point last_fast_time_;
    std::atomic<int> a;
    std::mutex m;
};

int main(int argc, char ** argv){
    rclcpp::init(argc, argv);
    auto node = std::make_shared<ExperimentNode>();
    rclcpp::executors::MultiThreadedExecutor exec(rclcpp::ExecutorOptions(), 4); 
    exec.add_node(node); 
    exec.spin();
    exec.remove_node(node);
    rclcpp::shutdown();
}