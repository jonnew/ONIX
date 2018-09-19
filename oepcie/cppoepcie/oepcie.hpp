#pragma once

#include <algorithm>
#include <cassert>
#include <exception>
#include <string>
#include <system_error>
#include <tuple>
#include <vector>

#include <oepcie.h>
#include <oedevices.h>

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

    class context_t;

    using device_t = oe_device_t;
    using device_map_t = std::vector<device_t>;

    class frame_t
    {
        friend context_t; // NB: Fills frame_t::frame_;

        public:

            inline explicit frame_t(const device_map_t &dev_map)
            : dev_map_(dev_map)
            {
                // Nothing
            }

            inline frame_t(const frame_t &rhs)
            : dev_map_(rhs.dev_map_)
            {
                frame_ = static_cast<oe_frame_t *>(malloc(sizeof(oe_frame_t)));
                frame_->clock = rhs.frame_->clock;
                frame_->num_dev = rhs.frame_->num_dev;
                frame_->corrupt = rhs.frame_->corrupt;
                frame_->dev_idxs_sz = rhs.frame_->dev_idxs_sz;
                frame_->dev_offs_sz = rhs.frame_->dev_offs_sz;
                frame_->data_sz = rhs.frame_->data_sz;

                std::memcpy(frame_->dev_offs, rhs.frame_->dev_offs, rhs.frame_->dev_offs_sz);
                std::memcpy(frame_->dev_idxs, rhs.frame_->dev_idxs, rhs.frame_->dev_idxs_sz);
                std::memcpy(frame_->data, rhs.frame_->data, rhs.frame_->data_sz);
            }

            inline frame_t(frame_t &&rhs)
            : dev_map_(rhs.dev_map_)
            , frame_(rhs.frame_)
            {
                rhs.frame_ = nullptr;
            }

            inline frame_t &operator=(const frame_t &rhs)
            {
                if (&rhs == this)
                    return *this;

                const_cast<device_map_t&>(dev_map_) = rhs.dev_map_;

                oe_destroy_frame(frame_);
                frame_ = static_cast<oe_frame_t *>(malloc(sizeof(oe_frame_t)));
                frame_->clock = rhs.frame_->clock;
                frame_->num_dev = rhs.frame_->num_dev;
                frame_->corrupt = rhs.frame_->corrupt;
                frame_->dev_idxs_sz = rhs.frame_->dev_idxs_sz;
                frame_->dev_offs_sz = rhs.frame_->dev_offs_sz;
                frame_->data_sz = rhs.frame_->data_sz;

                std::memcpy(frame_->dev_offs, rhs.frame_->dev_offs, rhs.frame_->dev_offs_sz);
                std::memcpy(frame_->dev_idxs, rhs.frame_->dev_idxs, rhs.frame_->dev_idxs_sz);
                std::memcpy(frame_->data, rhs.frame_->data, rhs.frame_->data_sz);

                return *this;
            }

            inline frame_t &operator=(frame_t &&rhs)
            {
                if (&rhs == this)
                    return *this;

                const_cast<device_map_t&>(dev_map_) = rhs.dev_map_;

                oe_destroy_frame(frame_);
                frame_ = rhs.frame_;
                rhs.frame_ = nullptr;

                return *this;
            }

            ~frame_t() noexcept { oe_destroy_frame(frame_); }

            uint64_t time() { return frame_->clock; }
            bool corrupt() { return static_cast<bool>(frame_->corrupt); }

            // TODO: raw_t should be deduced from call to oe_raw_type() using
            // c++14 features
            template <typename raw_t>
            raw_t *begin(size_t dev_idx)
            {
                // Find the position of the requested idx in the frames
                // dev_idx's array to get offset
                auto it = std::find(
                    frame_->dev_idxs, frame_->dev_idxs + frame_->num_dev, dev_idx);

                if (it == frame_->dev_idxs + frame_->num_dev)
                    throw(error_t(OE_EDEVIDX));

                // Return iterator dev_idx's data begin()
                auto i = std::distance(frame_->dev_idxs, it);
                return reinterpret_cast<raw_t *>(frame_->data
                                                 + frame_->dev_offs[i]);
            }

            template <typename raw_t>
            raw_t *end(size_t dev_idx)
            {
                auto it = std::find(
                    frame_->dev_idxs, frame_->dev_idxs + frame_->num_dev, dev_idx);

                if (it == frame_->dev_idxs + frame_->num_dev)
                    throw(error_t(OE_EDEVIDX));

                // Return iterator dev_idx's data begin()
                auto i = std::distance(frame_->dev_idxs, it);
                return reinterpret_cast<raw_t *>(frame_->data
                                                 + frame_->dev_offs[i]
                                                 + dev_map_[dev_idx].read_size);
            }

            std::vector<size_t> device_indices() const
            {
                return std::vector<size_t>(frame_->dev_idxs,
                                           frame_->dev_idxs + frame_->num_dev);
            }

        private:
            oe_frame_t *frame_ = nullptr;
            const device_map_t &dev_map_;
    };

    class context_t {

    public:
        inline context_t(const char* config_path = OE_DEFAULTCONFIGPATH,
                         const char* read_path = OE_DEFAULTREADPATH,
                         const char* signal_path = OE_DEFAULTSIGNALPATH)
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
        }

        // No copy
        inline context_t(const context_t &) = delete;
        inline context_t &operator = (const context_t &) = delete;

        // Moves OK
        inline context_t(context_t &&rhs) noexcept
        : ctx_(rhs.ctx_)
        , device_map_(std::move(rhs.device_map_))
        {
            rhs.ctx_ = nullptr;
        }

        inline context_t &operator = (context_t &&rhs) noexcept
        {
            std::swap(ctx_, rhs.ctx_);
            device_map_ = rhs.device_map_;
            return *this;
        }

        inline ~context_t() noexcept { close(); }


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
            oe_reg_val_t value = 0;
            auto rc = oe_read_reg(ctx_, dev_idx, addr, &value);
            if (rc != 0) throw error_t(rc);
            return value;
        }

        inline void write_reg(size_t dev_idx, oe_reg_addr_t addr, oe_reg_val_t value)
        {
            auto rc = oe_write_reg(ctx_, dev_idx, addr, value);
            if (rc != 0) throw error_t(rc);
        }

        inline device_map_t device_map() const { return device_map_; }

        inline frame_t read_frame()
        {
            frame_t frame(device_map_);
            oe_read_frame(ctx_, &frame.frame_);

            // TODO: Should be elided, check disassembly
            return frame;
        }

    private:
        inline void get_opt(int option, void *value, size_t *size) const
        {
            auto rc = oe_get_opt(ctx_, option, value, size);
            if (rc != 0) throw error_t(rc);
        }

        inline void set_opt(int option, const void *value, size_t size)
        {
            auto rc = oe_set_opt(ctx_, option, value, size);
            if (rc != 0) throw error_t(rc);
        }

        // NB: Called in destructor, no throwing
        inline void close() noexcept
        {
            if (ctx_ == nullptr)
                return;

            // Reset the hardware, ignore error codes since this may or may not
            // be appropriate
            oe_reg_val_t reset = 1;
            oe_set_opt(ctx_, OE_RESET, &reset, sizeof(reset));

            // Free resources
            auto rc = oe_destroy_ctx(ctx_);
            OE_ASSERT(rc == 0);
            ctx_ = nullptr;
        }

        oe_ctx ctx_ = nullptr;
        device_map_t device_map_;
    };
}
