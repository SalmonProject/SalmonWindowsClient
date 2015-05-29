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

#ifndef VMIME_NET_TIMEOUTHANDLER_HPP_INCLUDED
#define VMIME_NET_TIMEOUTHANDLER_HPP_INCLUDED


#include "../vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES


#include "../vmime/types.hpp"


namespace vmime {
namespace net {


/** A class to manage time-out in messaging services.
  */

class VMIME_EXPORT timeoutHandler : public object
{
public:

	virtual ~timeoutHandler() { }

	/** Called to test if the time limit has been reached.
	  *
	  * @return true if the time-out delay is elapsed
	  */
	virtual bool isTimeOut() = 0;

	/** Called to reset the time-out counter.
	  */
	virtual void resetTimeOut() = 0;

	/** Called when the time limit has been reached (when
	  * isTimeOut() returned true).
	  *
	  * @return true to continue (and reset the time-out)
	  * or false to cancel the current operation
	  */
	virtual bool handleTimeOut() = 0;

    // FIX by Elmue: Added the possibility to modify the timeout interval.
    // This may be used to reduce the timeout for example for the SMTP command QUIT 
    // that is not responded by the Microsoft Exchange Server after a 554 error (Transaction failed).
    // s32_Timeout in seconds!
    virtual void ModifyInterval(int s32_Timeout) = 0;
};


/** A class to create 'timeoutHandler' objects.
  */

class timeoutHandlerFactory : public object
{
public:

	virtual ~timeoutHandlerFactory() { }

	virtual ref <timeoutHandler> create() = 0;
};


} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES

#endif // VMIME_NET_TIMEOUTHANDLER_HPP_INCLUDED
