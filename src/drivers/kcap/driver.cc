 /*
 * \brief  Driver for harware-based capability parsing running on the FPGA of the Zybo Board.
 * \author Johannes Fischer
 * \date   2019-01-09
 */


#include <kcap/kcap.h>
#include <kcap/driver.h>

/* kernel capabilities includes */
#include <base/internal/cap_map.h>

using namespace Kcap;


Driver::Driver(Genode::Env &env,
               Genode::addr_t dma_address,
               Genode::uint32_t mm2s_irq_number,
               Genode::uint32_t s2mm_irq_number,
               Genode::addr_t gpio_address)
    :
    _mmio_dma(env, dma_address),
    _mmio_gpio(env, gpio_address),
    _mm2s_irq(env, mm2s_irq_number),
    _s2mm_irq(env, s2mm_irq_number)
{
    // initialize irq and signal receiver for dma mm2s transfer
    _mm2s_irq.sigh(_mm2s_sig_rec.manage(&_mm2s_sig_ctx));
    _mm2s_irq.ack_irq();

    // initialize irq and signal receiver for dma s2mm transfer    
    _s2mm_irq.sigh(_s2mm_sig_rec.manage(&_s2mm_sig_ctx));
    _s2mm_irq.ack_irq();
}


size_t Driver::parse_kcap_map(Genode::addr_t dst, Genode::addr_t src)
{
	#if defined(DEBUG) || defined(VERBOSE)
    Genode::log("parse_kcap_map(", dst, src, ")");
    #endif
    
    // set run/stop bit
    _mmio_dma.write<Mmio_dma::MM2S_DMACR::RS>(1);

    // enable interrupts for error and complete transfer    
    _mmio_dma.write<Mmio_dma::MM2S_DMACR::IOC_IrqEn>(1);
    _mmio_dma.write<Mmio_dma::MM2S_DMACR::Err_IrqEn>(1);

    // write source address
    _mmio_dma.write<Mmio_dma::MM2S_SA>((uint32_t) src);

    // write length (this activates the transfer)
    _mmio_dma.write<Mmio_dma::MM2S_LENGTH>(
        MAX_CAP_INDEX*sizeof(Genode::Cap_index));


	#if defined(DEBUG)
    Genode::log("Start MM2S transfer:");
    print_registers();       
    #endif

    
    // waiting for interrupt (complete or failure)
    _mm2s_sig_rec.wait_for_signal();
    _mm2s_irq.ack_irq();

    // got interrupt, because the DMA IP core successfully copied
    if(_mmio_dma.read<Mmio_dma::MM2S_DMASR::IOC_Irq>())
    {
        _mmio_dma.write<Mmio_dma::MM2S_DMASR::IOC_Irq>(1); // clear interrupt
    }
    // otherwise it is an error interrupt
    else if(_mmio_dma.read<Mmio_dma::MM2S_DMASR::Err_Irq>())
    {
        _mmio_dma.write<Mmio_dma::MM2S_DMASR::Err_Irq>(1); // clear interrupt

        // is it an internal error?
        if(_mmio_dma.read<Mmio_dma::MM2S_DMASR::DMAIntErr>())
        {
            Genode::error("An internal error has been encountered by the DataMover ",
                          "on the data transport channel during MM2S transfer.");
            print_registers();
            throw Kcap::Internal_error();
        }

        // is it a slave error?
        if(_mmio_dma.read<Mmio_dma::MM2S_DMASR::DMASlvErr>())
        {
            Genode::error("An AXI slave error response has been received by the ",
                          "AXI DataMover during an MM2S transfer.");
            print_registers();
            throw Kcap::Internal_error();
        }

        // is it a decode error?
        if(_mmio_dma.read<Mmio_dma::MM2S_DMASR::DMADecErr>())
        {
            Genode::error("An AXI decode error has been received by the AXI DataMover during MM2S transfer. ",
                          "This error occurs if the DataMover issues an address request ",
                          "to an invalid location.");
            print_registers();
            throw Kcap::Invalid_address();
        }
    } else {
        // this should never ever happen.
        Genode::error("Got interrupt from unknown source during MM2S transfer.");
        print_registers();
        throw Kcap::Internal_error();
    }

    // read number of parsed valid capabilities.
    size_t valid_kcap_count = _mmio_gpio.read<Mmio_gpio::VALID_KCAP_COUNT>();

    // if no valid capabilities are found, the second transfer is not necessary
    // anymore.
    if(valid_kcap_count == 0) {
		#if defined(DEBUG) || defined(VERBOSE)
        Genode::log("valid_kcap_count=",valid_kcap_count);
		#endif
        return 0;
    }
    
    // set run/stop bit
    _mmio_dma.write<Mmio_dma::S2MM_DMACR::RS>(1);

    // enable interrupts for error and complete transfer    
    _mmio_dma.write<Mmio_dma::S2MM_DMACR::IOC_IrqEn>(1);
    _mmio_dma.write<Mmio_dma::S2MM_DMACR::Err_IrqEn>(1);    
    
    // write destination address. This is the physical address of the dataspace
    // allocated by the driver.
    _mmio_dma.write<Mmio_dma::S2MM_DA>((uint32_t) dst);

    // write length (this activates the transfer)
    _mmio_dma.write<Mmio_dma::S2MM_LENGTH>(
        sizeof(Kcap::Parsed_cap_index)*valid_kcap_count);

	#if defined(DEBUG)
   	Genode::log("Start S2MM transfer:");
   	print_registers();       
	#endif    

    // waiting for interrupt (complete or failure)
    _s2mm_sig_rec.wait_for_signal();
    _s2mm_irq.ack_irq();

    // got interrupt, because the DMA IP core successfully copied
    if(_mmio_dma.read<Mmio_dma::S2MM_DMASR::IOC_Irq>())
    {
        _mmio_dma.write<Mmio_dma::S2MM_DMASR::IOC_Irq>(1); // clear interrupt
    }
    // otherwise it is an error interrupt
    else if(_mmio_dma.read<Mmio_dma::S2MM_DMASR::Err_Irq>())
    {
        _mmio_dma.write<Mmio_dma::S2MM_DMASR::Err_Irq>(1); // clear interrupt

        // is it an internal error?
        if(_mmio_dma.read<Mmio_dma::S2MM_DMASR::DMAIntErr>())
        {
            Genode::error("An internal error has been encountered by the DataMover ",
                          "on the data transport channel during S2MM transfer.");
            print_registers();
            throw Kcap::Internal_error();
        }

        // is it a slave error?
        if(_mmio_dma.read<Mmio_dma::S2MM_DMASR::DMASlvErr>())
        {
            Genode::error("An AXI slave error response has been received by the ",
                          "AXI DataMover during an S2MM transfer.");
            print_registers();
            throw Kcap::Internal_error();
        }

        // is it a decode error?
        if(_mmio_dma.read<Mmio_dma::S2MM_DMASR::DMADecErr>())
        {
            Genode::error("An AXI decode error has been received by the AXI DataMover during S2MM transfer. ",
                          "This error occurs if the DataMover issues an address request ",
                          "to an invalid location.");
            print_registers();
            throw Kcap::Invalid_address();
        }
    } else {
        // this should never ever happen.
        Genode::error("Got interrupt from unknown source during S2MM transfer.");
        print_registers();
        throw Kcap::Internal_error();
    }

	#if defined(DEBUG)
    Genode::log("Finish transfers");
    print_registers();       
    #endif

    
    return valid_kcap_count;
}


