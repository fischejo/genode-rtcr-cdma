/*
 * \brief  Intercepting Ram session
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#include <rtcr_cdma/ram_cdma_session.h>

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

Ram_cdma_session::Ram_cdma_session(Genode::Env &env,
				   Genode::Allocator &md_alloc,
				   const char *creation_args,
				   Child_info *child_info)
	:
	Ram_session(env, md_alloc, creation_args, child_info),
	_cdma_drv(env)
{
	DEBUG_THIS_CALL;

	if (!_cdma_drv.is_supported()) {
		Genode::error("Cdma driver is not supported. No fallback supported.");
	}	
}


void Ram_cdma_session::_destroy_dataspace(Ram_dataspace *ds)
{
	DEBUG_THIS_CALL;
	if(ds->i_cached) {
		Genode::destroy(_md_alloc, (Physical_address *)ds->storage);
	}
	Ram_session::_destroy_dataspace(ds);
}


void Ram_cdma_session::_alloc_dataspace(Ram_dataspace *ds)
{
	DEBUG_THIS_CALL;
	/* only if the src dataspace is allocated as uncached, also the
	 * destination dataspace will be allocated as uncached */
	ds->i_dst_cap = _env.ram().alloc(ds->i_size, ds->i_cached);
}


Genode::Ram_dataspace_capability Ram_cdma_session::alloc(Genode::size_t size,
							 Genode::Cache_attribute cached)
{
	DEBUG_THIS_CALL;
	return Ram_session::alloc(size, cached);
}


void Ram_cdma_session::_attach_dataspace(Ram_dataspace *ds)
{
	DEBUG_THIS_CALL;
	Ram_session::_attach_dataspace(ds);

	if(ds->i_cached) {
		/* also directly calculate the physical address. I assume that it will
		 * not change again. */
		Genode::Dataspace_client dst_client(ds->i_dst_cap);
		Genode::Dataspace_client src_client(ds->i_src_cap);

		ds->storage = new (_md_alloc) Physical_address(dst_client.phys_addr(),
							       src_client.phys_addr());
	}
}


void Ram_cdma_session::_copy_dataspace(Ram_dataspace *ds)
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL;
	/* only copy a dataspace with hardware-acceleration, if it is supported
	 * by the dataspace (uncached) */
	if(ds->i_cached) {
		Physical_address *p = (Physical_address *)ds->storage;		
		_cdma_drv.memcpy(p->dst_addr, p->src_addr, ds->i_size);	
	} else {
		Ram_session::_copy_dataspace(ds);
	}

}


Ram_session *Ram_cdma_root::_create_ram_session(Child_info *info, const char *args)
{
	DEBUG_THIS_CALL;	
	return new (md_alloc()) Ram_cdma_session(_env, _md_alloc, args, info);
}



