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

#include <rtcr_cdma/ram_cdma_session.h>

namespace Rtcr {
	class Cdma_module;
	class Cdma_module_factory;
}

using namespace Rtcr;


/**
 * The class Rtcr::Base_module provides the intercepting sessions and handles
 * the communication between a target child and the corresponding sessions.
 */
class Rtcr::Cdma_module : public virtual Init_module
{
public:	
	Cdma_module(Genode::Env &env, Genode::Allocator &alloc);
	
	Module_name name() override { return "cdma"; }
};

/**
 * Factory class for creating the Rtcr::Base_module
 */
class Rtcr::Cdma_module_factory : public Module_factory
{
public:
	Init_module* create(Genode::Env &env, Genode::Allocator &alloc) override {
		return new (alloc) Cdma_module(env, alloc);
	}
    
	Module_name name() override { return "cdma"; }
};

#endif /* _RTCR_BASE_MODULE_H_ */
