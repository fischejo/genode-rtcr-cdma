/*
 * \brief  Driver for harware-based capability parsing running on the FPGA of the Zybo Board.
 * \author Johannes Fischer
 * \date   2019-01-09
 */

#include <kcap_session/kcap_session.h>
#include <base/attached_rom_dataspace.h>

#include <base/component.h>
#include <base/log.h>
#include <base/heap.h>
#include <root/component.h>
#include <base/rpc_server.h>
#include <base/stdint.h>

#include <kcap/kcap.h>
#include <kcap/driver.h>

namespace Kcap {
	struct Session_component;
    struct Root_component;
    struct Main;
};


struct Kcap::Session_component : Genode::Rpc_object<Session>
{
private:
    Kcap::Driver &_driver;
    Genode::Ram_dataspace_capability _ds_cap;
    Genode::addr_t _ds_phys_addr;
    Genode::Env &_env;
    
public:
    static const Genode::uint32_t DS_SIZE =
        Kcap::MAX_CAP_INDEX*sizeof(Kcap::Parsed_cap_index);

    
    Session_component(Genode::Env &env, Kcap::Driver &driver)
		:
        _env(env),
        _driver(driver),
        _ds_cap(env.ram().alloc(DS_SIZE, Cache_attribute::UNCACHED)),
        _ds_phys_addr(Dataspace_client(_ds_cap).phys_addr())
        {
			#if defined(DEBUG)
            Genode::log("Created shared memory: ",
                        "phys_addr=", _ds_phys_addr,
                        "size=", DS_SIZE);
			#endif
        }

    virtual Genode::size_t parse_kcap_map(Genode::addr_t src) {
        return _driver.parse_kcap_map(_ds_phys_addr, src);
    }

    virtual Genode::Dataspace_capability retrieve_shared_mem() {
        return _ds_cap;
    }

    ~Session_component()
    {
        _env.ram().free(_ds_cap);
    }    
};


class Kcap::Root_component : public Genode::Root_component<Kcap::Session_component>
{
private:
    Genode::Env &_env;
    Kcap::Driver &_driver;

protected:

    Session_component *_create_session(const char *args)
		{
			return new (md_alloc()) Session_component(_env, _driver);
		}

public:
    Root_component(Genode::Env &env,
                   Genode::Entrypoint &ep,
                   Genode::Allocator &alloc,
        		   Kcap::Driver &driver)
        :
        Genode::Root_component<Kcap::Session_component>(ep, alloc),
        _env(env),
        _driver(driver)
        {
			Genode::log("creating root component");
		}
};


struct Kcap::Main
{
	Genode::Env         &env;
	Genode::Sliced_heap sliced_heap { env.ram(), env.rm() };
	Genode::Attached_rom_dataspace config_rom { env, "config" };    

	Main(Genode::Env &env)
        :
		env(env)
        {
            /*
             * Read config
             */
            Genode::addr_t dma_address = 0;
            Genode::uint32_t mm2s_irq_number = 0;
            Genode::uint32_t s2mm_irq_number = 0;
            Genode::addr_t gpio_address = 0;   

            // parse physical base address and interrupt numbers for DMA IP
            Genode::Xml_node dma_node = config_rom.xml().sub_node("dma");
            dma_node.attribute("address").value(&dma_address);
            dma_node.attribute("mm2s_irq").value(&mm2s_irq_number);
            dma_node.attribute("s2mm_irq").value(&s2mm_irq_number);            

            // parse physical base address and interrupt numbers for GPIO IP
            Genode::Xml_node gpio_node = config_rom.xml().sub_node("gpio");
            gpio_node.attribute("address").value(&gpio_address);

			#if defined(DEBUG)
            Genode::log("DMA Address: ", Hex(dma_address));
            Genode::log("MM2S Interrupt: ", mm2s_irq_number);
            Genode::log("S2MM Interrupt: ", s2mm_irq_number);
            Genode::log("GPIO Address", Hex(gpio_address));                        
			#endif

            
            /*
             * Create Driver
             */
            Kcap::Driver &driver = Kcap::Driver::factory(
                env,
                dma_address,
                mm2s_irq_number,
                s2mm_irq_number,
                gpio_address);

            /*
             * Announce service
             */
            static Kcap::Root_component root(env ,env.ep(), sliced_heap, driver);
            env.parent().announce(env.ep().manage(root));
        }
};


Genode::size_t Component::stack_size() { return 64*1024; }


void Component::construct(Genode::Env &env)
{
	static Kcap::Main main(env);
}
