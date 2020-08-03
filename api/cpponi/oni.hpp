#pragma once

#include <algorithm>
#include <cassert>
#include <cstring>
#include <exception>
#include <memory>
#include <string>
#include <system_error>
#include <vector>
#include <unordered_map>

#ifdef _WIN32
#if _MSVC_LANG >= 201704
#include <span>
#define CPPONI_USE_SPAN
#endif
#else
#if __cplusplus >= 201707
#include <span>
#define ONI_USE_SPAN
#endif
#endif

#include <oni.h>
#include <onidevices.h>

// Version macros for compile-time API version detection
// NB: see https://semver.org/
#define CPPONI_VERSION_MAJOR 3
#define CPPONI_VERSION_MINOR 1
#define CPPONI_VERSION_PATCH 0

#define CPPONI_VERSION                                                         \
    ONI_MAKE_VERSION(                                                          \
        CPPONI_VERSION_MAJOR, CPPONI_VERSION_MINOR, CPPONI_VERSION_PATCH)

// In order to prevent unused variable warnings when building in non-debug
// mode use this macro to make assertions.
#ifndef NDEBUG
#define ONI_ASSERT(expression) assert(expression)
#else
#define ONI_ASSERT(expression) (void)(expression)
#endif

namespace oni {

    static_assert(ONI_VERSION >= ONI_MAKE_VERSION(3, 0, 0), "liboni is too old.");

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
    using device_map_t = std::unordered_map<oni_dev_idx_t, device_t>;
    template<typename T> using driver_arg_t = std::pair<int,T>;

    // NB: Data held by frame_t is const. This means data needs to be copied
    // in order to be processed. This is good from a thread safety point of
    // view but potentially bad from an efficiency point of view.
    // NB: This frame class provide spans for each device that look into external
    // storage. Spans are C++20.
    class frame_t
    {
        friend class context_t; // NB: Fills frame_t::frame_ptr_;

    public:
        // NB: Copy and move assignment operators are going to be deleted
        // since this class has const members. Copy and move ctors will
        // implicitly declared.
        inline frame_t(oni_frame_t *frame)
        : frame_ptr_{frame, [=](oni_frame_t *fp) { oni_destroy_frame(fp); }}
        {
            // Nothing
        }

        uint64_t time() const { return frame_ptr_->time; }
        oni_dev_idx_t device_index() const { return frame_ptr_->dev_idx; }

#ifdef CPPONI_USE_SPAN
        // Data view, no copy
        template <typename raw_t>
        std::span<const raw_t> data() const
        {
            return std::span(reinterpret_cast<raw_t *>(frame_ptr_->data),
                                frame_ptr_->data_sz / sizeof(raw_t));
        }
#else
        // Copies
        template <typename raw_t>
        std::vector<raw_t> data() const
        {
            // This should be RVOed
            return std::vector(
                reinterpret_cast<raw_t *const>(frame_ptr_->data),
                reinterpret_cast<raw_t * const>(frame_ptr_->data) + frame_ptr_->data_sz / sizeof(raw_t));
        }
#endif

    private:
        const std::shared_ptr<const oni_frame_t> frame_ptr_;
    };

    class context_t {

    public:
        inline context_t(const char *driver_name, int host_idx)
        {
            // Create
            ctx_ = oni_create_ctx(driver_name);
            if (ctx_ == nullptr)
                throw std::system_error(errno, std::system_category());

            // Initialize
            auto rc = oni_init_ctx(ctx_, host_idx);
            if (rc != 0) throw error_t(rc);

            // Populate device map
            auto num_devs = get_opt<oni_size_t>(ONI_OPT_NUMDEVICES);

            size_t devices_sz = sizeof(oni_device_t) * num_devs;
            device_map_.reserve(num_devs);

            std::vector<device_t> devs;
            devs.resize(num_devs);
            get_opt_(ONI_OPT_DEVICETABLE, devs.data(), &devices_sz);

            // Convert to unordered_map
            for (const auto &d : devs)
                device_map_.insert({d.idx, d});
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
            device_map_ = std::move(rhs.device_map_);
            return *this;
        }

        inline ~context_t() noexcept { close_(); }

        template <typename opt_t>
        opt_t get_opt(int option) const
        {
            opt_t optval;
            size_t optlen = sizeof(opt_t);
            get_opt_(option, &optval, &optlen);
            return optval;
        }

