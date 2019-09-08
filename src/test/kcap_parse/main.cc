/*
 * \brief  Test application for `kcap_drv` driver. 
 * \author Johannes Fischer
 * \date   2019-01-09
 */


#include <base/component.h>
#include <base/log.h>
#include <base/heap.h>
#include <kcap_session/connection.h>
#include <base/sleep.h>
#include <timer_session/connection.h>
#include <region_map/client.h>
#include <dataspace/client.h>
#include <base/cache.h>


#include "rtcr/util/kcap_badge_info.h"


namespace Rtcr {
	struct Main;
}

struct Rtcr::Main
{
	enum { ROOT_STACK_SIZE = 16*1024 };
	Genode::Env              &env;
	Genode::Heap              heap            { env.ram(), env.rm() };

    const static Genode::size_t KCAP_LIST_SIZE = 4096;    
    Genode::uint32_t KCAP_LIST[KCAP_LIST_SIZE];


    Genode::size_t KCAP_INDEX_SIZE = 0x10;
    
	Main(Genode::Env &env_) : env(env_)
    {
        using namespace Genode;
        
        Timer::Connection timer(env);
        Kcap::Connection kcap(env);
        
        // create source dataspace
        Dataspace_capability src_ds_cap = env.ram().alloc(
            KCAP_INDEX_SIZE*KCAP_LIST_SIZE,
            Cache_attribute::UNCACHED);
        Dataspace_client src_ds_client(src_ds_cap);
        addr_t src_phys_addr = src_ds_client.phys_addr();
        uint32_t* src_ds_addr = env.rm().attach(src_ds_cap);
        Genode::log("src_phys_addr: ", Genode::Hex(src_phys_addr));


        for(int i = 1; i < 101; i++)
        {
            KCAP_LIST[i] = i;
        }
        
        // fill source dataspace with cap_index elements
        for(int i = 0; i < KCAP_LIST_SIZE; i++)
        {
            *(src_ds_addr+i*4+1) = (KCAP_LIST[i] << 16);
        }
        env.rm().detach(src_ds_addr);

        
        // parse capabilities with FPGA
        Genode::size_t parsed_kcap_count = kcap.parse_kcap_map(src_phys_addr);
        Genode::Dataspace_capability parsed_kcap_ds_cap = kcap.retrieve_shared_mem();
        Genode::addr_t parsed_kcap_ds_addr = env.rm().attach(parsed_kcap_ds_cap);
        Kcap::Parsed_cap_index* parsed_kcap_index = (Kcap::Parsed_cap_index*)parsed_kcap_ds_addr;
        for(size_t i = 0; i < parsed_kcap_count; i++)
        {
            Genode::log("hw_kcap_index[",i,"]: kcap=", Hex(parsed_kcap_index[i].kcap),
                        ", badge=", parsed_kcap_index[i].badge);
        }
        env.rm().detach(parsed_kcap_ds_addr);
        
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
