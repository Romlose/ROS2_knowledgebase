#include <termios.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>

#include <rclcpp/rclcpp.hpp>

class KeyboardListener : public rclcpp::Waitable {
public:
    KeyboardListener() {
        auto context = rclcpp::contexts::get_global_default_context();
        gc_ = rcl_get_zero_initialized_guard_condition();
        rcl_ret_t ret = rcl_guard_condition_init(
            &gc_,
            context->get_rcl_context().get(),
            rcl_guard_condition_get_default_options()
        );

        if (ret != RCL_RET_OK) {
             rclcpp::exceptions::throw_from_rcl_error(ret, "Failed to init guard condition");
        }

        tcgetattr(STDIN_FILENO, &old_settings_);
        new_settings_ = old_settings_;
        new_settings_.c_lflag &= ~(ICANON | ECHO);
        new_settings_.c_cc[VMIN] = 0;
        new_settings_.c_cc[VTIME] = 1;
        tcsetattr(STDIN_FILENO, TCSANOW, &new_settings_);

        running_ = true;
        input_thread_ = std::thread(&KeyboardListener::keyboard_thread_func, this);
    }

    ~KeyboardListener() override {
        running_ = false;
        if (input_thread_.joinable()) {
            input_thread_.join();
        }
        tcsetattr(STDIN_FILENO, TCSANOW, &old_settings_);
        rcl_guard_condition_fini(&gc_);
    }

    size_t get_number_of_ready_guard_conditions() override { return 1; }
    size_t get_number_of_ready_subscriptions() override { return 0; }
    size_t get_number_of_ready_timers() override { return 0; }
    size_t get_number_of_ready_clients() override { return 0; }
    size_t get_number_of_ready_services() override { return 0; }
    size_t get_number_of_ready_events() override { return 0; }

    void add_to_wait_set(rcl_wait_set_t * wait_set) override {
        std::lock_guard<std::mutex> lock(m_);
        rcl_wait_set_add_guard_condition(wait_set, &gc_, nullptr);
    }

    bool is_ready(rcl_wait_set_t * wait_set) override {
        (void)wait_set;
        std::lock_guard<std::mutex> lock(m_);
        return has_data_;
    }

    void execute(std::shared_ptr<void> & data) override {
        (void)data;
        char key_val;
        {
            std::lock_guard<std::mutex> lock(m_);
            key_val = last_char_;
            has_data_ = false;
        }
        RCLCPP_INFO(rclcpp::get_logger("Keyboard"), "Key pressed: '%c' (ASCII %d)", key_val, (int)key_val);
    }

    void set_on_ready_callback(std::function<void(size_t, int)> callback) override {
        std::lock_guard<std::mutex> lock(m_);
        on_ready_callback_ = callback;
    }

    void clear_on_ready_callback() override {
        std::lock_guard<std::mutex> lock(m_);
        on_ready_callback_ = nullptr;
    }
    
    std::shared_ptr<void> take_data() override {
        std::lock_guard<std::mutex> lock(m_);
        has_data_ = false;
        return std::make_shared<char>(last_char_);
    }
private:
    void keyboard_thread_func() {
        char ch;
        while (running_) {
            int bytes_read = read(STDIN_FILENO, &ch, 1);
            if (bytes_read > 0) {
                std::lock_guard<std::mutex> lock(m_);
                last_char_ = ch;
                has_data_ = true;
                rcl_trigger_guard_condition(&gc_);
                if (on_ready_callback_) {
                    on_ready_callback_(0, 0);
                }
            }
        }
    }

    rcl_guard_condition_t gc_;
    std::thread input_thread_;
    std::mutex m_;
    std::atomic<bool> running_;
    std::function<void(size_t, int)> on_ready_callback_;
    char last_char_ = 0;
    bool has_data_ = false;
    struct termios old_settings_, new_settings_;
};

class KeyboardNode : public rclcpp::Node {
public:
    KeyboardNode() : Node("keyboard_node") {
        keyboard_waitable = std::make_shared<KeyboardListener>();
        this->get_node_waitables_interface()->add_waitable(keyboard_waitable, nullptr);
    }
private:
    std::shared_ptr<KeyboardListener> keyboard_waitable;
};

int main(int argc, char ** argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<KeyboardNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
