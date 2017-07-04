#pragma once

// TODO: Are all these includes needed?
#include <array>
#include <iostream>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

// TODO: This is wrong. Depends on firmware definition.
#define SAMPLE_HEADER_BYTES 32

// NB: http://xillybus.com/doc/bandwidth-guidelines suggests 128kb per read to
// maximize average bandwdith performance
// For 256 channels, this is ~250 samples. At 30kHz, this is 8 millseconds,
// which is likely too slow.

// NB: Configuratoin xillybu streams should be synchronous so that the file descriptors are seekable
//     Data xillybus streams should be asynchronous so that the FPGAs FIFO does
//     not overflow when the reading process is pre-empted
#ifndef O_BINARY
#define O_BINARY 0
#endif

namespace oe {

static inline const char *errmsg(const char *prefix = "")
{
    auto msg = std::string(prefix);
    msg += ": ";
    msg += strerror(errno);
    return msg.c_str();
}

static inline void throw_err(const char *prefix)
{
    throw std::runtime_error(errmsg(prefix));
}

// TODO: Remove crap. Add controls for LED brightness, trigger, etc.
enum config_addr : int {
    reset_run = 0x00,
    max_timestep_lsb = 0x02,
    max_timestep_msb = 0x04,
    data_freq_pll = 0x06,
    miso_delay = 0x08,
    aux_cmdbank1 = 0x10,
    aux_cmdbank2 = 0x12,
    aux_cmdbank3 = 0x14,
    aux_cmdlength1 = 0x16,
    aux_cmdlength2 = 0x18,
    aux_cmdlength3 = 0x1a,
    aux_cmdloop1 = 0x1c,
    aux_cmdloop2 = 0x1e,
    aux_cmdloop3 = 0x20,
    data_streamsel1234 = 0x24,
    data_streamsel5678 = 0x26,
    data_streamsel9abc = 0x28,
    data_streamseldef10 = 0x2a,
    data_stream_en = 0x2c,
    aux_outputs = 0x2e,
    start = 0x3e
};

// *** Open Ephys PCIe interface *** //

template <typename Headstage>
class pcie_device {

    // Number of bytes in a single block read
    // TODO: Overhead bytes and sample number?
    // TODO: This depends on the memory layout for a particular headstage
    static constexpr size_t block_bytes
        = NumChannels * (BlockSamples + SAMPLE_HEADER_BYTES);

    // Control output and data input
    int ctrl_fid_ = -1;
    int data_fid_ = -1;

public:
    // Raw data and block types
    using raw_t = int16_t;
    using raw_block_t
        = std::array<std::array<raw_t, BlockSamples>, NumChannels>;

    // TODO: Better names for these device files
    pcie_device(const char *data_addr = "/dev/xillybus_neural_data_32",
                const char *ctrl_addr = "/dev/xillybus_control_regs_16")
    {
        data_fid_ = open(data_addr, O_RDWR | O_BINARY);
        ctrl_fid_ = open(ctrl_addr, O_RDWR | O_BINARY);

        if (data_fid_ < 0)
            throw_err(data_addr);

        if (ctrl_fid_ < 0)
            throw_err(ctrl_addr);
    }

    // Rule of 5 without thinking too much.
    pcie_device(const pcie_device &other) = delete;
    pcie_device &operator=(const pcie_device &other) = delete;
    pcie_device(const pcie_device &&other) = delete;
    pcie_device &operator=(const pcie_device &&other) = delete;

    ~pcie_device()
    {
        if (data_fid_ != -1)
            if (!close(data_fid_))
                std::cerr << errmsg();

        if (ctrl_fid_ != -1)
            if (!close(ctrl_fid_))
                std::cerr << errmsg();
    }

    // TODO: This is insane.
    // 1. There should be no mask
    // 2. Each configure value should be a single 16-bit register
    // 3. The reading just to write should be removed!
    int configure(config_addr ctrl_addr, uint16_t value, uint16_t mask = 0xffff)
    {
        // TODO: What to do if acqusition is already running?

        if (lseek(ctrl_fid_, ctrl_addr, SEEK_SET) < 0) {
            std::cerr << "Error seeking control to address " << ctrl_addr
                      << std::endl;
            return -1;
        }

        uint16_t write_val;
        uint16_t cur_val = 0x1010; // an easily recognizable value, to
        // distinguish it from an actual read value of
        // zero in debug strings
        if ((mask & 0xffff) != 0xffff) {

            int rd = read(ctrl_fid_, &cur_val, 2);
            if (rd < 2) {
                std::cerr << "Unsuccessful read to control address "
                          << ctrl_addr << " code: " << rd << std::endl;
                return -1;
            }
            if (lseek(ctrl_fid_, ctrl_addr, SEEK_SET) < 0) {
                std::cerr << "Error re-seeking control to address " << ctrl_addr
                          << std::endl;
                return -1;
            }
            write_val = (cur_val & ~mask) | (value & mask);
        } else {
            write_val = value;
        }

        int wd = write(ctrl_fid_, &write_val, 2);
        if (wd < 2) {
            std::cerr << "Unsuccessful write to control address " << ctrl_addr
                      << " code: " << wd << std::endl;
            return -1;
        }

        return 0;
    }

    // Fill a single multichannel data block
    // TODO: Get sample number, too
    int read_block(raw_block_t &block) // , int64_t &sample_number)
    {
        // TODO: Ensure hardware is started
        // TODO: Ensure data alignment and pull first sample number in block
        // TODO: Stride over header bytes somehow?
        return read(data_fid_, (void *)block.data(), block_bytes);
    }

};

} // namespace oe