        template <typename opt_t>
        inline void set_opt(int option, opt_t const &optval)
        {
            if constexpr(std::is_pointer<opt_t>::value)
                set_opt_(option, optval, opt_size_(optval));
            else
                set_opt_(option, &optval, opt_size_(optval));
        }

        template <typename opt_t>
        opt_t get_driver_opt(int option) const
        {
            opt_t optval;
            size_t optlen = sizeof(opt_t);
            get_driver_opt_(option, &optval, &optlen);
            return optval;
        }

        template <typename opt_t>
        inline void set_driver_opt(int option, opt_t const &optval)
        {
            if constexpr(std::is_pointer<opt_t>::value)
                set_driver_opt_(option, optval, opt_size_(optval));
            else
                set_driver_opt_(option, &optval, opt_size_(optval));
        }

        inline oni_reg_val_t read_reg(oni_dev_idx_t dev_idx,
                                      oni_reg_addr_t addr)
        {
            oni_reg_val_t value = 0;
            auto rc = oni_read_reg(ctx_, dev_idx, addr, &value);
            if (rc != 0) throw error_t(rc);
            return value;
        }

        inline void write_reg(oni_dev_idx_t dev_idx,
                              oni_reg_addr_t addr,
                              oni_reg_val_t value)
        {
            auto rc = oni_write_reg(ctx_, dev_idx, addr, value);
            if (rc != 0) throw error_t(rc);
        }

        inline device_map_t device_map() const noexcept { return device_map_; }

        inline frame_t read_frame() const
        {
            oni_frame_t *fp;
            auto rc = oni_read_frame(ctx_, &fp);
            if (rc < 0) throw error_t(rc);

            // TODO: Should use RVO, check disassembly
            return frame_t(fp);
        }

#ifdef CPPONI_USE_SPAN
        template <typename data_t>
        inline void write(size_t dev_idx, std::span<const data_t> data) const
        {
            // Light-weight allocate write frame
            oni_frame_t *w_frame = NULL;
            int rc= oni_create_frame(ctx_, &w_frame, dev_idx, data.size_bytes());
            if (rc < 0) throw error_t(rc);

            // Copy data into frame
            memcpy(w_frame->data, data.data(), data.size_bytes());

            // Do write
            rc = oni_write_frame(ctx_, w_frame);
            if (rc < 0)
                throw error_t(rc);

            oni_destroy_frame(w_frame);
        }
#else
        template <typename data_t>
        inline void write(size_t dev_idx, std::vector<data_t> data) const

        {
            // Light-weight allocate write frame
            oni_frame_t  *w_frame = NULL;
            int rc = oni_create_frame(ctx_, &w_frame, dev_idx, data.size() * sizeof(data_t));
            if (rc < 0) throw error_t(rc);

            // Copy data into frame
            memcpy(w_frame->data, data.data(), data.size() * sizeof(data_t));

            // Do write
            rc = oni_write_frame(ctx_, w_frame);
            if (rc < 0) throw error_t(rc);

            oni_destroy_frame(w_frame);
        }
#endif
    private:

        template<typename opt_t>
        inline size_t opt_size_(opt_t opt)
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

        inline void get_opt_(int option, void *value, size_t *size) const
        {
            auto rc = oni_get_opt(ctx_, option, value, size);
            if (rc != 0) throw error_t(rc);
        }

        inline void set_opt_(int option, const void *value, size_t size)
        {
            auto rc = oni_set_opt(ctx_, option, value, size);
            if (rc != 0) throw error_t(rc);
        }

        inline void get_driver_opt_(int option, void *value, size_t *size) const
        {
            auto rc = oni_get_driver_opt(ctx_, option, value, size);
            if (rc != 0) throw error_t(rc);
        }

        inline void set_driver_opt_(int option, const void *value, size_t size)
        {
            auto rc = oni_set_driver_opt(ctx_, option, value, size);
            if (rc != 0) throw error_t(rc);
        }

        // NB: Called in destructor, no throwing
        inline void close_() noexcept
        {
            if (ctx_ == nullptr)
                return;

            // Reset the hardware, ignore error codes since this may or
            // this is called in destructor and we cannot throw.
            oni_reg_val_t reset = 1;
            oni_set_opt(ctx_, ONI_OPT_RESET, &reset, sizeof(reset));

            // Free resources
            auto rc = oni_destroy_ctx(ctx_);
            ONI_ASSERT(rc == 0);
            ctx_ = nullptr;
        }

        oni_ctx ctx_ = nullptr;
        device_map_t device_map_;
    };
}
