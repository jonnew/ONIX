#pragma once

#include <algorithm>
#include <cassert>
#include <cstring>
#include <exception>
#include <memory>
#include <string>
#include <system_error>
#include <vector>

#if __cplusplus >= 201707
#include <span>
#endif

#include <oni.h>
#include <onidevices.h>

// In order to prevent unused variable warnings when building in non-debug
// mode use this macro to make assertions.
#ifndef NDEBUG
#define ONI_ASSERT(expression) assert(expression)
#else
#define ONI_ASSERT(expression) (void)(expression)
#endif

namespace oni {

    class error_t : public std::exception
    {
    public:
        explicit error_t(int errnum) : errnum_(errnum) {}

        virtual const char *what() const noexcept
        {
            return oni_error_str(errnum_);
        }

        int num() const { return errnum_; }

    private:
        int errnum_;
    };

    inline std::tuple<int, int, int> version()
    {
        std::tuple<int, int, int> v;
        oni_version(&std::get<0>(v), &std::get<1>(v), &std::get<2>(v) );
        return v;
    }

    class context_t;

    using device_t = oni_device_t;
    using device_map_t = std::vector<device_t>;
    template<typename T> using driver_arg_t = std::pair<int,T>;

    // TODO: Data held by frame_t is const. This means data needs to be copied
    // in order to be processed. This is good from a thread safety point of
    // view but potentially bad from an efficency point of view.
    // TODO: This frame class can hold a spans for each device into external
    // storage. Spans are C++20.
    class frame_t
    {
        friend class context_t; // NB: Fills frame_t::frame_ptr_;

        public:
            inline frame_t(const device_map_t &dev_map, oni_frame_t *frame)
            : dev_map_(dev_map)
            , frame_ptr_{frame, [=](oni_frame_t *fp) { oni_destroy_frame(fp); }}
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
            //    auto fp = (oni_frame_t *)malloc(size_);
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

#if __cplusplus >= 201707
            template <typename raw_t>
            std::span<const raw_t> data(size_t dev_idx)
            {
                // Find the position of the requested idx in the frames
                // dev_idx's array to get offset
                auto it = std::find(
                    frame_ptr_->dev_idxs, frame_ptr_->dev_idxs + frame_ptr_->num_dev, dev_idx);

                if (it == frame_ptr_->dev_idxs + frame_ptr_->num_dev)
                    throw(error_t(ONI_EDEVIDX));

                // Return iterator dev_idx's data begin()
                auto i = std::distance(frame_ptr_->dev_idxs, it);
                auto begin = reinterpret_cast<raw_t *>(
                    frame_ptr_->data + frame_ptr_->dev_offs[i]);

                return std::span(begin, dev_map_[dev_idx].read_size / sizeof(raw_t));
            }
#endif
            template <typename raw_t>
            raw_t const *begin(size_t dev_idx)
            {
                // Find the position of the requested idx in the frames
                // dev_idx's array to get offset
                auto it = std::find(
                    frame_ptr_->dev_idxs, frame_ptr_->dev_idxs + frame_ptr_->num_dev, dev_idx);

                if (it == frame_ptr_->dev_idxs + frame_ptr_->num_dev)
                    throw(error_t(ONI_EDEVIDX));

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
                    throw(error_t(ONI_EDEVIDX));

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
            const std::shared_ptr<const oni_frame_t> frame_ptr_;
    };

    class context_t {

