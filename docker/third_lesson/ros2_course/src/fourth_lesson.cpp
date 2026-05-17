/*
Первая часть урока, где рассматривается подробно wait_set способ ROS2 хранить в очереди исполняемые элементы
Продолжение урока в файле WaitSet там пример работы совсем с нижним уровнем rclcpp::Waitable
*/

#include <rclcpp/rclcpp.hpp>
#include <rclcpp/wait_set.hpp>
#include <std_msgs/msg/bool.hpp>


//Как было обговорено в прошлых уроках про экзекьюторы, они не обеспечивают детерменированности в исполнении колбеков
// Хоть executor и использует под капотом wait_set, само формирование очереди не стандартизировано
// Wait Set паттерн - Reactor, ведь пользователь должен сам реализовать действия с готовыми данными а
//wait set просто уведомляет нас о них
// wait_set это в свою очередь контейнер который содержит в себе Waitable объекты (WaitSet.cpp пример). 
int main(int argc, char ** argv){
    rclcpp::init(argc, argv);
    //Создадим ноду и 3 колбека A, B, C в ней
    auto node = std::make_shared<rclcpp::Node>("wait_set_node");
    auto A = node->create_subscription<std_msgs::msg::Bool>("A", 10, [](std_msgs::msg::Bool::SharedPtr){});
    auto B = node->create_subscription<std_msgs::msg::Bool>("B", 10, [](std_msgs::msg::Bool::SharedPtr){});
    auto C = node->create_subscription<std_msgs::msg::Bool>("C", 10, [](std_msgs::msg::Bool::SharedPtr){});
    //Создадим очередь wait_set
    auto wait_set = rclcpp::WaitSet();
    //добавим колбеки в той очередности, в какой хотим их исполнения
    wait_set.add_subscription(A);
    wait_set.add_subscription(B);
    wait_set.add_subscription(C);

    // Запускаем бесконечный цикл
    // На самом деле при запуске rclcpp::spin() под капотом происходит то же самое, просто проверяются еще условия
    // ниже из исходников
      // - while not shutdown, and spinning (not canceled), and not max duration reached...
    // - try to get an executable item to execute, and execute it if available
    // - otherwise, reset the wait result, and ...
    // - if there was no work available just after waiting, break the loop unconditionally
    //   - this is appropriate for both spin_some and spin_all which use this function
    // - else if exhaustive = true, then wait for work again
    //   - this is only used for spin_all and not spin_some
    // - else break
    //   - this only occurs with spin_some
    while(rclcpp::ok()){
        //ждем пока фиктивные данные придут 
        auto wait_result = wait_set.wait(std::chrono::milliseconds(100));

        if (wait_result.kind() == rclcpp::WaitResultKind::Ready) {
            
            // САМОЕ ГЛАВНОЕ - Ручное исполнение в конкретном порядке A -> B -> C
            
            //A
            //метод take передает само сообщение msg_A и еще его метаданные info_A 
            // метку времени отправителя, статус сообщения (внутренний процесс или с помощью DDS) статус в очереди и др.
            std_msgs::msg::Bool msg_A;
            rclcpp::MessageInfo info_A;
            if (A->take(msg_A, info_A)) {
                RCLCPP_INFO(node->get_logger(), "Processing A");
            }

            // B
            std_msgs::msg::Bool msg_B;
            rclcpp::MessageInfo info_B;
            if (B->take(msg_B, info_B)) {
                RCLCPP_INFO(node->get_logger(), "Processing B");
            }

            // C
            std_msgs::msg::Bool msg_C;
            rclcpp::MessageInfo info_C;
            if (C->take(msg_C, info_C)) {
                RCLCPP_INFO(node->get_logger(), "Processing C");
            }
        }
    }

    rclcpp::shutdown();
    return 0;

}