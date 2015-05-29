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

#include "../vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP


#include "../vmime/net/imap/IMAPSStore.hpp"


namespace vmime {
namespace net {
namespace imap {


IMAPSStore::IMAPSStore(ref <session> sess, ref <security::authenticator> auth)
	: IMAPStore(sess, auth, true)
{
}


IMAPSStore::~IMAPSStore()
{
}


const string IMAPSStore::getProtocolName() const
{
	return "imaps";
}



// Service infos

IMAPServiceInfos IMAPSStore::sm_infos(true);


const serviceInfos& IMAPSStore::getInfosInstance()
{
	return sm_infos;
}


const serviceInfos& IMAPSStore::getInfos() const
{
	return sm_infos;
}


} // imap
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP

