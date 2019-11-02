/*
 * \brief  Connection to CDMA driver
 * \author Johannes Fischer
 * \date   2018-12-15
 */

#ifndef _INCLUDE__CDMA_SESSION__CONNECTION_H_
#define _INCLUDE__CDMA_SESSION__CONNECTION_H_

#include <cdma_session/client.h>
#include <base/connection.h>

namespace Cdma {
    struct Connection;
}

struct Cdma::Connection : Genode::Connection<Session>, Session_client
{
	Connection(Genode::Env &env)
	:
		Genode::Connection<Session>(env, session(env.parent(), "ram_quota=8K")),
		Session_client(cap()) { }
};


#endif /* _INCLUDE__CDMA_SESSION__CONNECTION_H_ */
