//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2013 Vincent Richard <vincent@vmime.org>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// Linking this library statically or dynamically with other modules is making
// a combined work based on this library.  Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
//

#ifndef VMIME_NET_POP3_POP3UTILS_HPP_INCLUDED
#define VMIME_NET_POP3_POP3UTILS_HPP_INCLUDED


#include "../vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3


#include <map>

#include "../vmime/types.hpp"

#include "../vmime/net/messageSet.hpp"


namespace vmime {
namespace net {
namespace pop3 {


class POP3Response;


class VMIME_EXPORT POP3Utils
{
public:

	/** Parse a response of type ([integer] [string] \n)*.
	  * This is used in LIST or UIDL commands:
	  *
	  *    C: UIDL
	  *    S: +OK
	  *    S: 1 whqtswO00WBw418f9t5JxYwZ
	  *    S: 2 QhdPYR:00WBw1Ph7x7
	  *    S: .
	  *
	  * @param response raw response string as returned by the server
	  * @param result points to an associative array which maps a message
	  * number to its corresponding data (either UID or size)
	  */
	static void parseMultiListOrUidlResponse
		(ref <POP3Response> response, std::map <int, string>& result);

	/** Returns a list of message numbers given a message set.
	  *
	  * @param msgs message set
	  * @return list of message numbers
	  */
	static const std::vector <int> messageSetToNumberList(const messageSet& msgs);
};


} // pop3
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3

#endif // VMIME_NET_POP3_POP3UTILS_HPP_INCLUDED

