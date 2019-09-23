/*
 * \brief  Intercepting Pd session
 * \author Johannes Fischer
 * \date   2019-09-03
 */

#ifndef _RTCR_PD_CDMA_SESSION_H_
#define _RTCR_PD_CDMA_SESSION_H_

/* Genode includes */

/* Rtcr includes */
#include <rtcr/pd/pd_session.h>
#include <cdma_session/connection.h>

namespace Rtcr {
	class Pd_cdma_session;
}



class Rtcr::Pd_cdma_session : public Rtcr::Pd_session
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
	Pd_cdma_session(Genode::Env &env,
			Genode::Allocator &md_alloc,
			Genode::Entrypoint &ep,
			const char *creation_args,
			Child_info *child_info);
	
};

#endif /* _RTCR_PD_CDMA_SESSION_H_ */
