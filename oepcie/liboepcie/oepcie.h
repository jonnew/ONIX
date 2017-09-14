#ifndef OEPCIE_OEPCIE_H
#define OEPCIE_OEPCIE_H

#include <stddef.h>
#include <stdint.h>

#include "oedevice.h"

typedef enum oe_ctx_opt {
    OE_CONFIGSTREAMPATH,
    OE_DATASTREAMPATH,
    OE_SIGNALSTREAMPATH,
    OE_DEVICEMAP,
    OE_NUMDEVICES
} oe_ctx_opt_t;

// NB: If you add an error here, make sure to update the oe_error_string array
// in oepcie.c
typedef enum oe_error {
    OE_ESUCCESS         = 0,   // Success
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
    OE_EINVALARG        = -11, // Invalid function arguments
    OE_ECANTSETOPT      = -12, // Option cannot be set in current context state
    OE_ECOBSPACK        = -13, // Invalid COBS packet
    OE_ERETRIG          = -14, // Attempt to trigger an already triggered operation
    OE_EBUFFERSIZE      = -15, // Supplied buffer is too small
    OE_EBADDEVMAP       = -16, // Badly formated device map supplied by firmware
    OE_EBADALLOC        = -17, // Bad dynamic memory allocation
    OE_MINERRORNUM
} oe_error_t;

typedef enum oe_signal {
    OE_NULLSIG,
    OE_CONFIGNACK,             // Configuration no-acknowledgement
    OE_CONFIGWACK,             // Configuration write-acknowledgement
    OE_CONFIGRACK,             // Configuration read-acknowledgement
    OE_HEADERSTART,            // Header start marker
    OE_DEVICEINST,             // Deivce instance
    OE_HEADEREND,              // Header end marker
} oe_signal_t;

typedef enum oe_config_reg_offset {
    OE_CONFDEVID        = 0,   // Configuration device id register byte offset
    OE_CONFADDR         = 4,   // Configuration register address register byte offset
    OE_CONFVALUE        = 8,   // Configuration register value register byte offset
    OE_CONFRW           = 12,  // Configuration register read/write register byte offset
    OE_CONFTRIG         = 13,  // Configuration read/write trigger register byte offset
} oe_config_reg_offset_t;

// Context
typedef struct oe_ctx_impl *oe_ctx;

// Context manipulation
oe_ctx oe_create_ctx();
int oe_init_ctx(oe_ctx ctx);
int oe_close_ctx(oe_ctx ctx);
int oe_destroy_ctx(oe_ctx ctx);
int oe_set_ctx_opt(oe_ctx ctx, oe_ctx_opt_t option, const void* option_value, size_t option_len);
int oe_get_ctx_opt(const oe_ctx ctx, oe_ctx_opt_t option, void* option_value, size_t *option_len);

// Hardware inspection and manipulation
int oe_read_reg(const oe_ctx ctx, int32_t device_idx, uint32_t addr, uint32_t *value);
int oe_write_reg(const oe_ctx ctx, int32_t device_idx, uint32_t addr, uint32_t value);
int oe_read(const oe_ctx ctx, void *data, size_t size);

// Error inspection
void oe_error(oe_error_t err, char *str, size_t str_len);

// Static helpers
static inline int _oe_read(int fd, void* data, size_t size);
static inline int _oe_read_signal_packet(int signal_fd, uint8_t *buffer);
static int _oe_read_signal_type(int signal_fd, oe_signal_t *type);
static int _oe_read_signal_data(int signal_fd, oe_signal_t *type, void *data, size_t *size);
static int _oe_pump_signal_type(int signal_fd, const oe_signal_t type);
static int _oe_pump_signal_data(int signal_fd, const oe_signal_t type, void *data, size_t *size);
static int _oe_cobs_unstuff(uint8_t *dst, const uint8_t *src, size_t size);

#endif
