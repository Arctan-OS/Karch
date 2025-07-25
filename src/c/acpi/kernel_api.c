#include "arch/io/port.h"
#include "uacpi/status.h"
#include <uacpi/kernel_api.h>
#include <global.h>
#include <mm/allocator.h>
#include <arch/pager.h>
#include <lib/atomics.h>
#include <mp/scheduler.h>
#include <lib/util.h>
#include <arch/pci/pci.h>

struct kernel_io_handle {
	uint64_t base;
	size_t len;
};

/*
 * Convenience initialization/deinitialization hooks that will be called by
 * uACPI automatically when appropriate if compiled-in.
 */
#ifdef UACPI_KERNEL_INITIALIZATION
/*
 * This API is invoked for each initialization level so that appropriate parts
 * of the host kernel and/or glue code can be initialized at different stages.
 *
 * uACPI API that triggers calls to uacpi_kernel_initialize and the respective
 * 'current_init_lvl' passed to the hook at that stage:
 * 1. uacpi_initialize() -> UACPI_INIT_LEVEL_EARLY
 * 2. uacpi_namespace_load() -> UACPI_INIT_LEVEL_SUBSYSTEM_INITIALIZED
 * 3. (start of) uacpi_namespace_initialize() -> UACPI_INIT_LEVEL_NAMESPACE_LOADED
 * 4. (end of) uacpi_namespace_initialize() -> UACPI_INIT_LEVEL_NAMESPACE_INITIALIZED
 */
uacpi_status uacpi_kernel_initialize(uacpi_init_level current_init_lvl) { return UACPI_STATUS_OK; }
void uacpi_kernel_deinitialize(void) { return UACPI_STATUS_OK; }
#endif

// Returns the PHYSICAL address of the RSDP structure via *out_rsdp_address.
uacpi_status uacpi_kernel_get_rsdp(uacpi_phys_addr *out_rsdp_address) {
	*out_rsdp_address = Arc_KernelMeta->rsdp;
	return UACPI_STATUS_OK;
}

/*
 * Raw IO API, this is only used for accessing verified data from
 * "safe" code (aka not indirectly invoked by the AML interpreter),
 * e.g. programming FADT & FACS registers.
 *
 * NOTE:
 * 'byte_width' is ALWAYS one of 1, 2, 4, 8. You are NOT allowed to implement
 * this in terms of memcpy, as hardware expects accesses to be of the EXACT
 * width.
 * -------------------------------------------------------------------------
 */
uacpi_status uacpi_kernel_raw_memory_read(uacpi_phys_addr address, uacpi_u8 byte_width, uacpi_u64 *out_value) {
	uintptr_t virtual = ARC_PHYS_TO_HHDM(address);
	switch (byte_width) {
		case 1: {
			*(uint8_t *)out_value = *(uint8_t *)virtual;
			return UACPI_STATUS_OK;
		}

		case 2: {
			*(uint16_t *)out_value = *(uint16_t *)virtual;
			return UACPI_STATUS_OK;
		}

		case 4: {
			*(uint16_t *)out_value = *(uint32_t *)virtual;
			return UACPI_STATUS_OK;
		}

		case 8: {
			*(uint64_t *)out_value = *(uint64_t *)virtual;
			return UACPI_STATUS_OK;
		}

		default: {
			return UACPI_STATUS_DENIED;
		}
	}
}
uacpi_status uacpi_kernel_raw_memory_write(uacpi_phys_addr address, uacpi_u8 byte_width, uacpi_u64 in_value) {
	uintptr_t virtual = ARC_PHYS_TO_HHDM(address);
	switch (byte_width) {
		case 1: {
			*(uint8_t *)virtual = *(uint8_t *)in_value;
			return UACPI_STATUS_OK;
		}

		case 2: {
			*(uint16_t *)virtual = *(uint16_t *)in_value;
			return UACPI_STATUS_OK;
		}

		case 4: {
			*(uint32_t *)virtual = *(uint32_t *)in_value;
			return UACPI_STATUS_OK;
		}

		case 8: {
			*(uint64_t *)virtual = *(uint64_t *)in_value;
			return UACPI_STATUS_OK;
		}

		default: {
			return UACPI_STATUS_DENIED;
		}
	}
}

