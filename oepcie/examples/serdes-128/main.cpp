#include <csignal>
#include <iostream>

#include "oepcie/oepcie.hpp"

// Quit flag
volatile sig_atomic_t quit = 0;

// Signal handler for ctrl-c
static void int_handler(int)
{
    quit = 1;
}

// Device type alias
using device_t = oe::pcie_device<128, 1000>;

int main()
{
    try {
        // Install Ctrl-c signal handler
        std::signal(SIGINT, int_handler);

        // Make a headstage control instance
        device_t headstage;

        // TODO: Actual headstage configuration

        // Start acqusition
        auto rc = headstage.configure(oe::start, 1);
        if (!rc) {
            std::cerr << "Start failed." << std::endl;
            return -1;
        }

        while (!quit) {

            // Read block
            device_t::raw_buffer_t buffer;
            headstage.read_block(buffer);

            std::cout << buffer[0][0] << "\n\n";
        }

    } catch (const std::runtime_error &ex) {
        std::cerr << ex.what() << std::endl;
        return -1;
    }

    return 1;
}
