/*
 * \brief  Driver for CDMA implementation running on FPGA of Zybo Board.
 * \author Johannes Fischer
 * \date   2018-12-15
 */


#include <cdma/cdma.h>
#include <cdma/driver.h>

using namespace Cdma;


Driver::Driver(Genode::Env &env,
               Genode::addr_t cdma_address,
               Genode::uint32_t irq_number,
               bool sg_enabled)
    :
    _env(env),
    _mmio_cdma(env, cdma_address),
    _sg_enabled(sg_enabled),
    _timer(env),
    _td_ds_cap(env.ram().alloc(TD_DS_SIZE, Cache_attribute::UNCACHED)),
    _td_phys_addr(Genode::Dataspace_client(_td_ds_cap).phys_addr()),
    _irq(env, irq_number)

{
	#if defined(DEBUG)
    Genode::log("Allocate ", Hex(TD_DS_SIZE), "bytes for descriptors.");
    #endif

    _td_ds_addr = env.rm().attach(_td_ds_cap);

    // initialize irq and signal receiver
    _irq.sigh(sig_rec.manage(&sig_ctx));
    _irq.ack_irq();
    
    // reset CDMA.
    reset();

    // initialize and wait until CDMA is available (timeout after 500ms)
    _is_supported = false;
    for(int i = 0; i < 50; i++)
    {
        if( is_idle())
        {
            _is_supported=true;
            break;
        }
        else
        {
            _timer.msleep(10);
        }
    }

    if(! _is_supported)
    {
        Genode::error("CDMA is not available. Base Address might be wrong!");
    }
    else
    {
        // only enable Scather Gather Mode, if hardware supports it.
        bool sg_supported = _mmio_cdma.read<Mmio_cdma::CDMASR::SGIncld>();
        if(sg_enabled && !sg_supported)
        {
            _sg_enabled = false;
            Genode::warning("Scather Gather Mode is enabled, but not supported by hardware. ",
                            "Fallback to Simple Mode.");
        }
    }
}

Driver::~Driver()
{
    _env.rm().detach(_td_ds_addr);
    _env.ram().free(_td_ds_cap);
}


bool Driver::is_supported()
{
    return _is_supported;
}


void Driver::print_registers()
{
    Genode::log("CDMASR: ", Genode::Hex(_mmio_cdma.read<Mmio_cdma::CDMASR>()));
    Genode::log("CDMASR::Idle: ", _mmio_cdma.read<Mmio_cdma::CDMASR::Idle>());
    Genode::log("CDMASR::SGIncld: ", _mmio_cdma.read<Mmio_cdma::CDMASR::SGIncld>());
    Genode::log("CDMASR::DMAIntErr: ", _mmio_cdma.read<Mmio_cdma::CDMASR::DMAIntErr>());
    Genode::log("CDMASR::DMASlvErr: ", _mmio_cdma.read<Mmio_cdma::CDMASR::DMASlvErr>());
    Genode::log("CDMASR::DMADecErr: ", _mmio_cdma.read<Mmio_cdma::CDMASR::DMADecErr>());
    Genode::log("CDMASR::SGIntErr: ", _mmio_cdma.read<Mmio_cdma::CDMASR::SGIntErr>());
    Genode::log("CDMASR::SGSlvErr: ", _mmio_cdma.read<Mmio_cdma::CDMASR::SGSlvErr>());
    Genode::log("CDMASR::SGDecErr: ", _mmio_cdma.read<Mmio_cdma::CDMASR::SGDecErr>());
    Genode::log("CDMASR::IOC_Irq: ", _mmio_cdma.read<Mmio_cdma::CDMASR::IOC_Irq>());
    Genode::log("CDMASR::Dly_Irq: ", _mmio_cdma.read<Mmio_cdma::CDMASR::Dly_Irq>());
    Genode::log("CDMASR::Err_Irq: ", _mmio_cdma.read<Mmio_cdma::CDMASR::Err_Irq>());
    Genode::log("CDMASR::IRQThresholdSts: ", Hex(
                    _mmio_cdma.read<Mmio_cdma::CDMASR::IRQThresholdSts>()));
    Genode::log("CDMASR::IRQDelaySts: ", Hex(
                    _mmio_cdma.read<Mmio_cdma::CDMASR::IRQDelaySts>()));
    Genode::log("CDMACR: ", Genode::Hex(_mmio_cdma.read<Mmio_cdma::CDMACR>()));
    Genode::log("SA: ", Genode::Hex(_mmio_cdma.read<Mmio_cdma::SA>()));
    Genode::log("DA: ", Genode::Hex(_mmio_cdma.read<Mmio_cdma::DA>()));
    Genode::log("BTT: ", Genode::Hex(_mmio_cdma.read<Mmio_cdma::BTT>()));
    Genode::log("CURDESC_PNTR: ", Genode::Hex(
                    _mmio_cdma.read<Mmio_cdma::CURDESC_PNTR>() ));
    Genode::log("TAILDESC_PNTR: ", Genode::Hex(
                    _mmio_cdma.read<Mmio_cdma::TAILDESC_PNTR>() ));
}

