/*
 * \brief  Client for CDMA driver
 * \author Johannes Fischer
 * \date   2018-12-15
 */

#ifndef _INCLUDE__CDMA_SESSION_H__CLIENT_H_
#define _INCLUDE__CDMA_SESSION_H__CLIENT_H_

#include <cdma_session/cdma_session.h>
#include <base/rpc_client.h>
#include <base/log.h>
#include <base/stdint.h>

namespace Cdma {
    struct Session_client;
}

struct Cdma::Session_client : Genode::Rpc_client<Session>
{
  
	Session_client(Genode::Capability<Session> cap)
    : Genode::Rpc_client<Session>(cap) { }
  
  
    void memcpy(Genode::addr_t dst, Genode::addr_t src, Genode::size_t size) {
        call<Rpc_cdma_memcpy>(dst, src, size);
    }

    bool is_supported() {
        return call<Rpc_cdma_is_supported>();
    }
  
};


#endif /* _INCLUDE__CDMA_SESSION_H__CLIENT_H_ */
