/*
 * \brief  Session for CDMA driver
 * \author Johannes Fischer
 * \date   2018-12-15
 */

#ifndef _INCLUDE__CDMA_SESSION__CDMA_SESSION_H_
#define _INCLUDE__CDMA_SESSION__CDMA_SESSION_H_

#include <session/session.h>
#include <base/rpc.h>
#include <base/stdint.h>
#include <base/exception.h>
#include <cdma/driver.h>

namespace Cdma {
  struct Session;
}      

struct Cdma::Session : Genode::Session
{
	static const char *service_name() { return "Cdma"; }
    virtual void memcpy(Genode::addr_t dst, Genode::addr_t src, Genode::size_t size) = 0;
    virtual bool is_supported() = 0;

  /*******************
   ** RPC interface **
   *******************/
    GENODE_RPC_THROW(Rpc_cdma_memcpy,
                     void,
                     memcpy,
                     GENODE_TYPE_LIST(Cdma::Function_unsupported,
                                      Cdma::Internal_memcpy_error,
                                      Cdma::Invalid_memcpy_address),
                     Genode::addr_t,
                     Genode::addr_t,
                     Genode::size_t);
    
    GENODE_RPC(Rpc_cdma_is_supported,
               bool,
               is_supported);

    GENODE_RPC_INTERFACE(Rpc_cdma_memcpy, Rpc_cdma_is_supported);
};


#endif /* _INCLUDE__CDMA_SESSION__CDMA_SESSION_H_ */
