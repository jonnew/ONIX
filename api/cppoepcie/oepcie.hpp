#pragma once

#include <algorithm>
#include <cassert>
#include <cstring>
#include <exception>
#include <memory>
#include <string>
#include <system_error>
#include <vector>
#include <span>

#include <oepcie.h>
#include <oedevices.h>
#include <oelogo.h>

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

    // TODO: Data held by frame_t is const. This means data needs to be copied
    // in order to be processed. This is good from a thread safety point of
    // view but potentially bad from an efficency point of view.
    // TODO: This frame class can hold a spans for each device into external
    // storage. Spans are C++20.
    class frame_t
    {
        friend context_t; // NB: Fills frame_t::frame_ptr_;

        public:
            inline frame_t(const device_map_t &dev_map, oe_frame_t *frame)
            : dev_map_(dev_map)
            , frame_ptr_{frame, [=](oe_frame_t *fp) { oe_destroy_frame(fp); }}
            {
                // Nothing
            }

            // NB: Copy and move assignment operators are going to be deleted
            // since this class has const members. Copy and move ctors will
            // implicity delclared. This is good: assignment does not make sense
            // because we need assurance device map is equal. Only way to do
            // this is with construction.

            //TODO: Needed? Data is const. Why would we want to clone this?
            // Access by multiple threads using shallow copies should be safe.
            //inline frame_t clone() const
            //{
            //    auto fp = (oe_frame_t *)malloc(size_);
            //    std::memcpy(fp, frame_ptr_.get(), size_);
            //
            //    // Create frame with allocated pointer
            //    return frame_t(dev_map_, fp, size_);
            //}

            uint64_t clock() const { return frame_ptr_->clock; }

            bool corrupt() const
            {
                return static_cast<bool>(frame_ptr_->corrupt);
            }

            template <typename raw_t>
            raw_t const *begin(size_t dev_idx)
            {
                // Find the position of the requested idx in the frames
                // dev_idx's array to get offset
                auto it = std::find(
                    frame_ptr_->dev_idxs, frame_ptr_->dev_idxs + frame_ptr_->num_dev, dev_idx);

                if (it == frame_ptr_->dev_idxs + frame_ptr_->num_dev)
                    throw(error_t(OE_EDEVIDX));

                // Return iterator dev_idx's data begin()
                auto i = std::distance(frame_ptr_->dev_idxs, it);
                return reinterpret_cast<raw_t *>(frame_ptr_->data
                                                 + frame_ptr_->dev_offs[i]);
            }

            template <typename raw_t>
            raw_t const *end(size_t dev_idx)
            {
                auto it = std::find(
                    frame_ptr_->dev_idxs, frame_ptr_->dev_idxs + frame_ptr_->num_dev, dev_idx);

                if (it == frame_ptr_->dev_idxs + frame_ptr_->num_dev)
                    throw(error_t(OE_EDEVIDX));

                // Return iterator dev_idx's data begin()
                auto i = std::distance(frame_ptr_->dev_idxs, it);
                return reinterpret_cast<raw_t *>(frame_ptr_->data
                                                 + frame_ptr_->dev_offs[i]
                                                 + dev_map_[dev_idx].read_size);
            }

            std::vector<size_t> device_indices() const
            {
                return std::vector<size_t>(frame_ptr_->dev_idxs,
                                           frame_ptr_->dev_idxs + frame_ptr_->num_dev);
            }

        private:
            const device_map_t &dev_map_;
            const std::shared_ptr<const oe_frame_t> frame_ptr_;
    };

    class context_t {

    public:
        inline context_t(const char *config_path = OE_DEFAULTCONFIGPATH,
                         const char *read_path = OE_DEFAULTREADPATH,
                         const char *write_path = OE_DEFAULTWRITEPATH,
                         const char *signal_path = OE_DEFAULTSIGNALPATH)
        {
            // Create
            ctx_ = oe_create_ctx();
            if (ctx_ == nullptr)
                throw std::system_error(errno, std::system_category());

            // Set paths
            set_opt(OE_CONFIGSTREAMPATH, config_path, strlen(config_path) + 1);
            set_opt(OE_SIGNALSTREAMPATH, signal_path, strlen(signal_path) + 1);
            set_opt(OE_READSTREAMPATH, read_path, strlen(read_path) + 1);
            set_opt(OE_WRITESTREAMPATH, write_path, strlen(write_path) + 1);

            // Initialize
            auto rc = oe_init_ctx(ctx_);
            if (rc != 0) throw error_t(rc);

            // Populate device map
            auto num_devs = get_opt<oe_size_t>(OE_NUMDEVICES);

            size_t devices_sz = sizeof(oe_device_t) * num_devs;
            device_map_.resize(num_devs);
            get_opt(OE_DEVICEMAP, device_map_.data(), &devices_sz);
        }

        // No copies
        inline context_t(const context_t &) = delete;
        inline context_t &operator=(const context_t &) = delete;

        // Moves are OK
        inline context_t(context_t &&rhs) noexcept
            : ctx_(rhs.ctx_),
              device_map_(std::move(rhs.device_map_))
        {
            rhs.ctx_ = nullptr;
        }

        inline context_t &operator=(context_t &&rhs) noexcept
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

        inline void
        write_reg(size_t dev_idx, oe_reg_addr_t addr, oe_reg_val_t value)
        {
            auto rc = oe_write_reg(ctx_, dev_idx, addr, value);
            if (rc != 0)
                throw error_t(rc);
        }

        inline device_map_t device_map() const noexcept { return device_map_; }

        inline frame_t read_frame() const
        {
            oe_frame_t *fp;
            auto rc = oe_read_frame(ctx_, &fp);
            if (rc < 0) throw error_t(rc);

            // TODO: Should use RVO, check disassembly
            return frame_t(device_map_, fp);
        }

        template <typename data_t>
        inline void write(size_t dev_idx, std::vector<data_t> data) const 
        {
            auto rc = oe_write(
                ctx_, dev_idx, data.data, data.size * sizeof(data_t));
            if (rc < 0) throw error_t(rc);
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

            // Reset the hardware, ignore error codes since this may or
            // this is called in destructor and we cannot throw.
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