Driver& Driver::factory(Genode::Env &env,
                        Genode::addr_t dma_address,
                        Genode::uint32_t mm2s_irq_number,
                        Genode::uint32_t s2mm_irq_number,
                        Genode::addr_t gpio_address)
{
    static Driver driver(env, dma_address, mm2s_irq_number, s2mm_irq_number, gpio_address);
    return driver;
}


void Driver::print_registers()
{
    Genode::log("MM2S_DMASR: ", Genode::Hex(
                    _mmio_dma.read<Mmio_dma::MM2S_DMASR>()));
    Genode::log("MM2S_DMASR::Halted: ", Genode::Hex(
                    _mmio_dma.read<Mmio_dma::MM2S_DMASR::Halted>()));    
    Genode::log("MM2S_DMASR::Idle: ", Genode::Hex(
                    _mmio_dma.read<Mmio_dma::MM2S_DMASR::Idle>()));
    Genode::log("MM2S_DMASR::SGIncld: ", Genode::Hex(
                    _mmio_dma.read<Mmio_dma::MM2S_DMASR::SGIncld>()));    
    Genode::log("MM2S_DMASR::DMAIntErr: ", Genode::Hex(
                    _mmio_dma.read<Mmio_dma::MM2S_DMASR::DMAIntErr>()));
    Genode::log("MM2S_DMASR::DMASlvErr: ", Genode::Hex(
                    _mmio_dma.read<Mmio_dma::MM2S_DMASR::DMASlvErr>()));
    Genode::log("MM2S_DMASR::DMADecErr: ", Genode::Hex(
                    _mmio_dma.read<Mmio_dma::MM2S_DMASR::DMADecErr>()));
    Genode::log("MM2S_SA: ", Genode::Hex(
                    _mmio_dma.read<Mmio_dma::MM2S_SA>()));
    Genode::log("MM2S_LENGTH: ", Genode::Hex(
                    _mmio_dma.read<Mmio_dma::MM2S_LENGTH>()));    
    Genode::log("S2MM_DMASR: ", Genode::Hex(
                    _mmio_dma.read<Mmio_dma::S2MM_DMASR>()));
    Genode::log("S2MM_DMASR::Halted: ", Genode::Hex(
                    _mmio_dma.read<Mmio_dma::S2MM_DMASR::Halted>()));    
    Genode::log("S2MM_DMASR::Idle: ", Genode::Hex(
                    _mmio_dma.read<Mmio_dma::S2MM_DMASR::Idle>()));
    Genode::log("S2MM_DMASR::SGIncld: ", Genode::Hex(
                    _mmio_dma.read<Mmio_dma::S2MM_DMASR::SGIncld>()));    
    Genode::log("S2MM_DMASR::DMAIntErr: ", Genode::Hex(
                    _mmio_dma.read<Mmio_dma::S2MM_DMASR::DMAIntErr>()));
    Genode::log("S2MM_DMASR::DMASlvErr: ", Genode::Hex(
                    _mmio_dma.read<Mmio_dma::S2MM_DMASR::DMASlvErr>()));
    Genode::log("S2MM_DMASR::DMADecErr: ", Genode::Hex(
                    _mmio_dma.read<Mmio_dma::S2MM_DMASR::DMADecErr>()));
    Genode::log("S2MM_DA: ", Genode::Hex(
                    _mmio_dma.read<Mmio_dma::S2MM_DA>()));
    Genode::log("S2MM_LENGTH: ", Genode::Hex(
                    _mmio_dma.read<Mmio_dma::S2MM_LENGTH>()));
    Genode::log("TOTAL_KCAP_COUNT: ", Genode::Hex(
                    _mmio_gpio.read<Mmio_gpio::TOTAL_KCAP_COUNT>()));
    Genode::log("VALID_KCAP_COUNT: ", Genode::Hex(
                    _mmio_gpio.read<Mmio_gpio::VALID_KCAP_COUNT>()));    
}
