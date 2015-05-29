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


#include "../vmime/net/imap/IMAPFolder.hpp"

#include "../vmime/net/imap/IMAPStore.hpp"
#include "../vmime/net/imap/IMAPParser.hpp"
#include "../vmime/net/imap/IMAPMessage.hpp"
#include "../vmime/net/imap/IMAPUtils.hpp"
#include "../vmime/net/imap/IMAPConnection.hpp"
#include "../vmime/net/imap/IMAPFolderStatus.hpp"

#include "../vmime/message.hpp"

#include "../vmime/exception.hpp"
#include "../vmime/utility/smartPtr.hpp"

#include "../vmime/utility/outputStreamAdapter.hpp"

#include <algorithm>
#include <sstream>


namespace vmime {
namespace net {
namespace imap {


IMAPFolder::IMAPFolder(const folder::path& path, ref <IMAPStore> store, const int type, const int flags)
	: m_store(store), m_connection(store->connection()), m_path(path),
	  m_name(path.isEmpty() ? folder::path::component("") : path.getLastComponent()), m_mode(-1),
	  m_open(false), m_type(type), m_flags(flags)
{
	store->registerFolder(this);

	m_status = vmime::create <IMAPFolderStatus>();
}


IMAPFolder::~IMAPFolder()
{
	ref <IMAPStore> store = m_store.acquire();

	if (store)
	{
		if (m_open)
			close(false);

		store->unregisterFolder(this);
	}
	else if (m_open)
	{
		m_connection = NULL;
		onClose();
	}
}


int IMAPFolder::getMode() const
{
	if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	return (m_mode);
}


int IMAPFolder::getType()
{
    // FIX by Elmue: Removed exception here.
    // It is not mandatory to open the folder to get TYPE_CONTAINS_MESSAGES or TYPE_CONTAINS_FOLDERS which are set 
    // in IMAPFolder::getFolders() and transmitted by the LIST command even for folders that are closed.

	// Root folder
	if (m_path.isEmpty())
	{
		return (TYPE_CONTAINS_FOLDERS);
	}
	else
	{
		if (m_type == TYPE_UNDEFINED)
			testExistAndGetType();

		return (m_type);
	}
}


int IMAPFolder::getFlags()
{
    // FIX by Elmue: Removed exception here.
    // It is not mandatory to open the folder to get FLAG_NO_OPEN or FLAG_CHILDREN which are set 
    // in IMAPFolder::getFolders() and transmitted by the LIST command even for folders that are closed.

    // Root folder
	if (m_path.isEmpty())
	{
		return (FLAG_CHILDREN | FLAG_NO_OPEN);
	}
	else
	{
		if (m_flags == FLAG_UNDEFINED)
			testExistAndGetType();

		return (m_flags);
	}
}


const folder::path::component IMAPFolder::getName() const
{
	return (m_name);
}


const folder::path IMAPFolder::getFullPath() const
{
	return (m_path);
}


void IMAPFolder::open(const int mode, bool failIfModeIsNotAvailable)
{
	ref <IMAPStore> store = m_store.acquire();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");

	// Ensure this folder is not already open in the same session
	for (std::list <IMAPFolder*>::iterator it = store->m_folders.begin() ;
	     it != store->m_folders.end() ; ++it)
	{
		if ((*it) != this && (*it)->getFullPath() == m_path)
			throw exceptions::folder_already_open();
	}

	// Open a connection for this folder
	ref <IMAPConnection> connection =
		vmime::create <IMAPConnection>(store, store->getAuthenticator());

	try
	{
		connection->connect();

		// Emit the "SELECT" command
		//
		// Example:  C: A142 SELECT INBOX
		//           S: * 172 EXISTS
		//           S: * 1 RECENT
		//           S: * OK [UNSEEN 12] Message 12 is first unseen
		//           S: * OK [UIDVALIDITY 3857529045] UIDs valid
		//           S: * FLAGS (\Answered \Flagged \Deleted \Seen \Draft)
		//           S: * OK [PERMANENTFLAGS (\Deleted \Seen \*)] Limited
		//           S: A142 OK [READ-WRITE] SELECT completed

		std::ostringstream oss;

		if (mode == MODE_READ_ONLY)
			oss << "EXAMINE ";
		else
			oss << "SELECT ";

		oss << IMAPUtils::quoteString(IMAPUtils::pathToString
				(connection->hierarchySeparator(), getFullPath()));

		if (m_connection->hasCapability("CONDSTORE"))
			oss << " (CONDSTORE)";

		connection->send(true, oss.str(), true);

		// Read the response
		utility::auto_ptr <IMAPParser::response> resp(connection->readResponse());

		if (resp->isBad() || resp->response_done()->response_tagged()->
				resp_cond_state()->status() != IMAPParser::resp_cond_state::OK)
		{
			throw exceptions::command_error("SELECT",
				resp->getErrorLog(), "bad response");
		}

		const std::vector <IMAPParser::continue_req_or_response_data*>& respDataList =
			resp->continue_req_or_response_data();

		for (std::vector <IMAPParser::continue_req_or_response_data*>::const_iterator
		     it = respDataList.begin() ; it != respDataList.end() ; ++it)
		{
			if ((*it)->response_data() == NULL)
			{
				throw exceptions::command_error("SELECT",
					resp->getErrorLog(), "invalid response");
			}

			const IMAPParser::response_data* responseData = (*it)->response_data();

			// OK Untagged responses: UNSEEN, PERMANENTFLAGS, UIDVALIDITY (optional)
			if (responseData->resp_cond_state())
			{
				const IMAPParser::resp_text_code* code =
					responseData->resp_cond_state()->resp_text()->resp_text_code();

				if (code != NULL)
				{
					switch (code->type())
					{
					case IMAPParser::resp_text_code::NOMODSEQ:

						connection->disableMODSEQ();
						break;

					default:

						break;
					}
				}
			}
			// Untagged responses: FLAGS, EXISTS, RECENT (required)
			else if (responseData->mailbox_data())
			{
				switch (responseData->mailbox_data()->type())
				{
				default: break;

				case IMAPParser::mailbox_data::FLAGS:
				{
					m_type = IMAPUtils::folderTypeFromFlags
						(responseData->mailbox_data()->mailbox_flag_list());

					m_flags = IMAPUtils::folderFlagsFromFlags
						(responseData->mailbox_data()->mailbox_flag_list());

					break;
				}

				}
			}
		}

		processStatusUpdate(resp);

		// Check for access mode (read-only or read-write)
		const IMAPParser::resp_text_code* respTextCode = resp->response_done()->
			response_tagged()->resp_cond_state()->resp_text()->resp_text_code();

		if (respTextCode)
		{
			const int openMode =
				(respTextCode->type() == IMAPParser::resp_text_code::READ_WRITE)
					? MODE_READ_WRITE : MODE_READ_ONLY;

			if (failIfModeIsNotAvailable &&
			    mode == MODE_READ_WRITE && openMode == MODE_READ_ONLY)
			{
				throw exceptions::operation_not_supported();
			}
		}


		m_connection = connection;
		m_open = true;
		m_mode = mode;
	}
	catch (std::exception&)
	{
		throw;
	}
}


void IMAPFolder::close(const bool expunge)
{
	ref <IMAPStore> store = m_store.acquire();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");

	if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	ref <IMAPConnection> oldConnection = m_connection;

	// Emit the "CLOSE" command to expunge messages marked
	// as deleted (this is fastest than "EXPUNGE")
	if (expunge)
	{
		if (m_mode == MODE_READ_ONLY)
			throw exceptions::operation_not_supported();

		oldConnection->send(true, "CLOSE", true);
	}

	// Close this folder connection
	oldConnection->disconnect();

	// Now use default store connection
	m_connection = m_store.acquire()->connection();

	m_open = false;
	m_mode = -1;

	m_status = vmime::create <IMAPFolderStatus>();

	onClose();
}


void IMAPFolder::onClose()
{
	for (std::vector <IMAPMessage*>::iterator it = m_messages.begin() ;
	     it != m_messages.end() ; ++it)
	{
		(*it)->onFolderClosed();
	}

	m_messages.clear();
}


void IMAPFolder::create(const int type)
{
	ref <IMAPStore> store = m_store.acquire();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");
	else if (isOpen())
		throw exceptions::illegal_state("Folder is open");
	else if (exists())
		throw exceptions::illegal_state("Folder already exists");
	else if (!store->isValidFolderName(m_name))
		throw exceptions::invalid_folder_name();

	// Emit the "CREATE" command
	//
	// Example:   C: A003 CREATE owatagusiam/
	//            S: A003 OK CREATE completed
	//            C: A004 CREATE owatagusiam/blurdybloop
	//            S: A004 OK CREATE completed

	string mailbox = IMAPUtils::pathToString
		(m_connection->hierarchySeparator(), getFullPath());

	if (type & TYPE_CONTAINS_FOLDERS)
		mailbox += m_connection->hierarchySeparator();

	std::ostringstream oss;
	oss << "CREATE " << IMAPUtils::quoteString(mailbox);

	m_connection->send(true, oss.str(), true);


	utility::auto_ptr <IMAPParser::response> resp(m_connection->readResponse());

	if (resp->isBad() || resp->response_done()->response_tagged()->
			resp_cond_state()->status() != IMAPParser::resp_cond_state::OK)
	{
		throw exceptions::command_error("CREATE",
			resp->getErrorLog(), "bad response");
	}

	// Notify folder created
	ref <events::folderEvent> event =
		vmime::create <events::folderEvent>
			(thisRef().dynamicCast <folder>(),
			 events::folderEvent::TYPE_CREATED, m_path, m_path);

	notifyFolder(event);
}


void IMAPFolder::destroy()
{
	ref <IMAPStore> store = m_store.acquire();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");

	if (isOpen())
		throw exceptions::illegal_state("Folder is open");

	const string mailbox = IMAPUtils::pathToString
		(m_connection->hierarchySeparator(), getFullPath());

	std::ostringstream oss;
	oss << "DELETE " << IMAPUtils::quoteString(mailbox);

	m_connection->send(true, oss.str(), true);


	utility::auto_ptr <IMAPParser::response> resp(m_connection->readResponse());

	if (resp->isBad() || resp->response_done()->response_tagged()->
			resp_cond_state()->status() != IMAPParser::resp_cond_state::OK)
	{
		throw exceptions::command_error("DELETE",
			resp->getErrorLog(), "bad response");
	}

	// Notify folder deleted
	ref <events::folderEvent> event =
		vmime::create <events::folderEvent>
			(thisRef().dynamicCast <folder>(),
			 events::folderEvent::TYPE_DELETED, m_path, m_path);

	notifyFolder(event);
}


bool IMAPFolder::exists()
{
	ref <IMAPStore> store = m_store.acquire();

	if (!isOpen() && !store)
		throw exceptions::illegal_state("Store disconnected");

	return (testExistAndGetType() != TYPE_UNDEFINED);
}


int IMAPFolder::testExistAndGetType()
{
	m_type = TYPE_UNDEFINED;

	// To test whether a folder exists, we simple list it using
	// the "LIST" command, and there should be one unique mailbox
	// with this name...
	//
	// Eg. Test whether '/foo/bar' exists
	//
	//     C: a005 list "" foo/bar
	//     S: * LIST (\NoSelect) "/" foo/bar
	//     S: a005 OK LIST completed
	//
	// ==> OK, exists
	//
	// Test whether '/foo/bar/zap' exists
	//
	//     C: a005 list "" foo/bar/zap
	//     S: a005 OK LIST completed
	//
	// ==> NO, does not exist

	std::ostringstream oss;
	oss << "LIST \"\" ";
	oss << IMAPUtils::quoteString(IMAPUtils::pathToString
		(m_connection->hierarchySeparator(), getFullPath()));

	m_connection->send(true, oss.str(), true);


	utility::auto_ptr <IMAPParser::response> resp(m_connection->readResponse());

	if (resp->isBad() || resp->response_done()->response_tagged()->
			resp_cond_state()->status() != IMAPParser::resp_cond_state::OK)
	{
		throw exceptions::command_error("LIST",
			resp->getErrorLog(), "bad response");
	}

	// Check whether the result mailbox list contains this folder
	const std::vector <IMAPParser::continue_req_or_response_data*>& respDataList =
		resp->continue_req_or_response_data();

	for (std::vector <IMAPParser::continue_req_or_response_data*>::const_iterator
	     it = respDataList.begin() ; it != respDataList.end() ; ++it)
	{
		if ((*it)->response_data() == NULL)
		{
			throw exceptions::command_error("LIST",
				resp->getErrorLog(), "invalid response");
		}

		const IMAPParser::mailbox_data* mailboxData =
			(*it)->response_data()->mailbox_data();

		// We are only interested in responses of type "LIST"
		if (mailboxData != NULL && mailboxData->type() == IMAPParser::mailbox_data::LIST)
		{
			// Get the folder type/flags at the same time
			m_type = IMAPUtils::folderTypeFromFlags
				(mailboxData->mailbox_list()->mailbox_flag_list());

			m_flags = IMAPUtils::folderFlagsFromFlags
				(mailboxData->mailbox_list()->mailbox_flag_list());
		}
	}

	return (m_type);
}


bool IMAPFolder::isOpen() const
{
	return (m_open);
}


ref <message> IMAPFolder::getMessage(const int num)
{
	if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	if (num < 1 || num > (int)m_status->getMessageCount())
		throw exceptions::message_not_found();

	return vmime::create <IMAPMessage>(thisRef().dynamicCast <IMAPFolder>(), num);
}


std::vector <ref <message> > IMAPFolder::getMessages(const messageSet& msgs)
{
	if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	if (msgs.isEmpty())
		return std::vector <ref <message> >();

	std::vector <ref <message> > messages;

	if (msgs.isNumberSet())
	{
		const std::vector <int> numbers = IMAPUtils::messageSetToNumberList(msgs);

		ref <IMAPFolder> thisFolder = thisRef().dynamicCast <IMAPFolder>();

		for (std::vector <int>::const_iterator it = numbers.begin() ; it != numbers.end() ; ++it)
			messages.push_back(vmime::create <IMAPMessage>(thisFolder, *it));
	}
	else if (msgs.isUIDSet())
	{
		//     C: . UID FETCH uuuu1,uuuu2,uuuu3 UID
		//     S: * nnnn1 FETCH (UID uuuu1)
		//     S: * nnnn2 FETCH (UID uuuu2)
		//     S: * nnnn3 FETCH (UID uuuu3)
		//     S: . OK UID FETCH completed

		// Prepare command and arguments
		std::ostringstream cmd;
		cmd.imbue(std::locale::classic());

		cmd << "UID FETCH " << IMAPUtils::messageSetToSequenceSet(msgs) << " UID";

		// Send the request
		m_connection->send(true, cmd.str(), true);

		// Get the response
		utility::auto_ptr <IMAPParser::response> resp(m_connection->readResponse());

		if (resp->isBad() || resp->response_done()->response_tagged()->
				resp_cond_state()->status() != IMAPParser::resp_cond_state::OK)
		{
			throw exceptions::command_error("UID FETCH ... UID", resp->getErrorLog(), "bad response");
		}

		// Process the response
		const std::vector <IMAPParser::continue_req_or_response_data*>& respDataList =
			resp->continue_req_or_response_data();

		for (std::vector <IMAPParser::continue_req_or_response_data*>::const_iterator
			 it = respDataList.begin() ; it != respDataList.end() ; ++it)
		{
			if ((*it)->response_data() == NULL)
			{
				throw exceptions::command_error("UID FETCH ... UID",
					resp->getErrorLog(), "invalid response");
			}

			const IMAPParser::message_data* messageData =
				(*it)->response_data()->message_data();

			// We are only interested in responses of type "FETCH"
			if (messageData == NULL || messageData->type() != IMAPParser::message_data::FETCH)
				continue;

			// Get Process fetch response for this message
			const int msgNum = static_cast <int>(messageData->number());
			message::uid msgUID;

			// Find UID in message attributes
			const std::vector <IMAPParser::msg_att_item*> atts = messageData->msg_att()->items();

			for (std::vector <IMAPParser::msg_att_item*>::const_iterator
				 it = atts.begin() ; it != atts.end() ; ++it)
			{
				if ((*it)->type() == IMAPParser::msg_att_item::UID)
				{
					msgUID = (*it)->unique_id()->value();
					break;
				}
			}

			if (!msgUID.empty())
			{
				ref <IMAPFolder> thisFolder = thisRef().dynamicCast <IMAPFolder>();
				messages.push_back(vmime::create <IMAPMessage>(thisFolder, msgNum, msgUID));
			}
		}
	}

	return messages;
}


int IMAPFolder::getMessageCount()
{
	if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	return m_status->getMessageCount();
}


vmime_uint32 IMAPFolder::getUIDValidity() const
{
	if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	return m_status->getUIDValidity();
}


vmime_uint64 IMAPFolder::getHighestModSequence() const
{
	if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	return m_status->getHighestModSeq();
}


ref <folder> IMAPFolder::getFolder(const folder::path::component& name)
{
	ref <IMAPStore> store = m_store.acquire();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");

	return vmime::create <IMAPFolder>(m_path / name, store);
}


std::vector <ref <folder> > IMAPFolder::getFolders(const bool recursive)
{
	ref <IMAPStore> store = m_store.acquire();

	if (!isOpen() && !store)
		throw exceptions::illegal_state("Store disconnected");

	// Eg. List folders in '/foo/bar'
	//
	//     C: a005 list "foo/bar" *
	//     S: * LIST (\NoSelect) "/" foo/bar
	//     S: * LIST (\NoInferiors) "/" foo/bar/zap
	//     S: a005 OK LIST completed

	std::ostringstream oss;
	oss << "LIST ";

	const string pathString = IMAPUtils::pathToString
		(m_connection->hierarchySeparator(), getFullPath());

	if (recursive)
	{
		oss << IMAPUtils::quoteString(pathString);
		oss << " *";
	}
	else
	{
		if (pathString.empty()) // don't add sep for root folder
			oss << "\"\"";
		else
			oss << IMAPUtils::quoteString(pathString + m_connection->hierarchySeparator());

		oss << " %";
	}

	m_connection->send(true, oss.str(), true);


	utility::auto_ptr <IMAPParser::response> resp(m_connection->readResponse());

	if (resp->isBad() || resp->response_done()->response_tagged()->
			resp_cond_state()->status() != IMAPParser::resp_cond_state::OK)
	{
		throw exceptions::command_error("LIST", resp->getErrorLog(), "bad response");
	}

	const std::vector <IMAPParser::continue_req_or_response_data*>& respDataList =
		resp->continue_req_or_response_data();


	std::vector <ref <folder> > v;

	for (std::vector <IMAPParser::continue_req_or_response_data*>::const_iterator
	     it = respDataList.begin() ; it != respDataList.end() ; ++it)
	{
		if ((*it)->response_data() == NULL)
		{
			throw exceptions::command_error("LIST",
				resp->getErrorLog(), "invalid response");
		}

		const IMAPParser::mailbox_data* mailboxData =
			(*it)->response_data()->mailbox_data();

		if (mailboxData == NULL || mailboxData->type() != IMAPParser::mailbox_data::LIST)
			continue;

		// Get folder path
		const class IMAPParser::mailbox* mailbox =
			mailboxData->mailbox_list()->mailbox();

		folder::path path = IMAPUtils::stringToPath
			(mailboxData->mailbox_list()->quoted_char(), mailbox->name());

		if (recursive || m_path.isDirectParentOf(path))
		{
			// Append folder to list
			const class IMAPParser::mailbox_flag_list* mailbox_flag_list =
				mailboxData->mailbox_list()->mailbox_flag_list();

			v.push_back(vmime::create <IMAPFolder>(path, store,
				IMAPUtils::folderTypeFromFlags(mailbox_flag_list),
				IMAPUtils::folderFlagsFromFlags(mailbox_flag_list)));
		}
	}

	return (v);
}


void IMAPFolder::fetchMessages(std::vector <ref <message> >& msg, const int options,
                               utility::progressListener* progress)
{
	ref <IMAPStore> store = m_store.acquire();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	// Build message numbers list
	std::vector <int> list;
	list.reserve(msg.size());

	std::map <int, ref <IMAPMessage> > numberToMsg;

	for (std::vector <ref <message> >::iterator it = msg.begin() ; it != msg.end() ; ++it)
	{
		list.push_back((*it)->getNumber());
		numberToMsg[(*it)->getNumber()] = (*it).dynamicCast <IMAPMessage>();
	}

	// Send the request
	const string command = IMAPUtils::buildFetchRequest
		(m_connection, messageSet::byNumber(list), options);

	m_connection->send(true, command, true);

	// Get the response
	utility::auto_ptr <IMAPParser::response> resp(m_connection->readResponse());

	if (resp->isBad() || resp->response_done()->response_tagged()->
		resp_cond_state()->status() != IMAPParser::resp_cond_state::OK)
	{
		throw exceptions::command_error("FETCH",
			resp->getErrorLog(), "bad response");
	}

	const std::vector <IMAPParser::continue_req_or_response_data*>& respDataList =
		resp->continue_req_or_response_data();

	const int total = msg.size();
	int current = 0;

	if (progress)
		progress->start(total);

	try
	{
		for (std::vector <IMAPParser::continue_req_or_response_data*>::const_iterator
		     it = respDataList.begin() ; it != respDataList.end() ; ++it)
		{
			if ((*it)->response_data() == NULL)
			{
				throw exceptions::command_error("FETCH",
					resp->getErrorLog(), "invalid response");
			}

			const IMAPParser::message_data* messageData =
				(*it)->response_data()->message_data();

			// We are only interested in responses of type "FETCH"
			if (messageData == NULL || messageData->type() != IMAPParser::message_data::FETCH)
				continue;

			// Process fetch response for this message
			const int num = static_cast <int>(messageData->number());

			std::map <int, ref <IMAPMessage> >::iterator msg = numberToMsg.find(num);

			if (msg != numberToMsg.end())
			{
				(*msg).second->processFetchResponse(options, messageData);

				if (progress)
					progress->progress(++current, total);
			}
		}
	}
	catch (...)
	{
		if (progress)
			progress->stop(total);

		throw;
	}

	if (progress)
		progress->stop(total);

	processStatusUpdate(resp);
}


void IMAPFolder::fetchMessage(ref <message> msg, const int options)
{
	std::vector <ref <message> > msgs;
	msgs.push_back(msg);

	fetchMessages(msgs, options, /* progress */ NULL);
}


int IMAPFolder::getFetchCapabilities() const
{
	return (FETCH_ENVELOPE | FETCH_CONTENT_INFO | FETCH_STRUCTURE |
	        FETCH_FLAGS | FETCH_SIZE | FETCH_FULL_HEADER | FETCH_UID |
	        FETCH_IMPORTANCE);
}


ref <folder> IMAPFolder::getParent()
{
	if (m_path.isEmpty())
		return NULL;
	else
		return vmime::create <IMAPFolder>(m_path.getParent(), m_store.acquire());
}


ref <const store> IMAPFolder::getStore() const
{
	return m_store.acquire();
}


ref <store> IMAPFolder::getStore()
{
	return m_store.acquire();
}


void IMAPFolder::registerMessage(IMAPMessage* msg)
{
	m_messages.push_back(msg);
}


void IMAPFolder::unregisterMessage(IMAPMessage* msg)
{
	std::vector <IMAPMessage*>::iterator it =
		std::find(m_messages.begin(), m_messages.end(), msg);

	if (it != m_messages.end())
		m_messages.erase(it);
}


void IMAPFolder::onStoreDisconnected()
{
	m_store = NULL;
}


void IMAPFolder::deleteMessages(const messageSet& msgs)
{
	ref <IMAPStore> store = m_store.acquire();

	if (msgs.isEmpty())
		throw exceptions::invalid_argument();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");
	else if (m_mode == MODE_READ_ONLY)
		throw exceptions::illegal_state("Folder is read-only");

	// Build the request text
	std::ostringstream command;
	command.imbue(std::locale::classic());

	if (msgs.isUIDSet())
		command << "UID STORE " << IMAPUtils::messageSetToSequenceSet(msgs);
	else
		command << "STORE " << IMAPUtils::messageSetToSequenceSet(msgs);

	command << " +FLAGS (\\Deleted)";

	// Send the request
	m_connection->send(true, command.str(), true);

	// Get the response
	utility::auto_ptr <IMAPParser::response> resp(m_connection->readResponse());

	if (resp->isBad() || resp->response_done()->response_tagged()->
		resp_cond_state()->status() != IMAPParser::resp_cond_state::OK)
	{
		throw exceptions::command_error("STORE",
			resp->getErrorLog(), "bad response");
	}

	processStatusUpdate(resp);
}


void IMAPFolder::setMessageFlags(const messageSet& msgs, const int flags, const int mode)
{
	// Build the request text
	std::ostringstream command;
	command.imbue(std::locale::classic());

	if (msgs.isUIDSet())
		command << "UID STORE " << IMAPUtils::messageSetToSequenceSet(msgs);
	else
		command << "STORE " << IMAPUtils::messageSetToSequenceSet(msgs);

	switch (mode)
	{
	case message::FLAG_MODE_ADD:    command << " +FLAGS "; break;
	case message::FLAG_MODE_REMOVE: command << " -FLAGS "; break;
	default:
	case message::FLAG_MODE_SET:    command << " FLAGS "; break;
	}

	const string flagList = IMAPUtils::messageFlagList(flags);

	if (!flagList.empty())
	{
		command << flagList;

		// Send the request
		m_connection->send(true, command.str(), true);

		// Get the response
		utility::auto_ptr <IMAPParser::response> resp(m_connection->readResponse());

		if (resp->isBad() || resp->response_done()->response_tagged()->
			resp_cond_state()->status() != IMAPParser::resp_cond_state::OK)
		{
			throw exceptions::command_error("STORE",
				resp->getErrorLog(), "bad response");
		}

		processStatusUpdate(resp);
	}
}


void IMAPFolder::addMessage(ref <vmime::message> msg, const int flags,
                            vmime::datetime* date, utility::progressListener* progress)
{
	std::ostringstream oss;
	utility::outputStreamAdapter ossAdapter(oss);

	msg->generate(ossAdapter);

	const string& str = oss.str();
	utility::inputStreamStringAdapter strAdapter(str);

	addMessage(strAdapter, str.length(), flags, date, progress);
}


void IMAPFolder::addMessage(utility::inputStream& is, const int size, const int flags,
                            vmime::datetime* date, utility::progressListener* progress)
{
	ref <IMAPStore> store = m_store.acquire();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");
	else if (m_mode == MODE_READ_ONLY)
		throw exceptions::illegal_state("Folder is read-only");

	// Build the request text
	std::ostringstream command;
	command.imbue(std::locale::classic());

	command << "APPEND " << IMAPUtils::quoteString(IMAPUtils::pathToString
			(m_connection->hierarchySeparator(), getFullPath())) << ' ';

	const string flagList = IMAPUtils::messageFlagList(flags);

	if (flags != message::FLAG_UNDEFINED && !flagList.empty())
	{
		command << flagList;
		command << ' ';
	}

	if (date != NULL)
	{
		command << IMAPUtils::dateTime(*date);
		command << ' ';
	}

	command << '{' << size << '}';

	// Send the request
	m_connection->send(true, command.str(), true);

	// Get the response
	utility::auto_ptr <IMAPParser::response> resp(m_connection->readResponse());

	bool ok = false;
	const std::vector <IMAPParser::continue_req_or_response_data*>& respList
		= resp->continue_req_or_response_data();

	for (std::vector <IMAPParser::continue_req_or_response_data*>::const_iterator
	     it = respList.begin() ; !ok && (it != respList.end()) ; ++it)
	{
		if ((*it)->continue_req())
			ok = true;
	}

	if (!ok)
	{
		throw exceptions::command_error("APPEND",
			resp->getErrorLog(), "bad response");
	}

	// Send message data
	const int total = size;
	int current = 0;

	if (progress)
		progress->start(total);

	const socket::size_type blockSize = std::min(is.getBlockSize(),
		static_cast <size_t>(m_connection->getSocket()->getBlockSize()));

	std::vector <char> vbuffer(blockSize);
	char* buffer = &vbuffer.front();

	while (!is.eof())
	{
		// Read some data from the input stream
		const int read = is.read(buffer, sizeof(buffer));
		current += read;

		// Put read data into socket output stream
		m_connection->sendRaw(buffer, read);

		// Notify progress
		if (progress)
			progress->progress(current, total);
	}

	m_connection->send(false, "", true);

	if (progress)
		progress->stop(total);

	// Get the response
	utility::auto_ptr <IMAPParser::response> finalResp(m_connection->readResponse());

	if (finalResp->isBad() || finalResp->response_done()->response_tagged()->
		resp_cond_state()->status() != IMAPParser::resp_cond_state::OK)
	{
		throw exceptions::command_error("APPEND",
			resp->getErrorLog(), "bad response");
	}

	processStatusUpdate(resp);
}


void IMAPFolder::expunge()
{
	ref <IMAPStore> store = m_store.acquire();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");
	else if (m_mode == MODE_READ_ONLY)
		throw exceptions::illegal_state("Folder is read-only");

	// Send the request
	m_connection->send(true, "EXPUNGE", true);

	// Get the response
	utility::auto_ptr <IMAPParser::response> resp(m_connection->readResponse());

	if (resp->isBad() || resp->response_done()->response_tagged()->
		resp_cond_state()->status() != IMAPParser::resp_cond_state::OK)
	{
		throw exceptions::command_error("EXPUNGE",
			resp->getErrorLog(), "bad response");
	}

	processStatusUpdate(resp);
}


void IMAPFolder::rename(const folder::path& newPath)
{
	ref <IMAPStore> store = m_store.acquire();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");
	else if (m_path.isEmpty() || newPath.isEmpty())
		throw exceptions::illegal_operation("Cannot rename root folder");
	else if (m_path.getSize() == 1 && m_name.getBuffer() == "INBOX")
		throw exceptions::illegal_operation("Cannot rename 'INBOX' folder");
	else if (!store->isValidFolderName(newPath.getLastComponent()))
		throw exceptions::invalid_folder_name();

	// Build the request text
	std::ostringstream command;
	command.imbue(std::locale::classic());

	command << "RENAME ";
	command << IMAPUtils::quoteString(IMAPUtils::pathToString
			(m_connection->hierarchySeparator(), getFullPath())) << " ";
	command << IMAPUtils::quoteString(IMAPUtils::pathToString
			(m_connection->hierarchySeparator(), newPath));

	// Send the request
	m_connection->send(true, command.str(), true);

	// Get the response
	utility::auto_ptr <IMAPParser::response> resp(m_connection->readResponse());

	if (resp->isBad() || resp->response_done()->response_tagged()->
		resp_cond_state()->status() != IMAPParser::resp_cond_state::OK)
	{
		throw exceptions::command_error("RENAME",
			resp->getErrorLog(), "bad response");
	}

	// Notify folder renamed
	folder::path oldPath(m_path);

	m_path = newPath;
	m_name = newPath.getLastComponent();

	ref <events::folderEvent> event =
		vmime::create <events::folderEvent>
			(thisRef().dynamicCast <folder>(),
			 events::folderEvent::TYPE_RENAMED, oldPath, newPath);

	notifyFolder(event);

	// Notify sub-folders
	for (std::list <IMAPFolder*>::iterator it = store->m_folders.begin() ;
	     it != store->m_folders.end() ; ++it)
	{
		if ((*it) != this && oldPath.isParentOf((*it)->getFullPath()))
		{
			folder::path oldPath((*it)->m_path);

			(*it)->m_path.renameParent(oldPath, newPath);

			ref <events::folderEvent> event =
				vmime::create <events::folderEvent>
					((*it)->thisRef().dynamicCast <folder>(),
					 events::folderEvent::TYPE_RENAMED, oldPath, (*it)->m_path);

			(*it)->notifyFolder(event);
		}
	}

	processStatusUpdate(resp);
}


void IMAPFolder::copyMessages(const folder::path& dest, const messageSet& set)
{
	ref <IMAPStore> store = m_store.acquire();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	// Build the request text
	std::ostringstream command;
	command.imbue(std::locale::classic());

	command << "COPY " << IMAPUtils::messageSetToSequenceSet(set) << " ";
	command << IMAPUtils::quoteString(IMAPUtils::pathToString
			(m_connection->hierarchySeparator(), dest));

	// Send the request
	m_connection->send(true, command.str(), true);

	// Get the response
	utility::auto_ptr <IMAPParser::response> resp(m_connection->readResponse());

	if (resp->isBad() || resp->response_done()->response_tagged()->
		resp_cond_state()->status() != IMAPParser::resp_cond_state::OK)
	{
		throw exceptions::command_error("COPY",
			resp->getErrorLog(), "bad response");
	}

	processStatusUpdate(resp);
}


void IMAPFolder::status(int& count, int& unseen)
{
	count = 0;
	unseen = 0;

	ref <folderStatus> status = getStatus();

	count = status->getMessageCount();
	unseen = status->getUnseenCount();
}


ref <folderStatus> IMAPFolder::getStatus()
{
	ref <IMAPStore> store = m_store.acquire();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");

	// Build the request text
	std::ostringstream command;
	command.imbue(std::locale::classic());

	command << "STATUS ";
	command << IMAPUtils::quoteString(IMAPUtils::pathToString
			(m_connection->hierarchySeparator(), getFullPath()));
	command << " (";

	command << "MESSAGES" << ' ' << "UNSEEN" << ' ' << "UIDNEXT" << ' ' << "UIDVALIDITY";

	if (m_connection->hasCapability("CONDSTORE"))
		command << ' ' << "HIGHESTMODSEQ";

	command << ")";

	// Send the request
	m_connection->send(true, command.str(), true);

	// Get the response
	utility::auto_ptr <IMAPParser::response> resp(m_connection->readResponse());

	if (resp->isBad() || resp->response_done()->response_tagged()->
		resp_cond_state()->status() != IMAPParser::resp_cond_state::OK)
	{
		throw exceptions::command_error("STATUS",
			resp->getErrorLog(), "bad response");
	}

	const std::vector <IMAPParser::continue_req_or_response_data*>& respDataList =
		resp->continue_req_or_response_data();

	for (std::vector <IMAPParser::continue_req_or_response_data*>::const_iterator
	     it = respDataList.begin() ; it != respDataList.end() ; ++it)
	{
		if ((*it)->response_data() != NULL)
		{
			const IMAPParser::response_data* responseData = (*it)->response_data();

			if (responseData->mailbox_data() &&
				responseData->mailbox_data()->type() == IMAPParser::mailbox_data::STATUS)
			{
				ref <IMAPFolderStatus> status = vmime::create <IMAPFolderStatus>();
				status->updateFromResponse(responseData->mailbox_data());

				m_status->updateFromResponse(responseData->mailbox_data());

				return status;
			}
		}
	}

	throw exceptions::command_error("STATUS",
		resp->getErrorLog(), "invalid response");
}


void IMAPFolder::noop()
{
	ref <IMAPStore> store = m_store.acquire();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");

	m_connection->send(true, "NOOP", true);

	utility::auto_ptr <IMAPParser::response> resp(m_connection->readResponse());

	if (resp->isBad() || resp->response_done()->response_tagged()->
			resp_cond_state()->status() != IMAPParser::resp_cond_state::OK)
	{
		throw exceptions::command_error("NOOP", resp->getErrorLog());
	}

	processStatusUpdate(resp);
}


std::vector <int> IMAPFolder::getMessageNumbersStartingOnUID(const message::uid& uid)
{
	std::vector<int> v;

	std::ostringstream command;
	command.imbue(std::locale::classic());

	command << "SEARCH UID " << uid << ":*";

	// Send the request
	m_connection->send(true, command.str(), true);

	// Get the response
	utility::auto_ptr <IMAPParser::response> resp(m_connection->readResponse());

	if (resp->isBad() ||
	    resp->response_done()->response_tagged()->resp_cond_state()->status() != IMAPParser::resp_cond_state::OK)
	{
		throw exceptions::command_error("SEARCH",
			resp->getErrorLog(), "bad response");
	}

	const std::vector <IMAPParser::continue_req_or_response_data*>& respDataList = resp->continue_req_or_response_data();

	for (std::vector <IMAPParser::continue_req_or_response_data*>::const_iterator
	     it = respDataList.begin() ; it != respDataList.end() ; ++it)
	{
		if ((*it)->response_data() == NULL)
		{
			throw exceptions::command_error("SEARCH",
				resp->getErrorLog(), "invalid response");
		}

		const IMAPParser::mailbox_data* mailboxData =
			(*it)->response_data()->mailbox_data();

		// We are only interested in responses of type "SEARCH"
		if (mailboxData == NULL ||
		    mailboxData->type() != IMAPParser::mailbox_data::SEARCH)
		{
			continue;
		}

		for (std::vector <IMAPParser::nz_number*>::const_iterator
				it = mailboxData->search_nz_number_list().begin() ;
		     it != mailboxData->search_nz_number_list().end();
		     ++it)
		{
			v.push_back((*it)->value());
		}
	}

	processStatusUpdate(resp);

	return v;
}


void IMAPFolder::processStatusUpdate(const IMAPParser::response* resp)
{
	std::vector <ref <events::event> > events;

	ref <IMAPFolderStatus> oldStatus = m_status->clone().dynamicCast <IMAPFolderStatus>();
	int expungedMessageCount = 0;

	// Process tagged response
	if (resp->response_done() && resp->response_done()->response_tagged() &&
	    resp->response_done()->response_tagged()
	    	->resp_cond_state()->resp_text()->resp_text_code())
	{
		const IMAPParser::resp_text_code* code =
			resp->response_done()->response_tagged()
				->resp_cond_state()->resp_text()->resp_text_code();

		m_status->updateFromResponse(code);
	}

	// Process untagged responses
	for (std::vector <IMAPParser::continue_req_or_response_data*>::const_iterator
	     it = resp->continue_req_or_response_data().begin() ;
	     it != resp->continue_req_or_response_data().end() ; ++it)
	{
		if ((*it)->response_data() && (*it)->response_data()->resp_cond_state() &&
		    (*it)->response_data()->resp_cond_state()->resp_text()->resp_text_code())
		{
			const IMAPParser::resp_text_code* code =
				(*it)->response_data()->resp_cond_state()->resp_text()->resp_text_code();

			m_status->updateFromResponse(code);
		}
		else if ((*it)->response_data() && (*it)->response_data()->mailbox_data())
		{
			m_status->updateFromResponse((*it)->response_data()->mailbox_data());
		}
		else if ((*it)->response_data() && (*it)->response_data()->message_data())
		{
			const IMAPParser::message_data* msgData = (*it)->response_data()->message_data();
			const int msgNumber = msgData->number();

			if ((*it)->response_data()->message_data()->type() == IMAPParser::message_data::FETCH)
			{
				// Message changed
				for (std::vector <IMAPMessage*>::iterator mit =
				     m_messages.begin() ; mit != m_messages.end() ; ++mit)
				{
					if ((*mit)->getNumber() == msgNumber)
						(*mit)->processFetchResponse(/* options */ 0, msgData);
				}

				events.push_back(vmime::create <events::messageChangedEvent>
					(thisRef().dynamicCast <folder>(),
					 events::messageChangedEvent::TYPE_FLAGS,
					 std::vector <int>(1, msgNumber)));
			}
			else if ((*it)->response_data()->message_data()->type() == IMAPParser::message_data::EXPUNGE)
			{
				// A message has been expunged, renumber messages
				for (std::vector <IMAPMessage*>::iterator jt =
				     m_messages.begin() ; jt != m_messages.end() ; ++jt)
				{
					if ((*jt)->getNumber() == msgNumber)
						(*jt)->setExpunged();
					else if ((*jt)->getNumber() > msgNumber)
						(*jt)->renumber((*jt)->getNumber() - 1);
				}

				events.push_back(vmime::create <events::messageCountEvent>
					(thisRef().dynamicCast <folder>(),
					 events::messageCountEvent::TYPE_REMOVED,
					 std::vector <int>(1, msgNumber)));

				expungedMessageCount++;
			}
		}
	}

	// New messages arrived
	if (m_status->getMessageCount() > oldStatus->getMessageCount() - expungedMessageCount)
	{
		std::vector <int> newMessageNumbers;

		for (int msgNumber = oldStatus->getMessageCount() - expungedMessageCount ;
		     msgNumber <= (int)m_status->getMessageCount() ; ++msgNumber)
		{
			newMessageNumbers.push_back(msgNumber);
		}

		events.push_back(vmime::create <events::messageCountEvent>
			(thisRef().dynamicCast <folder>(),
			 events::messageCountEvent::TYPE_ADDED,
			 newMessageNumbers));
	}

	// Dispatch notifications
	for (std::vector <ref <events::event> >::iterator evit =
	     events.begin() ; evit != events.end() ; ++evit)
	{
		notifyEvent(*evit);
	}
}


} // imap
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP

