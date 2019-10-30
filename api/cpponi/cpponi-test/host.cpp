#include <iostream>
#include <string>
#include <errno.h>
#include <vector>
#include <thread>
#include <memory>
#include <sstream>
#include <string>

#include <oni.hpp>
#include <onidevices.hpp>

// Dump raw device streams to files?
//#define DUMPFILES

#ifdef DUMPFILES
std::vector<FILE *> dump_files;
#endif

// Windows- and UNIX-specific includes etc
#ifdef _WIN32
#include <windows.h>
#pragma comment(lib, "liboni")
#include <stdio.h>
#include <stdlib.h>

#else
#include <unistd.h>
#include <pthread.h>
#endif

volatile int quit = 0;
volatile int display = 0;
volatile int display_clock = 0;
int running = 1;

int parse_reg_cmd(const char *cmd, long *values)
{
    char *end;
    int k = 0;
    for (long i = strtol(cmd, &end, 10);
         cmd != end;
         i = strtol(cmd, &end, 10))
    {

        cmd = end;
        if (errno == ERANGE){ return -1; }

        values[k++] = i;
        if (k == 3)
            break;
    }

    if (k < 3)
        return -1;

    return 0;
}

void data_loop(std::shared_ptr<oni::context_t> ctx)
{
    unsigned long counter = 0;
    int rc = 0;
    const auto dev_map = ctx->device_map();

    try {

        while (rc == 0 && !quit) {

            auto frame = ctx->read_frame();

            if (display_clock && counter % 100 == 0)
                std::cout << "\tSample: " << frame.clock() << "\n\n";

            auto device_indices = frame.device_indices();
            for (const auto i : device_indices) {

                // TODO: Eventually can be used to construct std::span for easy
                // consumption
                auto begin = frame.begin<uint16_t>(i);
                auto end = frame.end<uint16_t>(i);

#ifdef DUMPFILES
                fwrite(
                    begin, sizeof(uint16_t), end - begin, dump_files[i]);
#endif

            if (display && counter % 100 == 0) {
                std::cout << "\tDev: " << i << " ("
                          << oni::device_str(dev_map[i].id) << ")\n";

                std::cout << "\tData: [";
                for (auto d = begin; d != end; d++)
                    std::cout << *d << " ";
                std::cout << "]\n";
                }
            }

            counter++;
        }

    } catch (const oni::error_t &ex) {
        quit = true;
        std::cerr << ex.what() << "\n";
    }
}

int main(int argc, char *argv[])
{
    const char *config_path = ONI_DEFAULTCONFIGPATH;
    const char *sig_path = ONI_DEFAULTSIGNALPATH;
    const char *read_path = ONI_DEFAULTREADPATH;
    const char *write_path = ONI_DEFAULTWRITEPATH;

    std::cout << logo_med;

    if (argc != 1 && argc != 5) {
        std::cout << "usage:\n";
        std::cout << "\thost : run using default stream paths\n";
        std::cout << "\thost config signal read write: specify the "
                     "configuration, signal, read, and write paths.\n";
        exit(1);
    }
    else if (argc == 5) {

        // Set firmware paths
        config_path = argv[1];
        sig_path = argv[2];
        read_path = argv[3];
        write_path = argv[4];
    }

    // Create context
    auto ctx = std::make_shared<oni::context_t>(
        config_path, read_path, write_path, sig_path);

    // Examine device map
    auto dev_map = ctx->device_map();

    // Show device map
    std::cout << "Found the following devices:\n";

    // TODO: use ranges
    int k = 0;
    for (const auto &d : dev_map) {

        const auto dev_str = oni::device_str(d.id);

        std::cout << "\t" << k << ") ID: " << d.id << "("
                  << dev_str << "), Read size: " << d.read_size
                  << "\n";

#ifdef DUMPFILES
        // Open dump files
        std::stringstream ss;
        ss << "idx-" << k <<  "_id-" << d.id << ".raw";
        dump_files.emplace_back();
        dump_files.back() = fopen(ss.str().c_str(), "wb");
#endif
        k++;
    }

    std::cout << "Max. read frame size: "
              << ctx->get_opt<uint32_t>(ONI_MAXREADFRAMESIZE)
              << " bytes\n";

    std::cout << "Block read size: "
              << ctx->get_opt<size_t>(ONI_BLOCKREADSIZE)
              << " bytes\n";

    // TODO: If I specify a 64-bit type param  here, I get a buffer too small
    // exception. If buffer is fixed to size of actual option register in c
    // library, then why must I specify a type parameter here?
    std::cout << "System clock rate: "
              << ctx->get_opt<uint32_t>(ONI_SYSCLKHZ)
              << " Hz\n";

    std::cout << "Acquisition clock rate: "
              << ctx->get_opt<uint32_t>(ONI_ACQCLKHZ)
              << " Hz\n";

    // Start acquisition
    ctx->set_opt(ONI_RUNNING, 1);

    // Generate data thread and continue here config/signal handling in
    // parallel
    std::thread tid(data_loop, ctx);

    // Read stdin
    std::string cmd;
    while (cmd != "q") {

        std::cout << "Enter a command and press enter:\n"
                  << "\tc - toggle 1/100 clock display\n"
                  << "\td - toggle 1/100 display\n"
                  << "\tp - toggle stream pause\n"
                  << "\tr - enter register command\n"
                  << "\tq - quit\n"
                  << ">>> ";

        std::getline(std::cin, cmd);

        if (cmd == "p") {
            running = (running == 1) ? 0 : 1;
            ctx->set_opt(ONI_RUNNING, running);

            if (running)
                std::cout << "Running.\n";
            else
                std::cout << "Paused\n";
        }
        else if (cmd == "c") {
            display_clock = (display_clock == 0) ? 1 : 0;
        }
        else if (cmd == "d") {
            display = (display == 0) ? 1 : 0;
        }
        else if (cmd == "r") {
            std::cout << "Enter dev_idx reg_addr reg_val\n"
                      << ">>> ";

            // Read the command
            std::string reg_cmd;
            std::getline(std::cin, reg_cmd);

            // Parse the command string
            long values[3];
            auto rc = parse_reg_cmd(reg_cmd.c_str(), values);
            if (rc == -1) { std::cerr << "Error: bad command\n"; continue; }

            size_t dev_idx = (size_t)values[0];
            oni_reg_addr_t addr = (oni_reg_addr_t)values[1];
            oni_reg_val_t val = (oni_reg_val_t)values[2];

            ctx->write_reg(dev_idx, addr, val);
        }
    }

    // Join data and signal threads
    quit = 1;
    tid.join();

#ifdef DUMPFILES
    // Close dump files
    for (int dev_idx = 0; dev_idx < dev_map.size(); dev_idx++) {
        fclose(dump_files[dev_idx]);
    }
#endif

    return 0;
}
