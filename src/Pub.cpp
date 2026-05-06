#include <chrono>
#include <string>
#include <memory>

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>

using namespace std::chrono_literals;

class Publisher : public rclcpp::Node {
public:
    Publisher() 
    : Node("my_node"), 
      count_(0)
    {
        pub_ = this->create_publisher<std_msgs::msg::String>(
            "topic_name",
            10
        );

        timer_ = this->create_wall_timer(
            2ms,
            [this]() { this->timerCallback(); }
        );
    }

private:
    void timerCallback() {
        count_++;
        auto message = std_msgs::msg::String();
        message.data = "Count: " + std::to_string(count_);
        
        pub_->publish(message);
        RCLCPP_INFO(this->get_logger(), "Number %s", message.data.c_str());        
        auto get_node = this->get_node_parameters_interface();
    }

    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_;
    rclcpp::TimerBase::SharedPtr timer_;
    
    int count_; 
};

int main(int argc, char ** argv){
    rclcpp::init(argc, argv);
    auto node = std::make_shared<Publisher>();
    rclcpp::executors::MultiThreadedExecutor exec(rclcpp::ExecutorOptions(), 5);
    exec.add_node(node);
    auto ns = std::chrono::nanoseconds(1'00000000000000000000000);
    //std::cout << exec.get_number_of_threads() << std::endl;
    exec.spin();
    auto cb_groups = exec.get_all_callback_groups();
    for (auto &&i : cb_groups)
    {
        std::cout << "chmo" << std::endl;
    }
    exec.remove_node(node);

    rclcpp::shutdown();
}