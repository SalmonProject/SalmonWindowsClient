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


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3


#include "../vmime/net/pop3/POP3Message.hpp"
#include "../vmime/net/pop3/POP3Command.hpp"
#include "../vmime/net/pop3/POP3Response.hpp"
#include "../vmime/net/pop3/POP3Folder.hpp"
#include "../vmime/net/pop3/POP3Store.hpp"

#include "../vmime/utility/outputStreamAdapter.hpp"
#include "../vmime/utility/outputStreamStringAdapter.hpp"

#include <sstream>


namespace vmime {
namespace net {
namespace pop3 {


POP3Message::POP3Message(ref <POP3Folder> folder, const int num)
	: m_folder(folder), m_num(num), m_size(-1), m_deleted(false)
{
	folder->registerMessage(this);
}


POP3Message::~POP3Message()
{
	ref <POP3Folder> folder = m_folder.acquire();

	if (folder)
		folder->unregisterMessage(this);
}


void POP3Message::onFolderClosed()
{
	m_folder = NULL;
}


int POP3Message::getNumber() const
{
	return (m_num);
}

// FIX by Elmue: Performance improvement -> get only on demand from server
const message::uid POP3Message::getUID()
{
    if (m_uid.empty())
    {
        ref<POP3Folder> folder = m_folder.acquire();
	    if (!folder)
		    throw exceptions::illegal_state("Folder closed");

        folder->fetchMessage(thisRef().dynamicCast<message>(), folder::FETCH_UID);
    }
	return (m_uid);
}

// FIX by Elmue: Performance improvement -> get only on demand from server
int POP3Message::getSize()
{
	if (m_size == -1)
    {
        ref<POP3Folder> folder = m_folder.acquire();
        if (!folder)
		    throw exceptions::illegal_state("Folder closed");

        folder->fetchMessage(thisRef().dynamicCast<message>(), folder::FETCH_SIZE);
    }
	return (m_size);
}


bool POP3Message::isExpunged() const
{
	return (false);
}


int POP3Message::getFlags() const
{
    // FIX by Elmue: Removed flag RECENT.
    // It is nonsense to set RECENT flag always, even for emails that are 1 year old!
	int flags = 0;

	if (m_deleted)
		flags |= FLAG_DELETED;

	return (flags);
}


ref <const messageStructure> POP3Message::getStructure() const
{
	throw exceptions::operation_not_supported();
}


ref <messageStructure> POP3Message::getStructure()
{
	throw exceptions::operation_not_supported();
}


ref <const header> POP3Message::getHeader() const
{
	if (m_header == NULL)
		throw exceptions::unfetched_object();

	return (m_header);
}


void POP3Message::extract(utility::outputStream& os,
	utility::progressListener* progress, const int start,
	const int length, const bool /* peek */) const
{
	ref <const POP3Folder> folder = m_folder.acquire();

	if (!folder)
		throw exceptions::illegal_state("Folder closed");
	else if (!folder->getStore())
		throw exceptions::illegal_state("Store disconnected");

	if (start != 0 && length != -1)
		throw exceptions::partial_fetch_not_supported();

	// Emit the "RETR" command
	ref <POP3Store> store = folder.constCast <POP3Folder>()->m_store.acquire();

	POP3Command::RETR(m_num)->send(store->getConnection());

	try
	{
		POP3Response::readLargeResponse
			(store->getConnection(), os, progress, m_size);
	}
	catch (exceptions::command_error& e)
	{
		throw exceptions::command_error("RETR", e.response());
	}
}


void POP3Message::extractPart
	(ref <const messagePart> /* p */, utility::outputStream& /* os */,
	 utility::progressListener* /* progress */,
	 const int /* start */, const int /* length */,
	 const bool /* peek */) const
{
	throw exceptions::operation_not_supported();
}


void POP3Message::fetchPartHeader(ref <messagePart> /* p */)
{
	throw exceptions::operation_not_supported();
}


void POP3Message::fetch(ref <POP3Folder> msgFolder, const int options)
{
	ref <POP3Folder> folder = m_folder.acquire();

	if (folder != msgFolder)
		throw exceptions::folder_not_found();

	// FETCH_STRUCTURE and FETCH_FLAGS are not supported by POP3.
	if (options & (folder::FETCH_STRUCTURE | folder::FETCH_FLAGS))
		throw exceptions::operation_not_supported();

	// Check for the real need to fetch the full header
	static const int optionsRequiringHeader =
		folder::FETCH_ENVELOPE | folder::FETCH_CONTENT_INFO |
		folder::FETCH_FULL_HEADER | folder::FETCH_IMPORTANCE;

	if (!(options & optionsRequiringHeader))
		return;

	// No need to differenciate between FETCH_ENVELOPE,
	// FETCH_CONTENT_INFO, ... since POP3 only permits to
	// retrieve the whole header and not fields in particular.

	// Emit the "TOP" command
	ref <POP3Store> store = folder->m_store.acquire();

	POP3Command::TOP(m_num, 0)->send(store->getConnection());

	try
	{
		string buffer;
		utility::outputStreamStringAdapter bufferStream(buffer);

		POP3Response::readLargeResponse(store->getConnection(),
			bufferStream, /* progress */ NULL, /* predictedSize */ 0);

		m_header = vmime::create <header>();
		m_header->parse(buffer);
	}
	catch (exceptions::command_error& e)
	{
		throw exceptions::command_error("TOP", e.response());
	}
}


void POP3Message::setFlags(const int /* flags */, const int /* mode */)
{
	throw exceptions::operation_not_supported();
}


ref <vmime::message> POP3Message::getParsedMessage()
{
	std::ostringstream oss;
	utility::outputStreamAdapter os(oss);

	extract(os);

	vmime::ref <vmime::message> msg = vmime::create <vmime::message>();
	msg->parse(oss.str());

	return msg;
}


} // pop3
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3

