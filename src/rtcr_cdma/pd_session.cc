/*
 * \brief  Intercepting Ram session
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#include <rtcr_cdma/pd_session.h>

using namespace Rtcr;

#ifdef PROFILE
#include <util/profiler.h>
#define PROFILE_THIS_CALL PROFILE_FUNCTION("cyan");
#else
#define PROFILE_THIS_CALL
#endif

#if DEBUG 
#define DEBUG_THIS_CALL Genode::log("\e[38;5;87m", __PRETTY_FUNCTION__, "\033[0m");
#else
#define DEBUG_THIS_CALL
#endif

Pd_cdma_session::Pd_cdma_session(Genode::Env &env,
				 Genode::Allocator &md_alloc,
				 Genode::Entrypoint &ep,
				 const char *creation_args,
				 Child_info *child_info)
	:
	Pd_session(env, md_alloc, ep, creation_args, child_info),
	_cdma_drv(env)
{
	DEBUG_THIS_CALL;

	if (!_cdma_drv.is_supported()) {
		Genode::error("Cdma driver is not supported. No fallback supported.");
	}	
}


void Pd_cdma_session::_destroy_dataspace(Ram_dataspace *ds)
{
	DEBUG_THIS_CALL;
	if(!ds->i_cached) {
		Genode::destroy(_md_alloc, (Physical_address *)ds->storage);
	}
	Pd_session::_destroy_dataspace(ds);
}


void Pd_cdma_session::_alloc_dataspace(Ram_dataspace *ds)
{
	DEBUG_THIS_CALL;
	/* only if the src dataspace is allocated as uncached, also the
	 * destination dataspace will be allocated as uncached */
	ds->i_dst_cap = _env.pd().alloc(ds->i_size, ds->i_cached);
}


void Pd_cdma_session::_attach_dataspace(Ram_dataspace *ds)
{
	DEBUG_THIS_CALL;
	Pd_session::_attach_dataspace(ds);

	if(!ds->i_cached) {
		/* also directly calculate the physical address. I assume that it will
		 * not change again. */
		Genode::Dataspace_client dst_client(ds->i_dst_cap);
		Genode::Dataspace_client src_client(ds->i_src_cap);

		ds->storage = new (_md_alloc) Physical_address(dst_client.phys_addr(),
							       src_client.phys_addr());
	}
}


void Pd_cdma_session::_copy_dataspace(Ram_dataspace *ds)
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL;
	/* only copy a dataspace with hardware-acceleration, if it is supported
	 * by the dataspace (uncached) */
	if(!ds->i_cached) {
		Physical_address *p = (Physical_address *)ds->storage;		
		_cdma_drv.memcpy(p->dst_addr, p->src_addr, ds->i_size);	
	} else {
		Pd_session::_copy_dataspace(ds);
	}

}

