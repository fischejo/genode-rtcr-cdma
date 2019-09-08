/*
 * \brief  Driver for CDMA implementation running on FPGA of Zybo Board.
 * \author Johannes Fischer
 * \date   2018-12-16
 */


#ifndef _CDMA_DRIVER_H_
#define _CDMA_DRIVER_H_

/* Genode includes */
#include <io_mem_session/connection.h>
#include <timer_session/connection.h>
#include <base/stdint.h>
#include <spec/32bit/base/fixed_stdint.h>
#include <base/exception.h>
#include <irq_session/connection.h>
#include <base/sleep.h>
#include "cdma.h"
#include <dataspace/client.h>
#include <ram_session/capability.h>
#include <region_map/client.h>
#include <base/lock.h>

namespace Cdma {
	using namespace Genode;
    class Driver;

    struct Exception            : Genode::Exception { };
    struct Function_unsupported : Exception { };
    struct Invalid_memcpy_address : Exception { };
    struct Internal_memcpy_error : Exception { };
}


class Cdma::Driver
{
private:
    Mmio_cdma _mmio_cdma;
    bool _sg_enabled;
    bool _is_supported;
    Timer::Connection _timer;
	Lock _lock;

    static const uint32_t MAX_BTT = 0x007FFFFF; // MAX_BURST_LEN × (AXI_DATA_WIDTH/8) TODO

    // Even if the driver supports 64-bit addresses, the number of descriptors
    // is set to maxium 512. This means that it is restricted to 4 GB of
    // addressable memory.
    static const uint32_t MAX_TD_COUNT = 512;
    static const uint32_t TD_SIZE = 0x40;
    static const uint32_t TD_DS_SIZE = MAX_TD_COUNT*TD_SIZE; // 32 KiB

    Genode::Env &_env;

    // memory for descriptors (required by scather/gather mode)
    Genode::Ram_dataspace_capability _td_ds_cap;
    void *_td_ds_addr;
    Genode::uint64_t _td_phys_addr;

    // interrupts for transfer
    Genode::Irq_connection _irq;
    Genode::Signal_receiver sig_rec;
    Genode::Signal_context  sig_ctx;

    Driver(Genode::Env &env,
           Genode::addr_t cdma_address,
           Genode::uint32_t irq_number,
           bool sg_enabled);
    
    ~Driver();

    /** 
     * Internal implementation for copying memory based on the scather
     * mode. Only aligned can be copied. This function supports more than
     * `MAX_BTT` bytes of data.
     *
     * @param dst Physical destination address
     *
     * @param src Physical source address
     *
     * @param size_t Number of bytes to copy.
     */    
    void sg_memcpy(Genode::uint64_t dst, Genode::uint64_t src, Genode::size_t size);

    /** 
     * Internal implementation for copying memory based on the simple mode. Only
     * aligned can be copied. Furthmermore this function can copy up to
     * `MAX_BTT` bytes.
     *
     * @param dst Physical destination address
     *
     * @param src Physical source address
     *
     * @param btt Number of bytes to copy. Maximum of bytes is `MAX_BTT`.
     */        
    void simple_memcpy(Genode::uint64_t dst, Genode::uint64_t src, Genode::size_t btt);

    /** 
     * Internal implementation for copying memory based on the simple mode. Only
     * aligned can be copied. This function supports more than `MAX_BTT` bytes
     * of data, but internally calls `simple_memcpy`.
     *
     * @param dst Physical destination address
     *
     * @param src Physical source address
     *
     * @param size Number of bytes to copy.
     */        
    void multiple_simple_memcpy(Genode::uint64_t dst, Genode::uint64_t src, Genode::size_t size);

    /** 
     * print descriptor list
     */            
    void print_descriptor_list(Genode::uint32_t count);
    

    /** 
     * Checks, if the CDMA IP core is idle.
     *
     * @return `True`, if the core is idle, otherwise `False`.
     */                
    bool is_idle();

    /** 
     * Resets the CDMA IP core.
     *
     */
    void reset();

    /** 
     * Prints all registers which are necessary in order to program the CDMA IP
     * core.
     */                        
    void print_registers();
    
public:

    /** 
     * Singleton driver for communication with Hardware.
     *
     * @param env 
     *
     * @param cdma_address Memory mapped address of CDMA IP core
     *
     * @param irq_number Interrupt number of CDMA IP core. This interrupt is
     * triggered after completing a successful and unsuccessful transfer.
     *
     * @param sg_enabled Set `true` in order to enable the faster scather
     * mode. This need to be supported by the hardware implementation. If it is
     * not supported, the driver will automatically fallback to simple mode.
     *
     */    
    static Driver& factory(Genode::Env &env,
                           Genode::addr_t cmda_address,
                           Genode::uint32_t irq_number,
                           bool sg_enabled);

    /** 
     * Hardware accelerated copying of memory. This function supports simple and
     * scather mode of the CDMA IP core. Only aligned can be copied. Copying of
     * up to 4GB (`TD_SIZE`) is possible.
     *
     * @exception Function_unsupported The driver is not able to connect to the
     * CDMA IP core.
     *
     * @exception Invalid_memcpy_address The physical memory addres is not valid.
     *
     * @exception Internal_memcpy_address An internal error in hardware occured.
     *
     * @param dst Physical destination address
     *
     * @param src Physical source address
     *
     * @param size Number of bytes to copy.
     */            
    void memcpy(Genode::addr_t dst, Genode::addr_t src, Genode::size_t size);

    /** 
     * Checks, whether the CDMA IP core is available.
     *
     * @return `True`, if the driver found the CDMA ip core, otherwise `False`. 
     */                
    bool is_supported();
};


#endif // _CDMA_DRIVER_H_
