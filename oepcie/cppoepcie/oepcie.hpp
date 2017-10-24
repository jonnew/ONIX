#pragma once

#include <algorithm>
#include <cassert>
#include <exception>
#include <string>
#include <system_error>
#include <tuple>
#include <vector>

#include <oepcie.h>

// In order to prevent unused variable warnings when building in non-debug
// mode use this macro to make assertions.
#ifndef NDEBUG
#   define OE_ASSERT(expression) assert(expression)
#else
#   define OE_ASSERT(expression) (void)(expression)
#endif

namespace oe {

    class error_t : public std::exception
    {
    public:
        explicit error_t(int errnum) : errnum_(errnum) {}

        virtual const char *what() const noexcept
        {
            return oe_error_str(errnum_);
        }

        int num() const { return errnum_; }

    private:
        int errnum_;
    };

    inline std::tuple<int, int, int> version()
    {
        std::tuple<int, int, int> v;
        oe_version(&std::get<0>(v), &std::get<1>(v), &std::get<2>(v) );
        return v;
    }

    inline const char *device_str(int dev_id)
    {
        return oe_device_str(dev_id);
    }

    class context_t;

    using device_map_t = std::vector<oe_device_t>;

    class frame_t
    {
        friend context_t; // Fills data_
        using raw_t = uint8_t;

        public:

            inline frame_t(size_t size, device_map_t &dev_map)
            : size_(size)
            , data_(new raw_t[size])
            , dev_map_(dev_map)
            {
                // Nothing
            }

            inline frame_t(const frame_t &rhs)
            : size_(rhs.size_)
            , data_(new raw_t[rhs.size_])
            , dev_map_(rhs.dev_map_)
            {
                std::memcpy(data_, rhs.data_, sizeof(raw_t) * rhs.size_);
            }

            inline frame_t(frame_t &&rhs)
            : size_(rhs.size_)
            , data_(rhs.data_)
            , dev_map_(rhs.dev_map_)
            {
                rhs.data_ = nullptr;
            }

            inline frame_t &operator=(const frame_t &rhs)
            {
                if (&rhs == this)
                    return *this;

                size_ = rhs.size_;
                auto tmp = new raw_t[rhs.size_];
                std::memcpy(tmp, rhs.data_, sizeof(raw_t) * rhs.size_);
                delete [] data_;
                data_ = tmp;
                dev_map_ = rhs.dev_map_;

                return *this;
            }

            inline frame_t &operator=(frame_t &&rhs)
            {
                if (&rhs == this)
                    return *this;

                size_ = rhs.size_;
                delete[] data_;
                data_ = rhs.data_;
                rhs.data_ = nullptr;
                dev_map_ = rhs.dev_map_;

                return *this;
            }

            ~frame_t() noexcept {
                delete [] data_;
            }

            uint64_t time() { return *reinterpret_cast<uint64_t *>(data_); }

            template <typename sample_t>
            sample_t * begin(size_t dev_idx)
            {
                return reinterpret_cast<sample_t *>(
                    data_ + dev_map_[dev_idx].read_offset);
            }

            template <typename sample_t>
            sample_t * end(size_t dev_idx)
            {
                return reinterpret_cast<sample_t *>(
                           data_ + dev_map_[dev_idx].read_offset
                           + dev_map_[dev_idx].read_size) + 1;
            }

        private:
            size_t size_;
            raw_t *data_ = nullptr;
            device_map_t &dev_map_;
    };

    class context_t {

    public:

        inline context_t(const char* config_path = OE_DEFAULTCONFIGPATH,
                         const char* read_path = OE_DEFAULTREADPATH,
                         const char* signal_path = OE_DEFAULTSINGALPATH)
        {
            // Create
            ctx_ = oe_create_ctx();
            if (ctx_ == nullptr)
                throw std::system_error(errno, std::system_category());

            // Set paths
            set_opt(OE_CONFIGSTREAMPATH, config_path, strlen(config_path) + 1);
            set_opt(OE_SIGNALSTREAMPATH, signal_path, strlen(signal_path) + 1);
            set_opt(OE_READSTREAMPATH, read_path, strlen(read_path) + 1);

            // Initialize
            auto rc = oe_init_ctx(ctx_);
            if (rc != 0) throw error_t(rc);

            // Populate device map
            auto num_devs = get_opt<oe_size_t>(OE_NUMDEVICES);

            size_t devices_sz = sizeof(oe_device_t) * num_devs;
            device_map_.resize(num_devs);
            get_opt(OE_DEVICEMAP, device_map_.data(), &devices_sz);

            // Get data frame size and allocate
            frame_size_ = get_opt<oe_size_t>(OE_READFRAMESIZE);
            buffer_ = std::vector<uint8_t>(frame_size_ * frames_per_update_);
        }