void Driver::memcpy(Genode::addr_t dst, Genode::addr_t src, Genode::size_t size)
{
    // only support memcpy, if initializing of this driver was successful.
    if(! _is_supported)
        throw Cdma::Function_unsupported();

    _lock.lock();
    // if scather gather is enabled, use it.  Instead of using the loop
    // implemented in simple_memcpy, the CDMA IP is programmed with a loop.
    if(_sg_enabled) {
        sg_memcpy(dst, src, size);
    } else {
        multiple_simple_memcpy(dst, src, size);
    }
    _lock.unlock();
}


void Driver::print_descriptor_list(Genode::uint32_t count)
{

    for(uint32_t i = 0; i < count; i++)
    {
        uint64_t td_addr =  (uint64_t)_td_ds_addr + i*0x40;
        Genode::log("td[", i,"]::NXTDESC_PNTR:    ", Hex(*(uint32_t *)(td_addr+0x00))); 
        Genode::log("td[", i,"]::NXTDESC_PNTR_MSB ", Hex(*(uint32_t *)(td_addr+0x04)));       
        Genode::log("td[", i,"]::SA:              ", Hex(*(uint32_t *)(td_addr+0x08)));
        Genode::log("td[", i,"]::SA_MSB           ", Hex(*(uint32_t *)(td_addr+0x0c)));        
        Genode::log("td[", i,"]::DA:              ", Hex(*(uint32_t *)(td_addr+0x10)));
        Genode::log("td[", i,"]::DA_MSB:          ", Hex(*(uint32_t *)(td_addr+0x1c)));        
        Genode::log("td[", i,"]::CONTROL:         ", Hex(*(uint32_t *)(td_addr+0x18)));
        Genode::log("td[", i,"]::STATUS:          ", Hex(*(uint32_t *)(td_addr+0x1c)));
    }
}