/*
 * NOTE:
 * 'byte_width' is ALWAYS one of 1, 2, 4. You are NOT allowed to break e.g. a
 * 4-byte access into four 1-byte accesses. Hardware ALWAYS expects accesses to
 * be of the exact width.
 */
uacpi_status uacpi_kernel_raw_io_read(uacpi_io_addr address, uacpi_u8 byte_width, uacpi_u64 *out_value) {
	(void)address;
	(void)out_value;

	switch (byte_width) {
		case 1: {
			*out_value = inb(address);
			return UACPI_STATUS_OK;
		}

		case 2: {
			*out_value = inw(address);
			return UACPI_STATUS_OK;
		}

		case 4: {
			*out_value = ind(address);
			return UACPI_STATUS_OK;
		}

		case 8: {
			*out_value = inq(address);
			return UACPI_STATUS_OK;
		}

		default: {
			return UACPI_STATUS_DENIED;
		}
	}
}

uacpi_status uacpi_kernel_raw_io_write(uacpi_io_addr address, uacpi_u8 byte_width, uacpi_u64 in_value) {
	(void)address;
	(void)in_value;

	switch (byte_width) {
		case 1: {
			outb(address, in_value);
			return UACPI_STATUS_OK;
		}

		case 2: {
			outw(address, in_value);
			return UACPI_STATUS_OK;
		}

		case 4: {
			outd(address, in_value);
			return UACPI_STATUS_OK;
		}

		case 8: {
			outq(address, in_value);
			return UACPI_STATUS_OK;
		}

		default: {
			return UACPI_STATUS_DENIED;
		}
	}
}
// -------------------------------------------------------------------------

/*
 * NOTE:
 * 'byte_width' is ALWAYS one of 1, 2, 4. Since PCI registers are 32 bits wide
 * this must be able to handle e.g. a 1-byte access by reading at the nearest
 * 4-byte aligned offset below, then masking the value to select the target
 * byte.
 */
