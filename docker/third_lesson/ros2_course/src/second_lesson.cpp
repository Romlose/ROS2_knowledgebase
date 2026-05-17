/*
Основная тема урока - знакомство с executor, rclcpp::NodeOptions ну и по мелочи
*/


#include <rclcpp/rclcpp.hpp>
//сегодня поработаем с publisher
#include <rclcpp/publisher.hpp>
//тип сообщений
#include <std_msgs/msg/string.hpp>

#include <chrono>
#include <memory>
#include <functional>

using namespace std::chrono_literals;

//Вообще класс rclcpp::Node инициализируется не только std::string именем, но также и 
// rclcpp::Node options + namespace. Сейчас рассмотрим только options
// NodeOptions - класс, который реализует создание скрытых параметров инициализации, например 
// аллокатор, context, use_global_arguments и др. Вообще можете посмотреть заголовочный файл, там по названиям довольно понятно
//создавать и изменять options будем в main()
class SecondNode : public rclcpp::Node{
public:
    SecondNode(const std::string name, const rclcpp::NodeOptions &options)
    : rclcpp::Node(name, options)
    , a(0) // для вывода
    {
        // Создаем паблишера ровно также через дефолтный метод и передаем туда
        // название топика куда мы публикуем и QoS - законы или правила, по которым работает топик 
        pub_ = this->create_publisher<std_msgs::msg::String>(
                            "topic_second",
                            rclcpp::QoS(10).best_effort() // настраиваем quality of service
                                                      // Храним 10 сообщений в очереди и без повторных попыток передачи
                        );
        // создаем таймер чтобы публиковать сообщения раз в интервал
        timer_ = this->create_wall_timer(1000ms, [this]{this->timerCallback();});
    }

private:
    void timerCallback(){
        auto msg = std_msgs::msg::String();
        msg.data = "loh" + std::to_string(a);
        pub_->publish(std::move(msg));
        a++;
    }

    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_;
    rclcpp::TimerBase::SharedPtr timer_;
    int a;
};

//Создаем executor. Вообще когда мы запускаем rclcpp::spin() это эквивалетно
// созданию SingleThreadExecutor. Из названия буквально следует что это однопоточный исполнитель
// что под копотом - в следующем уроке
int main(int argc, char ** argv){
    rclcpp::init(argc, argv);
    //Создаем кастомные options, но сначала получим дефолтные
    rclcpp::NodeOptions options = rclcpp::NodeOptions();
    //Изменим context запуска нашей ноды, но для начала создадим его
    auto custom_context = std::make_shared<rclcpp::Context>();
    //обязательно его проициализируем
    custom_context->init(argc, argv);
    //создаем колбек для срабатывания перед завершением ноды
    //Если зайти в исходники, там будет именно тип колбека using ShutdownCallbackType = std::function<void ()>;
    std::function<void()> callback = [](){RCLCPP_INFO(rclcpp::get_logger("pre_shutdown_logger"), "Блять, ты долбаеб, не ну реально...");};
    auto handle = custom_context->add_pre_shutdown_callback(callback);
    //тоже самое для on_shutdown в момент выключения
    auto callback_on_shutdown = [](){RCLCPP_INFO(rclcpp::get_logger("on_shutdown_logger"), "БЛЯЯЯЯТЬ ДА НАКОНЕЦ ТО");};
    auto handle_on_shutdown = custom_context->add_on_shutdown_callback(callback_on_shutdown);
    //добавляем конктекст в options
    options.context(custom_context);
    //эквивалент ::spin()
    //только перед этим создадим ноды с кастомным options
    //spin по дефолту создает SingleThreadExecutor - исполнитель с одним неблокирующим потоком
    // то есть, если несколько колбеков в одном executor то может получиться так, что поток заблокируется тяжелым колбеком
    auto node = std::make_shared<SecondNode>("second_node", std::move(options));
    //обязательно передаем такие же options в executor
    rclcpp::ExecutorOptions exec_options;
    exec_options.context = custom_context;
    rclcpp::executors::SingleThreadedExecutor exec(std::move(exec_options));
    //добавляем в executor ноды
    exec.add_node(node);
    exec.spin();
    //завершаем работу как обычно    
    rclcpp::shutdown(custom_context);
}