void Driver::sg_memcpy(Genode::uint64_t dst, Genode::uint64_t src, Genode::size_t size)
{
	#if defined(DEBUG) || defined(VERBOSE)
    Genode::log("sg_memcpy(", Hex(dst), ", ", Hex(src), ", ", Hex(size), ")");    
    #endif

    // bytes to copy
    Genode::size_t btt;

    // offset from where copy starts
    Genode::uint64_t offset = 0;

    // points to first processed td in physical memory
    Genode::uint64_t td_head_phys_addr = _td_phys_addr;

    // points to the previous td
    Genode::uint64_t td_previous_phys_addr = td_head_phys_addr;

    // points to the start of the first td in attached dataspace.
    Genode::uint64_t td_head_ds_addr = (uint64_t)_td_ds_addr;

    Genode::uint32_t td_counter = 0;
    do
    {
        if(size >= MAX_BTT)
        {
            btt = MAX_BTT;
            size -= MAX_BTT;
        } else {
            btt = size;
            size = 0;
        }
        
        // create descriptor in td dataspace and insert it at the start of the td list.
        *((int *)(td_head_ds_addr+0x00)) = (uint32_t) td_previous_phys_addr;
        *((int *)(td_head_ds_addr+0x04)) = (uint32_t) (td_previous_phys_addr << 32);
        *((int *)(td_head_ds_addr+0x08)) = (uint32_t) (src + offset); // src
        *((int *)(td_head_ds_addr+0x0c)) = (uint32_t) ((src + offset) << 32);
        *((int *)(td_head_ds_addr+0x10)) = (uint32_t) (dst + offset); // dst
        *((int *)(td_head_ds_addr+0x14)) = (uint32_t) ((dst + offset) << 32);
        *((int *)(td_head_ds_addr+0x18)) = (uint32_t) btt; // bytes to transfer.
        *((int *)(td_head_ds_addr+0x1c)) = 0x00000000; // status filled by device

        td_head_ds_addr += TD_SIZE; // prepare next descriptor position for head
        td_previous_phys_addr = td_head_phys_addr;
        td_head_phys_addr += TD_SIZE; // prepare next descriptor position for head
        offset += btt;
        td_counter++;
    } while(size > 0);

    
    // enable interrupts for notifying complete transfer
    _mmio_cdma.write<Mmio_cdma::CDMACR::IOC_IrqEn>(1);
    _mmio_cdma.write<Mmio_cdma::CDMACR::Err_IrqEn>(1);
    _mmio_cdma.write<Mmio_cdma::CDMACR::IRQThreshold>(td_counter);
    
    // initialize Scather Gather Mode by setting it to zero and than to one.
    _mmio_cdma.write<Mmio_cdma::CDMACR::SGMode>(0);
    _mmio_cdma.write<Mmio_cdma::CDMACR::SGMode>(1);

    
    // start td processing with head of td list
    _mmio_cdma.write<Mmio_cdma::CURDESC_PNTR>((uint32_t) td_previous_phys_addr);
    _mmio_cdma.write<Mmio_cdma::CURDESC_PNTR_MSB>((uint32_t) (td_previous_phys_addr << 32));

	#if defined(DEBUG)
    Genode::log("Registers before memcpy:");
    print_registers();
    #endif
    
    // processing is finished, when reaching the tail of td list. The first
    // element of the descriptor list is always written at the beginning of the
    // dataspace. Therefore it is the last processed element in the chain. The
    // physical start of the dataspace is the TAILDESC pointer.
    // Writing this register will start the copying.
    _mmio_cdma.write<Mmio_cdma::TAILDESC_PNTR>((uint32_t) _td_phys_addr);
    _mmio_cdma.write<Mmio_cdma::TAILDESC_PNTR_MSB>((uint32_t) (_td_phys_addr << 32));    

    // waiting for interrupt
    sig_rec.wait_for_signal();

	#if defined(DEBUG)
    Genode::log("Registers after memcpy:");
    print_registers();
    print_descriptor_list(td_counter);    
    #endif
    
    // got interrupt, because the CDMA IP core successfully copied    
    if(_mmio_cdma.read<Mmio_cdma::CDMASR::IOC_Irq>())
    {
        _mmio_cdma.write<Mmio_cdma::CDMASR::IOC_Irq>(1); // clear interrupt
        _irq.ack_irq();
    }
    // otherwise it is an error interrupt
    else if(_mmio_cdma.read<Mmio_cdma::CDMASR::Err_Irq>())
    {
        _mmio_cdma.write<Mmio_cdma::CDMASR::Err_Irq>(1); // clear interrupt
        _irq.ack_irq();

        if(_mmio_cdma.read<Mmio_cdma::CDMASR::SGIntErr>())
        {
            Genode::error("A internal error has been encountered by the DataMover ",
                          "on the data transport channel.");
            throw Cdma::Internal_memcpy_error();
        }

        if(_mmio_cdma.read<Mmio_cdma::CDMASR::SGSlvErr>())
        {
            Genode::error("AXI slave error response has been received by the ",
                          "AXI DataMover during an AXI transfer.");
            throw Cdma::Internal_memcpy_error();
        }

        if(_mmio_cdma.read<Mmio_cdma::CDMASR::SGDecErr>())
        {
            Genode::error("An AXI decode error has been received by the AXI DataMover. ",
                          "This error occurs if the DataMover issues an address request ",
                          "to an invalid location.");
            throw Cdma::Invalid_memcpy_address();
        }
    } else {
        // this should never ever happen.
        Genode::error("Got interrupt from unknown source.");
        throw Cdma::Internal_memcpy_error();            
    }
}


