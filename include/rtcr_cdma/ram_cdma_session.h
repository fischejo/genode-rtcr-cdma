/*
 * \brief  Intercepting Ram session
 * \author Johannes Fischer
 * \date   2019-09-03
 */

#ifndef _RTCR_RAM_CDMA_SESSION_H_
#define _RTCR_RAM_CDMA_SESSION_H_

/* Genode includes */

/* Rtcr includes */
#include <rtcr/ram/ram_session.h>
#include <cdma_session/connection.h>

namespace Rtcr {
	class Ram_cdma_session;
	class Ram_cdma_root;
}



class Rtcr::Ram_cdma_session : public Rtcr::Ram_session
{
private:
	Cdma::Connection _cdma_drv;
	
	struct Physical_address {
		Genode::addr_t dst_addr;
		Genode::addr_t src_addr;
		
		Physical_address(Genode::addr_t _dst_addr, Genode::addr_t _src_addr)
			: dst_addr(_dst_addr), src_addr(_src_addr) {}
	};

protected:
	
	void _copy_dataspace(Ram_dataspace *info) override;
	void _attach_dataspace(Ram_dataspace *info) override;
	void _destroy_dataspace(Ram_dataspace *ds) override;
	void _alloc_dataspace(Ram_dataspace *ds) override;
	
public:
	Ram_cdma_session(Genode::Env &env,
			 Genode::Allocator &md_alloc,
			 const char *creation_args,
			 Child_info *child_info);

	Genode::Ram_dataspace_capability alloc(Genode::size_t size,
					       Genode::Cache_attribute cached) override;
	
};


class Rtcr::Ram_cdma_root : public Rtcr::Ram_root
{
protected:
	Ram_session *_create_ram_session(Child_info *info, const char *args) override;

public:
	Ram_cdma_root(Genode::Env &env,
		      Genode::Allocator &md_alloc,
		      Genode::Entrypoint &session_ep,
		      Genode::Lock &childs_lock,
		      Genode::List<Child_info> &childs)
		:
		Ram_root(env, md_alloc, session_ep, childs_lock, childs) {}
};


#endif /* _RTCR_RAM_CDMA_SESSION_H_ */
