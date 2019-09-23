/*
 * \brief  Cdma Module creates all intecepting sessions
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_CDMA_MODULE_H_
#define _RTCR_CDMA_MODULE_H_

/* Genode includes */
#include <base/heap.h>
#include <base/allocator.h>
#include <base/service.h>


/* Local includes */
#include <rtcr/init_module.h>
#include <rtcr/module_factory.h>

#include <rtcr/cpu/cpu_session.h>
#include <rtcr/pd/pd_session.h>
#include <rtcr/rm/rm_session.h>
#include <rtcr/log/log_session.h>
#include <rtcr/timer/timer_session.h>
#include <rtcr/rom/rom_session.h>
#include <rtcr/cap/capability_mapping.h>
#include <rtcr/child_info.h>
#include <rtcr/root_component.h>

#include <rtcr_cdma/pd_session.h>

namespace Rtcr {
	class Cdma_module;
}

using namespace Rtcr;


/**
 * The class Rtcr::Base_module provides the intercepting sessions and handles
 * the communication between a target child and the corresponding sessions.
 */
class Rtcr::Cdma_module : public virtual Init_module
{
private:
	Genode::Entrypoint _ep;
	Root_component<Pd_cdma_session> _pd;
	Root_component<Cpu_session> _cpu;
	Root_component<Log_session> _log;
	Root_component<Timer_session> _timer;
	Root_component<Rom_session> _rom;
	Root_component<Rm_session> _rm;      
public:	
	Cdma_module(Genode::Env &env, Genode::Allocator &alloc);
	
	static Module_name name() { return "cdma"; }
};

#endif /* _RTCR_CDMA_MODULE_H_ */
