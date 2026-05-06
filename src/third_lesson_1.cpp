/*
В этом уроке мы разберем типы executors и callback groups
*/
#include <chrono>
#include <rclcpp/rclcpp.hpp>

using namespace std::chrono_literals;

// В этой ноде будет вызов одновремнно двух колбеков таймеров
// в случае singleThread у нас может происходить блокировка потока и  второй таймер будет простаивать
// это очень легко доказать с помощью замера времени chrono 
class MultiCallbacksNode : public rclcpp::Node{
public:
    MultiCallbacksNode() : rclcpp::Node("name"){
        //Можете эту строчку пока проспустить, а можете ниже за объяснением спуститься
        cb_reent_ = this->create_callback_group(rclcpp::CallbackGroupType::Reentrant);
        cb_mut_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

        first_timer_ = create_wall_timer(100ms, [this](){this->firstCallback();}, cb_reent_);
        second_timer_ = create_wall_timer(1000ms, [this](){this->secondCallback();}, cb_reent_);
        //Это нужно для замера времени дейсвтия колбека, используем именно steady_clock, чтобы система 
        // не усредняла время
        last_time_ = std::chrono::steady_clock::now();
    }
private:
    void firstCallback(){
        auto now = std::chrono::steady_clock::now();
        auto duration = now - last_time_;
        last_time_ = now;
        auto ms = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
        if(ms > 1500000) {RCLCPP_WARN(this->get_logger(), "Произошла jitter");}
    }

    void secondCallback(){
        RCLCPP_INFO(this->get_logger(), "Отработал тяжелый таймер");
        // имитация тяжелых вычислений
        std::this_thread::sleep_for(1500ms); 
    }

    rclcpp::TimerBase::SharedPtr first_timer_;
    rclcpp::TimerBase::SharedPtr second_timer_;
    std::chrono::steady_clock::time_point last_time_;
    rclcpp::CallbackGroup::SharedPtr cb_reent_;
    rclcpp::CallbackGroup::SharedPtr cb_mut_;
};

//Если мы создадим дефолтный SingleThreadExecutor у нас может получаться задержка
// и чем больше занято ресурсов процессора, тем более вероятна задержка без использования callback_groups
// Поверьте на слово, я запускал, и задержка была в половине случаев
int main(int argc, char ** argv){
    rclcpp::init(argc, argv);
    auto node = std::make_shared<MultiCallbacksNode>();
    //Подробно поговорим про MultiThreadExecutor и про executors в целом
    // когда мы создаем executor и добавляем туда ноду, executor регистрирует все колбеки и в целом создает 
    // очередь выполнения. Особенность этой очереди выполения в том, что она не гарантирует последовательность исполнений
    // а также элементы этой очереди могут обновляться в runtime.
    // Как уже может быть понятно, это все накладывает ненужную неопределенность и трату ресурсов на сканирование каждый тик цикла
    // SingleThread, MultiThread это как раз эти executors. Но multithread просто создает несколько потоков на эти задачи
    // Зато StaticSingleThreadExecutor не сканирует каждый раз элементы очереди на изменения, поэтому может быть полезен
    // Когда мы добавляем ноду в executor, мы неявно говорим, 
    //что все колбеки этой ноды имеет группу MutuallyExlusive - когда выполняется одна, вторая ждет. 
    //Конечно, мы можем поставить Reentrant и тогда они могут выполняться параллельно 
    //
    rclcpp::executors::MultiThreadedExecutor multi_exec(rclcpp::ExecutorOptions(), 4);
    multi_exec.add_node(node);
    multi_exec.spin();
    //а можно так, в течении 5000ms, причем spin вообще под копотом вызывает spin_some
    //multi_exec.spin_some(5000ms);

    //можете проверить сами для SingleThreadExec
    //rclcpp::spin(node);
    
    rclcpp::shutdown();
}