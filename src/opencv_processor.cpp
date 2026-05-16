/*
Основная тема урока - работа с OpenCV и cv_bridge в ROS2
Разберём как нормально работать с картинками в ROS2.
*/

#include <rclcpp/rclcpp.hpp>
// Сообщение с картинкой
#include <sensor_msgs/msg/image.hpp>

// cv_bridge - библиотека для работы между ROS-картинкой и OpenCV
#include <cv_bridge/cv_bridge.hpp>
// image_transport умеет публиковать сжатые изображения
#include <image_transport/image_transport.hpp>

// Сам OpenCV
#include <opencv2/opencv.hpp>

#include <memory>
#include <functional>

using namespace std::chrono_literals;

// Класс ноды
class OpenCVProcessor : public rclcpp::Node {
public:
    OpenCVProcessor() 
        : Node("opencv_processor")
    {
        // QoS очень важен для камер. 
        // Чаще всего камеры шлют best_effort, потому что им важна скорость, а не то, что все кадры дойдут
        auto qos = rclcpp::QoS(10).best_effort();

        // Подписываемся на сырое изображение с камеры
        image_sub_ = this->create_subscription<sensor_msgs::msg::Image>(
            "/camera/image_raw", 
            qos,
            std::bind(&OpenCVProcessor::imageCallback, this, std::placeholders::_1)
        );

        // Публикуем через image_transport - это правильный способ в ROS2
        // хотя Обычный publisher тоже можно
        image_pub_ = image_transport::create_publisher(this, "/processed_image");

        RCLCPP_INFO(this->get_logger(), "OpenCV Processor запущен. Ждём картинки на /camera/image_raw");
    }

private:
    void imageCallback(const sensor_msgs::msg::Image::SharedPtr msg)
    {
        try {
            // toCvCopy копирует данные, toCvShare пытается поделиться памятью
            cv_bridge::CvImagePtr cv_ptr = 
                cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);

            cv::Mat gray, edges;

            // Делаем чёрно-белое изображение
            cv::cvtColor(cv_ptr->image, gray, cv::COLOR_BGR2GRAY);
            
            // Детектор краёв Canny
            cv::Canny(gray, edges, 50, 150);

            // OpenCV Mat ROS Image
            auto processed_msg = cv_bridge::CvImage(
                msg->header,                    // берём заголовок от оригинала (timestamp и frame_id)
                sensor_msgs::image_encodings::MONO8, 
                edges
            ).toImageMsg();

            // Публикуем обработанную картинку
            image_pub_.publish(processed_msg);

            RCLCPP_DEBUG(this->get_logger(), "Обработали картинку размером %dx%d", 
                        edges.cols, edges.rows);

        } catch (cv_bridge::Exception& e) {
            RCLCPP_ERROR(this->get_logger(), "cv_bridge упал: %s", e.what());
        } catch (std::exception& e) {
            RCLCPP_ERROR(this->get_logger(), "Что-то другое: %s", e.what());
        }
    }

    // Члены класса
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr image_sub_;
    image_transport::Publisher image_pub_;
};


int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    
    auto node = std::make_shared<OpenCVProcessor>();
    
    RCLCPP_INFO(node->get_logger(), "Крутимся в spin...");
    rclcpp::spin(node);
    
    rclcpp::shutdown();
    return 0;
}