    public:
        template<typename... DriverArgs>
        inline context_t(const char *driver_name, int host_idx, DriverArgs... args)
        {
            // Create
            ctx_ = oni_create_ctx(driver_name);
            if (ctx_ == nullptr)
                throw std::system_error(errno, std::system_category());

            // Apply driver-specific options
            for(const auto a : {args...})
                set_driver_opt(std::get<0>(a), std::get<1>(a));

            // Initialize
            auto rc = oni_init_ctx(ctx_, host_idx);
            if (rc != 0) throw error_t(rc);

            // Populate device map
            auto num_devs = get_opt<oni_size_t>(ONI_NUMDEVICES);

            size_t devices_sz = sizeof(oni_device_t) * num_devs;
            device_map_.resize(num_devs);
            get_opt(ONI_DEVICEMAP, device_map_.data(), &devices_sz);
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

        template <typename opt_t>
        opt_t get_opt(int option) const
        {
            opt_t optval;
            size_t optlen = sizeof(opt_t);
            get_opt(option, &optval, &optlen);
            return optval;
        }

        template <typename opt_t>
        inline void set_opt(int option, opt_t const &optval)
        {
            if constexpr(std::is_pointer<opt_t>::value)
                set_opt(option, optval, opt_size(optval));
            else
                set_opt(option, &optval, opt_size(optval));
        }

        template <typename opt_t>
        opt_t get_driver_opt(int option) const
        {
            opt_t optval;
            size_t optlen = sizeof(opt_t);
            get_driver_opt(option, &optval, &optlen);
            return optval;
        }

        template <typename opt_t>
        inline void set_driver_opt(int option, opt_t const &optval)
        {
            if constexpr(std::is_pointer<opt_t>::value)
                set_driver_opt(option, optval, opt_size(optval));
            else
                set_driver_opt(option, &optval, opt_size(optval));
        }

        inline oni_reg_val_t read_reg(size_t dev_idx, oni_reg_addr_t addr)
        {
            oni_reg_val_t value = 0;
            auto rc = oni_read_reg(ctx_, dev_idx, addr, &value);
            if (rc != 0) throw error_t(rc);
            return value;
        }

        inline void
        write_reg(size_t dev_idx, oni_reg_addr_t addr, oni_reg_val_t value)
        {
            auto rc = oni_write_reg(ctx_, dev_idx, addr, value);
            if (rc != 0)
                throw error_t(rc);
        }

        inline device_map_t device_map() const noexcept { return device_map_; }

        inline frame_t read_frame() const
        {
            oni_frame_t *fp;
            auto rc = oni_read_frame(ctx_, &fp);
            if (rc < 0) throw error_t(rc);

            // TODO: Should use RVO, check disassembly
            return frame_t(device_map_, fp);
        }

        template <typename data_t>
        inline void write(size_t dev_idx, std::vector<data_t> data) const
        {
            auto rc = oni_write(
                ctx_, dev_idx, data.data, data.size * sizeof(data_t));
            if (rc < 0) throw error_t(rc);
        }

    private:

        template<typename opt_t>
        inline size_t opt_size(opt_t opt)
        {
            size_t optlen = 0;
            if constexpr(std::is_same<opt_t, char *>::value)
                optlen = strlen(opt) + 1;
            if constexpr(std::is_same<opt_t, const char *>::value)
                optlen = strlen(opt) + 1;
            else
                optlen = sizeof(opt);

            return optlen;
        }

        inline void get_opt(int option, void *value, size_t *size) const
        {
            auto rc = oni_get_opt(ctx_, option, value, size);
            if (rc != 0) throw error_t(rc);
        }

        inline void set_opt(int option, const void *value, size_t size)
        {
            auto rc = oni_set_opt(ctx_, option, value, size);
            if (rc != 0) throw error_t(rc);
        }

        inline void get_driver_opt(int option, void *value, size_t *size) const
        {
            auto rc = oni_get_driver_opt(ctx_, option, value, size);
            if (rc != 0) throw error_t(rc);
        }

        inline void set_driver_opt(int option, const void *value, size_t size)
        {
            auto rc = oni_set_driver_opt(ctx_, option, value, size);
            if (rc != 0) throw error_t(rc);
        }

        // NB: Called in destructor, no throwing
        inline void close() noexcept
        {
            if (ctx_ == nullptr)
                return;

            // Reset the hardware, ignore error codes since this may or
            // this is called in destructor and we cannot throw.
            oni_reg_val_t reset = 1;
            oni_set_opt(ctx_, ONI_RESET, &reset, sizeof(reset));

            // Free resources
            auto rc = oni_destroy_ctx(ctx_);
            ONI_ASSERT(rc == 0);
            ctx_ = nullptr;
        }

        oni_ctx ctx_ = nullptr;
        device_map_t device_map_;
    };
}
