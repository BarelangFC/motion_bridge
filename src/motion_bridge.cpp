#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "std_msgs/msg/string.hpp"
#include "bfc_msgs/msg/head_movement.hpp"
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#define PORT 3838
#define BUFLEN 4096
#define BUFSIZE 1024

#define PORTM 5000 // motion
#define PORTH 5001 // head
#define HOST "localhost"

using namespace std;

// Socket LUA head&motion
int sockmotion;
int sockhead;
struct sockaddr_in addrmotion;
struct sockaddr_in addrhead;
struct hostent *localserver;
char dataMotion[20];
char dataHead[20];
char line[10];

void initSendDataHead()
{
    sockhead = socket(AF_INET, SOCK_DGRAM, 0);
    localserver = gethostbyname(HOST);
    bzero((char *)&addrhead, sizeof(addrhead));
    addrhead.sin_family = AF_INET;
    addrhead.sin_port = htons(PORTH);
}

void initSendDataMotion()
{
    sockmotion = socket(AF_INET, SOCK_DGRAM, 0);
    localserver = gethostbyname(HOST);
    bzero((char *)&addrmotion, sizeof(addrmotion));
    addrmotion.sin_family = AF_INET;
    addrmotion.sin_port = htons(PORTM);
}

class motion_bridge : public rclcpp::Node
{
public:
    motion_bridge();
    void walkCommand(const geometry_msgs::msg::Twist::SharedPtr msg);
    void headCommand(const bfc_msgs::msg::HeadMovement::SharedPtr msg);
    void motionCommand(const std_msgs::msg::String::SharedPtr msg);
    void motion(char line[2]);
    double Walk(double x, double y, double a);
    void headMove(double pan, double tilt);

private:
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr walk_command_;
    rclcpp::Subscription<bfc_msgs::msg::HeadMovement>::SharedPtr head_command_;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr motion_command_;

    int robotNumber;
    int accelero_x, accelero_y, accelero_z, gyroscope_x, gyroscope_y, gyroscope_z, roll, pitch, yaw, strategy, kill, voltage, publish_freq_ = 0;
    double error_walk_x_, error_walk_y_, error_walk_a_;
};

motion_bridge::motion_bridge() : Node("motion_bridge",rclcpp::NodeOptions().use_intra_process_comms(true))
{
    walk_command_ = this->create_subscription<geometry_msgs::msg::Twist>(
        "walk", 5,
        std::bind(&motion_bridge::walkCommand, this, std::placeholders::_1));

    head_command_ = this->create_subscription<bfc_msgs::msg::HeadMovement>(
        "head", 5,
        std::bind(&motion_bridge::headCommand, this, std::placeholders::_1));

    motion_command_ = this->create_subscription<std_msgs::msg::String>(
        "motion", 5,
        std::bind(&motion_bridge::motionCommand, this, std::placeholders::_1));

    RCLCPP_INFO(this->get_logger(), "motion_bridge started");
}

void motion_bridge::walkCommand(const geometry_msgs::msg::Twist::SharedPtr msg)
{
    Walk(msg->linear.x, msg->linear.y, msg->linear.z);
}

void motion_bridge::headCommand(const bfc_msgs::msg::HeadMovement::SharedPtr msg)
{
    headMove(msg->pan, msg->tilt);
}

void motion_bridge::motionCommand(const std_msgs::msg::String::SharedPtr msg)
{
    motion(&msg->data[0]);
}

void motion_bridge::motion(char line[2])
{
    char awalan[50];
    strcpy(awalan, "motion");
    sprintf(dataMotion, "%s,%s", awalan, line);
    sendto(sockmotion, dataMotion, strlen(dataMotion), 0, (struct sockaddr *)&addrmotion, sizeof(addrmotion));
    // printf("data motion = %s,%s,%s\n", dataMotion, awalan, line);
}

double motion_bridge::Walk(double x, double y, double a)
{
    char line[50];
    strcpy(line, "walk");
    sprintf(dataMotion, "%s,%.2f,%.2f,%.2f", line, x, y, a);
    sendto(sockmotion, dataMotion, strlen(dataMotion), 0, (struct sockaddr *)&addrmotion, sizeof(addrmotion));
    // printf("  data walk = %s\n", dataMotion);
}

void motion_bridge::headMove(double pan, double tilt)
{
    sprintf(dataHead, "%.2f,%.2f", pan, tilt);
    sendto(sockhead, dataHead, strlen(dataHead), 0, (struct sockaddr *)&addrhead, sizeof(addrhead));
    // printf("  data head = %s\n", dataHead);
}

int main(int argc, char **argv)
{
    initSendDataHead();
    initSendDataMotion();
    rclcpp::init(argc, argv);
    auto node = std::make_shared<motion_bridge>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}