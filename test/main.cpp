#include "receiver.hpp"
#include "general_receiver.hpp"

#include <thread>
#include <chrono>


int main(int argc, char* argv[])
{
    gst_init(&argc, &argv);
    GeneralReceiver gr;

    gr.start();

    std::cout << "heelo" << std::endl;

    while (true)
    {
        gr.start();
        std::this_thread::sleep_for(std::chrono::seconds(5));
        std::cout << "sleep 1" << std::endl;
    }

}