void Driver::simple_memcpy(Genode::uint64_t dst, Genode::uint64_t src, Genode::size_t btt)
{
	#if defined(DEBUG) || defined(VERBOSE)
    Genode::log("simple_memcpy(", Hex(dst), ", ", Hex(src), ", ", Hex(btt), ")");
    #endif
    
    // reset the hardware before every programming
    reset();

    // enable interrupts for notifying complete transfer    
    _mmio_cdma.write<Mmio_cdma::CDMACR::IOC_IrqEn>(1);
    _mmio_cdma.write<Mmio_cdma::CDMACR::Err_IrqEn>(1);

    // write source address
    _mmio_cdma.write<Mmio_cdma::SA>((uint32_t) src);
    _mmio_cdma.write<Mmio_cdma::SA_MSB>((uint32_t) (src<<32));    

    // write destination address
    _mmio_cdma.write<Mmio_cdma::DA>((uint32_t) dst);
    _mmio_cdma.write<Mmio_cdma::DA_MSB>((uint32_t) (dst<<32));    

    // write bytes to transfer. This starts the transfer.
    _mmio_cdma.write<Mmio_cdma::BTT>(btt);

    // waiting for interrupt
    sig_rec.wait_for_signal();

	#if defined(DEBUG)
    Genode::log("Registers after simple_memcpy:");
    print_registers();
    #endif
    
    // got interrupt, because the CDMA IP core successfully copied
    if(_mmio_cdma.read<Mmio_cdma::CDMASR::IOC_Irq>())
    {
        _mmio_cdma.write<Mmio_cdma::CDMASR::IOC_Irq>(1); // clear interrupt
        _irq.ack_irq();        
    }
    // otherwise it is an error interrupt
    else if(_mmio_cdma.read<Mmio_cdma::CDMASR::Err_Irq>())
    {
        _mmio_cdma.write<Mmio_cdma::CDMASR::Err_Irq>(1); // clear interrupt
        _irq.ack_irq();        

        // is it an internal error?
        if(_mmio_cdma.read<Mmio_cdma::CDMASR::DMAIntErr>())
        {
            Genode::error("A internal error has been encountered by the DataMover ",
                          "on the data transport channel.");
            throw Cdma::Internal_memcpy_error();
        }

        // is it a slave error?
        if(_mmio_cdma.read<Mmio_cdma::CDMASR::DMASlvErr>())
        {
            Genode::error("AXI slave error response has been received by the ",
                          "AXI DataMover during an AXI transfer.");
            throw Cdma::Internal_memcpy_error();
        }

        // is it a decode error?
        if(_mmio_cdma.read<Mmio_cdma::CDMASR::DMADecErr>())
        {
            Genode::error("An AXI decode error has been received by the AXI DataMover. ",
                          "This error occurs if the DataMover issues an address request ",
                          "to an invalid location.");
            throw Cdma::Invalid_memcpy_address();
        }
    } else {
        // this should never ever happen.
        Genode::error("Got interrupt from unknown source.");
        throw Cdma::Internal_memcpy_error();            
    }
    
}


void Driver::multiple_simple_memcpy(Genode::uint64_t dst, Genode::uint64_t src, Genode::size_t size)
{
	#if defined(DEBUG) || defined(VERBOSE)
    Genode::log("multiple_simple_memcpy(", Hex(dst), ", ", Hex(src), ", ", Hex(size), ")");
    #endif
    
    Genode::size_t btt;  // bytes to copy
    Genode::uint64_t offset = 0;
    do
    {
        if(size >= MAX_BTT)
        {
            btt = MAX_BTT;
            size -= MAX_BTT;
        } else {
            btt = size;
            size = 0;
        }
        simple_memcpy(dst+offset, src+offset, btt);
        offset += btt;
    } while(size > 0);
}


bool Driver::is_idle()
{
    return _mmio_cdma.read<Mmio_cdma::CDMASR::Idle>();
}

void Driver::reset()
{
    _mmio_cdma.write<Mmio_cdma::CDMACR::Reset>(0x1);
}


Driver& Driver::factory(Genode::Env &env,
                        Genode::addr_t cdma_address,
                        Genode::uint32_t irq_number,
                        bool sg_enabled)
{
    static Driver driver(env, cdma_address, irq_number, sg_enabled);
    return driver;
}