uacpi_status uacpi_kernel_pci_read(uacpi_pci_address *address, uacpi_size offset, uacpi_u8 byte_width, uacpi_u64 *value) {
	uint32_t read = pci_read(address->segment, address->bus, address->device, address->function, offset);

	switch (byte_width) {
		case 1: {
			*value = read & UINT8_MAX;
			return UACPI_STATUS_OK;
		}
		case 2: {
			*value = read & UINT16_MAX;
			return UACPI_STATUS_OK;
		}
		case 4: {
			*value = read;
			return UACPI_STATUS_OK;
		}
	}

	return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_write(uacpi_pci_address *address, uacpi_size offset, uacpi_u8 byte_width, uacpi_u64 value) {
	int r = pci_write(address->segment, address->bus, address->device, address->function, offset, byte_width, value);
	return (r == 0 ? UACPI_STATUS_OK : UACPI_STATUS_DENIED);
}

/*
 * Map a SystemIO address at [base, base + len) and return a kernel-implemented
 * handle that can be used for reading and writing the IO range.
 */
uacpi_status uacpi_kernel_io_map(uacpi_io_addr base, uacpi_size len, uacpi_handle *out_handle) {
	(void)len;
	ARC_DEBUG(WARN, "IO MAP\n");

	struct kernel_io_handle *handle = alloc(sizeof(*handle));

	if (handle == NULL) {
		ARC_DEBUG(ERR, "Failed to create handle\n");
		return UACPI_STATUS_DENIED;
	}

	handle->base = base;
	handle->len = len;
	
	*out_handle = handle;

	return UACPI_STATUS_OK;
}

void uacpi_kernel_io_unmap(uacpi_handle handle) {
	if (handle == NULL) {
		ARC_DEBUG(ERR, "Failed to unmap handle\n");
		return;
	}
	
	free(handle);

	return;
}

/*
 * Read/Write the IO range mapped via uacpi_kernel_io_map
 * at a 0-based 'offset' within the range.
 *
 * NOTE:
 * 'byte_width' is ALWAYS one of 1, 2, 4. You are NOT allowed to break e.g. a
 * 4-byte access into four 1-byte accesses. Hardware ALWAYS expects accesses to
 * be of the exact width.
 */
uacpi_status uacpi_kernel_io_read(uacpi_handle handle, uacpi_size offset, uacpi_u8 byte_width, uacpi_u64 *value) {
	(void)handle;
	(void)offset;
	(void)byte_width;
	(void)value;
	ARC_DEBUG(INFO, "IO RANGE READ\n");
	return UACPI_STATUS_OK;
}
uacpi_status uacpi_kernel_io_write(uacpi_handle handle, uacpi_size offset, uacpi_u8 byte_width, uacpi_u64 value) {
	(void)handle;
	(void)offset;
	(void)byte_width;
	(void)value;
	ARC_DEBUG(INFO, "IO RANGE WRITE\n");
	return UACPI_STATUS_OK;
}

void *uacpi_kernel_map(uacpi_phys_addr addr, uacpi_size len) {
	(void)len;
	return (void *)ARC_PHYS_TO_HHDM(addr);
}

void uacpi_kernel_unmap(void *addr, uacpi_size len) {
	(void)addr;
	(void)len;
}

/*
 * Allocate a block of memory of 'size' bytes.
 * The contents of the allocated memory are unspecified.
 */
void *uacpi_kernel_alloc(uacpi_size size) {
	void *r = alloc(size);
	if (r == NULL) {
		ARC_DEBUG(INFO, "Allocating %lu (%p)\n", size, r);
	}
	return r;
}

/*
 * Allocate a block of memory of 'count' * 'size' bytes.
 * The returned memory block is expected to be zero-filled.
 */
void *uacpi_kernel_calloc(uacpi_size count, uacpi_size size) {
	void *r = calloc(size, count);
	if (r == NULL) {
		ARC_DEBUG(INFO, "Allocating %lu %lu (%p)\n", size, count, r);
	}
	memset(r, 0, size * count);
	return r;
}

/*
 * Free a previously allocated memory block.
 *
 * 'mem' might be a NULL pointer. In this case, the call is assumed to be a
 * no-op.
 *
 * An optionally enabled 'size_hint' parameter contains the size of the original
 * allocation. Note that in some scenarios this incurs additional cost to
 * calculate the object size.
 */
#ifndef UACPI_SIZED_FREES
void uacpi_kernel_free(void *mem) {
	if (mem == NULL) {
		return;
	}

	if (free(mem) == 0) {
		ARC_DEBUG(ERR, "Freeing %p\n", mem);
	}
}
#else
void uacpi_kernel_free(void *mem, uacpi_size size_hint) { return UACPI_STATUS_OK; }
#endif

#ifndef UACPI_FORMATTED_LOGGING
void uacpi_kernel_log(uacpi_log_level level, const uacpi_char* fmt) {
	switch (level) {
		case UACPI_LOG_INFO: {
			ARC_DEBUG(INFO, "[uACPI]: %s", fmt);
			break;
		}
		case UACPI_LOG_TRACE: {
			ARC_DEBUG(INFO, "[uACPI TRACE]: %s", fmt);
			break;
		}
		case UACPI_LOG_WARN: {
			ARC_DEBUG(WARN, "[uACPI]: %s", fmt);
			break;
		}
		case UACPI_LOG_DEBUG: {
			ARC_DEBUG(INFO, "[uACPI DEBUG]: %s", fmt);
			break;
		}
		case UACPI_LOG_ERROR: {
			ARC_DEBUG(ERR, "[uACPI]: %s", fmt);
			break;
		}
	}
	return;
}
#else
UACPI_PRINTF_DECL(2, 3)
void uacpi_kernel_log(uacpi_log_level, const uacpi_char*, ...) { return UACPI_STATUS_OK; }
void uacpi_kernel_vlog(uacpi_log_level, const uacpi_char*, uacpi_va_list) { return UACPI_STATUS_OK; }
#endif

/*
 * Returns the number of 100 nanosecond ticks elapsed since boot,
 * strictly monotonic.
 */
uacpi_u64 uacpi_kernel_get_ticks(void) {
	ARC_DEBUG(INFO, "There are no ticks\n");
	return UACPI_STATUS_OK;
}

/*
 * Spin for N microseconds.
 */
void uacpi_kernel_stall(uacpi_u8 usec) {
	(void)usec;
	ARC_DEBUG(INFO, "Not stalling\n");
	return;
}

/*
 * Sleep for N milliseconds.
 */
void uacpi_kernel_sleep(uacpi_u64 msec) {
	(void)msec;
	ARC_DEBUG(INFO, "Not sleeping\n");
	return;
}

/*
 * Create/free an opaque non-recursive kernel mutex object.
 */
uacpi_handle uacpi_kernel_create_mutex(void) {
	ARC_GenericMutex *mutex = NULL;

	if (init_mutex(&mutex) != 0) {
		ARC_DEBUG(ERR, "Failed to allocate mutex\n");
		return NULL;
	}

	return mutex;
}

void uacpi_kernel_free_mutex(uacpi_handle handle) {
	if (uninit_mutex((ARC_GenericMutex *)handle) != 0) {
		ARC_DEBUG(ERR, "Failed to destroy mutex %p\n", handle);
	}
}

/*
 * Create/free an opaque kernel (semaphore-like) event object.
 */
uacpi_handle uacpi_kernel_create_event(void) {
	ARC_DEBUG(INFO, "Creating event\n");
	return alloc(16);
}
void uacpi_kernel_free_event(uacpi_handle handle) {
	ARC_DEBUG(INFO, "Freeing event\n");
	free(handle);
	return;
}

/*
 * Returns a unique identifier of the currently executing thread.
 *
 * The returned thread id cannot be UACPI_THREAD_ID_NONE.
 */
uacpi_thread_id uacpi_kernel_get_thread_id(void) {
	return (uacpi_thread_id)sched_get_current_tid();
}

/*
 * Try to acquire the mutex with a millisecond timeout.
 * A timeout value of 0xFFFF implies infinite wait.
 */
uacpi_bool uacpi_kernel_acquire_mutex(uacpi_handle handle, uacpi_u16 timeout) {
	(void)timeout;
	return mutex_lock((ARC_GenericMutex *)handle) == 0;
}

void uacpi_kernel_release_mutex(uacpi_handle handle) {
	mutex_unlock((ARC_GenericMutex *)handle);
}

/*
 * Try to wait for an event (counter > 0) with a millisecond timeout.
 * A timeout value of 0xFFFF implies infinite wait.
 *
 * The internal counter is decremented by 1 if wait was successful.
 *
 * A successful wait is indicated by returning UACPI_TRUE.
 */
uacpi_bool uacpi_kernel_wait_for_event(uacpi_handle handle, uacpi_u16 timeout) {
	(void)handle;
	(void)timeout;
	ARC_DEBUG(WARN, "Waiting for event\n");
	return 0;
}

/*
 * Signal the event object by incrementing its internal counter by 1.
 *
 * This function may be used in interrupt contexts.
 */
void uacpi_kernel_signal_event(uacpi_handle handle) {
	(void)handle;
	ARC_DEBUG(WARN, "Signalling\n");
	return;
}

/*
 * Reset the event counter to 0.
 */
void uacpi_kernel_reset_event(uacpi_handle handle) {
	(void)handle;
	ARC_DEBUG(WARN, "Reseting event\n");
	return;
}

/*
 * Handle a firmware request.
 *
 * Currently either a Breakpoint or Fatal operators.
 */
uacpi_status uacpi_kernel_handle_firmware_request(uacpi_firmware_request *req) {
	(void)req;
	ARC_DEBUG(INFO, "Handling firmware request\n");
	return UACPI_STATUS_OK;
}

/*
 * Install an interrupt handler at 'irq', 'ctx' is passed to the provided
 * handler for every invocation.
 *
 * 'out_irq_handle' is set to a kernel-implemented value that can be used to
 * refer to this handler from other API.
 */
uacpi_status uacpi_kernel_install_interrupt_handler(uacpi_u32 irq, uacpi_interrupt_handler handler, uacpi_handle ctx, uacpi_handle *out_irq_handle) {
	(void)handler;
	(void)ctx;
	(void)out_irq_handle;
	ARC_DEBUG(INFO, "Installing IRQ handler for %d\n", irq);
	return UACPI_STATUS_OK;
}

/*
 * Uninstall an interrupt handler. 'irq_handle' is the value returned via
 * 'out_irq_handle' during installation.
 */
uacpi_status uacpi_kernel_uninstall_interrupt_handler(uacpi_interrupt_handler handler, uacpi_handle irq_handle) {
	(void)handler;
	(void)irq_handle;
	ARC_DEBUG(INFO, "Uninstalling IRQ\n");
	return UACPI_STATUS_OK;
}

/*
 * Create/free a kernel spinlock object.
 *
 * Unlike other types of locks, spinlocks may be used in interrupt contexts.
 */
uacpi_handle uacpi_kernel_create_spinlock(void) {
	ARC_GenericSpinlock *spinlock = NULL;

	if (init_spinlock(&spinlock) != 0) {
		ARC_DEBUG(ERR, "Failed to create spinlock\n");
	}

	return spinlock;
}

void uacpi_kernel_free_spinlock(uacpi_handle handle) {
	if (uninit_spinlock((ARC_GenericSpinlock *)handle) != 0) {
		ARC_DEBUG(ERR, "Failed to free spinlock\n");
	}
}

/*
 * Lock/unlock helpers for spinlocks.
 *
 * These are expected to disable interrupts, returning the previous state of cpu
 * flags, that can be used to possibly re-enable interrupts if they were enabled
 * before.
 *
 * Note that lock is infalliable.
 */
uacpi_cpu_flags uacpi_kernel_lock_spinlock(uacpi_handle handle) {
	ARC_DEBUG(ERR, "Disable interrupts, RFLAGS\n");
	if (spinlock_lock((ARC_GenericSpinlock *)handle) != 0) {
		ARC_DEBUG(ERR, "Failed to lock spinlock\n");
	}

	// TODO: RFLAGS need to be returned here, figure it out
	return 0;
}

void uacpi_kernel_unlock_spinlock(uacpi_handle handle, uacpi_cpu_flags flags) {
	(void)flags;
	if (spinlock_unlock((ARC_GenericSpinlock *)handle) != 0) {
		ARC_DEBUG(ERR, "Failed to unlock spinlock\n");
	}
}

/*
 * Schedules deferred work for execution.
 * Might be invoked from an interrupt context.
 */
uacpi_status uacpi_kernel_schedule_work(uacpi_work_type type, uacpi_work_handler handler, uacpi_handle ctx) {
	(void)type;
	(void)handler;
	(void)ctx;
	ARC_DEBUG(INFO, "Schedule work\n");
	return UACPI_STATUS_OK;
}

/*
 * Blocks until all scheduled work is complete and the work queue becomes empty.
 */
uacpi_status uacpi_kernel_wait_for_work_completion(void) { return UACPI_STATUS_OK; }
