/*
 * \brief  Driver for CDMA implementation running on FPGA of Zybo Board.
 * \author Johannes Fischer
 * \date   2018-12-15
 */

#include <cdma_session/cdma_session.h>
#include <base/attached_rom_dataspace.h>

#include <base/component.h>
#include <base/log.h>
#include <base/heap.h>
#include <root/component.h>
#include <base/rpc_server.h>
#include <base/stdint.h>

#include <cdma/cdma.h>
#include <cdma/driver.h>

namespace Cdma {
	struct Session_component;
    struct Root_component;
    struct Main;
};


struct Cdma::Session_component : Genode::Rpc_object<Session>
{
private:

    Driver &_driver;

public:
    Session_component(Driver &driver)
		: _driver(driver) {}

    virtual void memcpy(Genode::addr_t dst, Genode::addr_t src, Genode::size_t size) {
        _driver.memcpy(dst, src, size);
    }

    virtual bool is_supported() {
        return _driver.is_supported();
    }
  
};


class Cdma::Root_component : public Genode::Root_component<Cdma::Session_component>
{
private:

    Driver &_driver;

protected:

    Session_component *_create_session(const char *args)
		{
			Genode::size_t ram_quota = Genode::Arg_string::find_arg(args, "ram_quota").ulong_value(0);

			if (ram_quota < sizeof(Session_component)) {
                Genode::warning("Insufficient dontated ram_quota (", ram_quota, " bytes), "
                                "require ", sizeof(Session_component), " bytes");
			}
            
			return new (md_alloc()) Session_component(_driver);
		}

public:

    Root_component(Genode::Entrypoint &ep,
                   Genode::Allocator &alloc,
                   Driver &driver)
        :
        Genode::Root_component<Cdma::Session_component>(ep, alloc),
        _driver(driver)
        {
			#if defined(DEBUG)
			Genode::log("creating root component");
            #endif
		}
};


struct Cdma::Main
{
	Genode::Env         &env;
	Genode::Attached_rom_dataspace config_rom { env, "config" };
	Genode::Sliced_heap sliced_heap { env.ram(), env.rm() };
    

	Main(Genode::Env &env)
        :
		env(env)
        {

            /*
             * Read config
             */
            Genode::addr_t cdma_address = 0;
            Genode::uint32_t irq_number = 0;
            bool sg_enabled = true;

            // parse physical base address of CDMA 
            Genode::Xml_node cdma_node = config_rom.xml().sub_node("cdma");
            cdma_node.attribute("address").value(&cdma_address);
            cdma_node.attribute("irq").value(&irq_number);
            
            try {
                // is scather gather mode enabled?
                cdma_node.attribute("sg_enabled").value(&sg_enabled);
            }
            catch (Genode::Xml_node::Nonexistent_attribute) {}

			#if defined(DEBUG)
            Genode::log("CDMA Address: ", Hex(cdma_address));
            Genode::log("CDMA Interrupt: ", irq_number);
            Genode::log("Scather/Gather: ", sg_enabled ? "enabled" : "disabled");                        
			#endif

            /*
             * Create Driver
             */
            Cdma::Driver &driver = Cdma::Driver::factory(
                env,
                cdma_address,
                irq_number,
                sg_enabled);

            /*
             * Announce service
             */
            static Cdma::Root_component root(env.ep(), sliced_heap, driver);
            env.parent().announce(env.ep().manage(root));

        }
};


Genode::size_t Component::stack_size() { return 64*1024; }


void Component::construct(Genode::Env &env)
{
	static Cdma::Main main(env);
}
