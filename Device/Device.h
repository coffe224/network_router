#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>

class Device
{
protected:
    sockaddr_in OSAddress;
    int OSSocket;

    std::atomic<bool> isWorking = false;

    std::thread receiverThread;
    std::thread handlerThread;

    std::string ownIP;
    std::string ownMAC;

    std::mutex recvCVMtx;
    std::condition_variable recvCV;


    // добавить структуру <Message>
    struct Message {
        std::string message;
        sockaddr_in sender;
        Message(std::string message, sockaddr_in sender) : message(message), sender(sender) {}
    };


    std::queue<Message> msgQueue;

    std::vector<std::string> parseHeader(std::string header);

    void sendMessage(std::string msg, sockaddr_in destAddress); 
    void recvMessageThread();
    void handleMessageThread();
    
    virtual void handleMsg(std::string msg, sockaddr_in senderOSAddress) = 0;
    
public:
    Device(std::string IP, std::string MAC, unsigned int OS_port);
    Device(std::string MAC, unsigned int OS_port) : Device("", MAC, OS_port) {}

    void start();

    ~Device();
};