        // No copy
        inline context_t(const context_t &) = delete;
        inline context_t &operator = (const context_t &) = delete;

        // Moves OK
        inline context_t(context_t &&rhs) noexcept
        : ctx_(rhs.ctx_)
        , device_map_(rhs.device_map_)
        , frame_size_(rhs.frame_size_)
        , buffer_(rhs.buffer_)
        , buf_idx_(rhs.buf_idx_)
        {
            rhs.ctx_ = nullptr;
        }

        inline context_t &operator = (context_t &&rhs) noexcept
        {
            std::swap(ctx_, rhs.ctx_);
            std::swap(device_map_, rhs.device_map_);
            std::swap(frame_size_, rhs.frame_size_);
            std::swap(buffer_, rhs.buffer_);
            std::swap(buf_idx_, rhs.buf_idx_);
            return *this;
        }

        inline ~context_t() noexcept { close(); }

        // NB: Called in destructor, no throwing
        inline void close() noexcept
        {
            if (ctx_ == nullptr)
                return;

            // Reset the hardware, ignore error codes since this may or may not
            // be approriate
            oe_reg_val_t reset = 1;
            oe_set_opt(ctx_, OE_RESET, &reset, sizeof(reset));

            // Free resources
            auto rc = oe_destroy_ctx(ctx_);
            OE_ASSERT(rc == 0);
            ctx_ = nullptr;
        }

        template <typename opt_type>
        opt_type get_opt(int option) const
        {
            opt_type optval;
            size_t optlen = sizeof(opt_type);
            get_opt(option, &optval, &optlen);
            return optval;
        }

        template <typename opt_type>
        void set_opt(int option, opt_type const &optval)
        {
            set_opt(option, &optval, sizeof(opt_type));
        }

        inline oe_reg_val_t read_reg(size_t dev_idx, oe_reg_addr_t addr)
        {
            oe_reg_val_t *value;
            auto rc = oe_read_reg(ctx_, dev_idx, addr, value);
            if (rc != 0) throw error_t(rc);
            return *value;
        }

        inline void write_reg(size_t dev_idx, oe_reg_addr_t addr, oe_reg_val_t value)
        {
            auto rc = oe_write_reg(ctx_, dev_idx, addr, value);
            if (rc != 0) throw error_t(rc);
        }

        inline device_map_t device_map() const { return device_map_; }

        inline frame_t read_frame()
        {
            // If we have exausted buffer
            if (buf_idx_ == frames_per_update_)
                update_buffer();

            // Move data from buffer into frame
            frame_t frame(frame_size_, device_map_);
            std::move(buffer_.begin() + buf_idx_ * frame_size_,
                      buffer_.begin() + (buf_idx_ + 1) * frame_size_,
                      frame.data_);

            // Increment buffer index
            buf_idx_ += 1;

            // Should be elided
            return frame;
        }

    private:
        inline void
        get_opt(int option, void *value, size_t *size) const
        {
            auto rc = oe_get_opt(ctx_, option, value, size);
            if (rc != 0) throw error_t(rc);
        }

        inline void
        set_opt(int option, const void *value, size_t size)
        {
            auto rc = oe_set_opt(ctx_, option, value, size);
            if (rc != 0) throw error_t(rc);
        }

        inline void update_buffer() {
           auto rc =  oe_read(ctx_, buffer_.data(), buffer_.size());
           if (rc < 0) throw error_t(rc);
           if (rc != buffer_.size())
               throw std::runtime_error("Incomplete read.");
           buf_idx_ = 0;
        }

        oe_ctx ctx_ = nullptr;
        device_map_t device_map_;
        size_t frame_size_;
        static constexpr size_t frames_per_update_ = 100; // TODO: Settable
        std::vector<uint8_t> buffer_;
        size_t buf_idx_ = frames_per_update_;
    };
}
