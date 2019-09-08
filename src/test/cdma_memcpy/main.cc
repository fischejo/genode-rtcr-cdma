/*
 * \brief  Test application for `cmda_drv` driver. A dataspace is filled with
 *         characters and copied to a second dataspace.
 * \author Johannes Fischer
 * \date   2018-12-16
 */


#include <base/component.h>
#include <base/log.h>
#include <cdma_session/connection.h> 
#include <base/sleep.h>
#include <timer_session/connection.h>
#include <region_map/client.h>
#include <dataspace/client.h>


namespace Rtcr {
	class Main;
}

class Rtcr::Main
{
    Genode::size_t DATASPACE_SIZE = 0x18;
	enum {
		ROOT_STACK_SIZE = 16*1024,

	};

	Genode::Env &env;

public:
	Main(Genode::Env &env_) : env(env_)
    {
        using namespace Genode;
        Timer::Connection timer(env);
        Cdma::Connection cdma(env);

        
        if(!cdma.is_supported())
        {
            Genode::error("CDMA Driver is not supported.");
            return;

        }
        // create two dataspaces
        Dataspace_capability dst_ds_cap = env.ram().alloc(DATASPACE_SIZE, Cache_attribute::UNCACHED);
        Dataspace_capability src_ds_cap = env.ram().alloc(DATASPACE_SIZE, Cache_attribute::UNCACHED);        

        Dataspace_client dst_ds_client(dst_ds_cap);
        Dataspace_client src_ds_client(src_ds_cap);

        // attach
        void *src_addr_start = env.rm().attach(src_ds_cap);
        void *dst_addr_start = env.rm().attach(dst_ds_cap);        

        // fill source dataspace with random characters
        for(unsigned int i = 0; i < DATASPACE_SIZE; i++)
        {
            *((int *) src_addr_start+i) = 0xEFBEADDE;
        }

        // fill second dataspace with an arbitrary character. Otherwise Genode
        // keeps it uninitialized.
        *((int *) dst_addr_start) = 0x1;

        // detach
        // Note: During cdma copying, `_dst_ds_cap` should be detached.
        env.rm().detach(src_addr_start);
        env.rm().detach(dst_addr_start);
        
        // calculate physical address for destination and source
        addr_t dst_addr = dst_ds_client.phys_addr();
        addr_t src_addr = src_ds_client.phys_addr();        

        // copy
        cdma.memcpy(dst_addr, src_addr, DATASPACE_SIZE);

        // attach again
        src_addr_start = env.rm().attach(src_ds_cap);
        dst_addr_start = env.rm().attach(dst_ds_cap);        

        // compare both dataspaces        
        for(unsigned int i = 0; i < DATASPACE_SIZE; i++)
        {
            char src = ((char *) src_addr_start)[i];
            char dst = ((char *) dst_addr_start)[i];
            Genode::log( Genode::Hex(src), " at ", Genode::Hex(i), " => ", \
                         Genode::Hex(dst), " at ", Genode::Hex(i), \
                         " [", (src == dst ? "x" : " "), "]");
        }
        

        if (Genode::memcmp(dst_addr_start, src_addr_start, DATASPACE_SIZE)) {
            Genode::log("Test failed.");
        }
        else {
            Genode::log("Test successful.");
        }


        // detach 
        env.rm().detach(src_addr_start);
        env.rm().detach(dst_addr_start);

        // done
        log("the_end");
        Genode::sleep_forever();
    }

};

Genode::size_t Component::stack_size() { return 16*1024; }

void Component::construct(Genode::Env &env)
{
	static Rtcr::Main main(env);
}
