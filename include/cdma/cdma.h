/*
 * \brief  Driver for CDMA implementation running on FPGA of Zybo Board.
 * \author Johannes Fischer
 * \date   2018-12-16
 */


#ifndef _CDMA_H_
#define _CDMA_H_

/* Genode includes */
#include <base/attached_io_mem_dataspace.h>
#include <util/mmio.h>


namespace Cdma {
	using namespace Genode;
	class Mmio_cdma;
}


struct Cdma::Mmio_cdma :  Attached_io_mem_dataspace, Mmio
{
	Mmio_cdma(Genode::Env &env, Genode::addr_t const mmio_address)
    :
    Genode::Attached_io_mem_dataspace(env, mmio_address, 0x32),
    Genode::Mmio((Genode::addr_t)local_addr<void>())
    {}


    // Register Address Map
    // CDMA Control
    struct CDMACR : Register<0x00, 32>
    {
        struct Undefined : Bitfield<0,1> {};
        struct TailPntrEn : Bitfield<1,1> {};
        struct Reset : Bitfield<2,1> {};
        struct SGMode : Bitfield<3,1> {};
        struct Key_Hole_Read : Bitfield<4,1> {};
        struct Key_Hole_Write : Bitfield<5,1> {};
        struct Cyclic_BD_Enable : Bitfield<6,1> {};
        struct IOC_IrqEn : Bitfield<12,1> {};
        struct Dly_IrqEn : Bitfield<13,1> {};
        struct Err_IrqEn : Bitfield<14,1> {};
        struct IRQThreshold : Bitfield<16,8> {};
        struct IRQDelay : Bitfield<24, 8> {};
    };

    // CDMA Status
    struct CDMASR : Register<0x04, 32>
    {
        struct Idle : Bitfield<1, 1> {};
        struct SGIncld : Bitfield<3, 1> {};
        struct DMAIntErr : Bitfield<4, 1> {};
        struct DMASlvErr : Bitfield<5, 1> {};
        struct DMADecErr : Bitfield<6, 1> {};
        struct SGIntErr : Bitfield<8, 1> {};
        struct SGSlvErr : Bitfield<9, 1> {};
        struct SGDecErr : Bitfield<10, 1> {};
        struct IOC_Irq : Bitfield<12, 1> {};
        struct Dly_Irq : Bitfield<13, 1> {};
        struct Err_Irq : Bitfield<14, 1> {};
        struct IRQThresholdSts : Bitfield<16, 8> {};
        struct IRQDelaySts : Bitfield<24, 8> {};
    };
    
    // Current Descriptor Pointer
    struct CURDESC_PNTR : Register<0x08, 32>{};
    struct CURDESC_PNTR_MSB : Register<0x0c, 32>{};    
    
    // Tail Descriptor Pointer
    struct TAILDESC_PNTR : Register<0x10, 32>{};
    struct TAILDESC_PNTR_MSB : Register<0x14, 32>{};    
    
    // Source Address
    struct SA : Register<0x18, 32> {};
    struct SA_MSB : Register<0x1c, 32> {};

    // Destination Address
    struct DA : Register<0x20, 32> {};
    struct DA_MSB : Register<0x24, 32> {};
    
    // Bytes to Transfer
    struct BTT : Register<0x28, 32>{};


};

#endif // _CDMA_H_
