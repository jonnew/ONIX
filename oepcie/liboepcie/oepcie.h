#ifndef OEPCIE_H
#define OEPCIE_H

typedef enum oe_ctx_opt {
    OE_HEADERSTREAMPATH,
    OE_CONFIGSTREAMPATH,
    OE_DATASTREAMPATH,
    OE_SIGNALSTREAMPATH,
    OE_DEVIDS,
    OE_DEVREADOFFSETS,
    OE_NUMDEVICES
} oe_ctx_opt_t;

typedef enum oe_signal {
    OE_NULLSIG,
    OE_CONFIGNACK,            // Configuration no-acknowledgement
    OE_CONFIGWACK,            // Configuration write-acknowledgement
    OE_CONFIGRACK,            // Configuration read-acknowledgement
    OE_CONFIGWSTART,          // Configuration write-start
    OE_CONFIGRSTART,          // Configuration read-start

} oe_signal_t;

typedef enum oe_error {
    OE_EPATHINVALID     = -1,  // Invalid stream path, fail on open
    OE_EREINITCTX       = -2,  // Double initialization attempt
    OE_EDEVID           = -3,  // Invalid device ID on init or reg op
    OE_EREADFAILURE     = -4,  // Failure to read from a stream/register
    OE_EWRITEFAILURE    = -5,  // Failure to write to a stream/register
    OE_ENULLCTX         = -6,  // Attempt to call function w null ctx
    OE_ESEEKFAILURE     = -7,  // Failure to seek on stream
    OE_ENOTINIT         = -8,  // Invalid operation on non-initialized ctx
    OE_EDEVIDX          = -9,  // Invalid device index
    OE_EINVALOPT        = -10, // Invalid context option
    OE_EINVALARG        = -11, // Invalid function arguements
    OE_ECANTSETOPT      = -12, // Option cannot be set in current context state
} oe_error_t;

// Context
typedef struct oe_ctx_impl *oe_ctx;
oe_ctx oe_create_ctx();
int oe_init_ctx(oe_ctx ctx);
int oe_close_ctx(oe_ctx ctx);
int oe_destroy_ctx(oe_ctx ctx);
int oe_set_ctx_opt(oe_ctx ctx, int option, const void* option_value, size_t option_len);
int oe_get_ctx_opt(const oe_ctx ctx, int option, void* option_value, size_t *option_len);

// Hardware manipulation
int oe_write_reg(const oe_ctx ctx, int device_idx, int addr, int value, int *ack);
int oe_read_reg(const oe_ctx ctx, int device_idx, int addr, int* value, int *ack);
int oe_read(const oe_ctx ctx, void *data, size_t size);
// int oe_write(const oe_ctx* ctx, void* data, size_t size);

// Helpers
static inline int oe_all_read(int fd, void* data, size_t size);
static inline int oe_signal_read(const oe_ctx ctx, int *sig);

#endif
