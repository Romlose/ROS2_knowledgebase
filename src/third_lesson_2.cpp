/*
Продолжение урока. Здесь разберем стандарные способы как получить UB, runtime_error
*/
#include <rclcpp/rclcpp.hpp>
#include <chrono>

using namespace std::chrono_literals;

// Это просто UB
//По названию думаю понятно, что тут происходит, единственная проблема
// почему тут будет race_condition - неправильная callback группа, так как Reentrant разрешает
// колбекам работать паралелльно. Решение сделать cb_group (callback group) MutuallyExlusive 
class RaceConditionNode : public rclcpp::Node{
public:
    RaceConditionNode() : rclcpp::Node("race_condition_node"), counter(0){
        cb_reent = this->create_callback_group(rclcpp::CallbackGroupType::Reentrant);
        first_timer_ = this->create_wall_timer(2ms, [this]{this->workCallback();}, cb_reent);
        second_timer_= this->create_wall_timer(2ms, [this]{this->workCallback();}, cb_reent);
    }
private:
    void workCallback(){
        for(int i = 0; i < 10000; ++i){counter++;}
        RCLCPP_INFO(this->get_logger(), "A is %i", counter);
    }

    rclcpp::TimerBase::SharedPtr first_timer_;
    rclcpp::TimerBase::SharedPtr second_timer_;
    rclcpp::CallbackGroup::SharedPtr cb_reent;
    int counter;
};


//тут чтобы не засорять main собраны все rte
void runtime_errors(int a, char ** b){
    //1.
    //нельзя до инициализации или после выключания что то создавать, удалять и так далее
    // все наши хендлеры ресурсов будут указазывать на перезаписанную память уже 
    rclcpp::init(a, b);
    rclcpp::shutdown();
    auto node = std::make_shared<RaceConditionNode>();
    //2.
    //нельзя добавлять в exec одну и ту же ноду
    rclcpp::executors::SingleThreadedExecutor exec;
    exec.add_node(node);
    exec.add_node(node);
    //3.
    // нельзя вызывать ::spin() в ::spin()
    // условно в ноде которая уже крутится экзекьютором мы вызываем блокирующий ::spin()
    // и это не сломает программу, но она зависнет просто
}

int main(int argc, char ** argv){
    rclcpp::init(argc, argv);
    auto node = std::make_shared<RaceConditionNode>();
    rclcpp::executors::MultiThreadedExecutor exec(rclcpp::ExecutorOptions(), 4);
    exec.add_node(node->get_node_base_interface());
    exec.spin();
    rclcpp::shutdown();
}