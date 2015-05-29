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

#ifndef VMIME_NET_FOLDER_HPP_INCLUDED
#define VMIME_NET_FOLDER_HPP_INCLUDED


#include "../vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES


#include <vector>

#include "../vmime/types.hpp"
#include "../vmime/dateTime.hpp"

#include "../vmime/message.hpp"
#include "../vmime/net/message.hpp"
#include "../vmime/net/messageSet.hpp"
#include "../vmime/net/events.hpp"
#include "../vmime/net/folderStatus.hpp"

#include "../vmime/utility/path.hpp"
#include "../vmime/utility/stream.hpp"
#include "../vmime/utility/progressListener.hpp"


namespace vmime {
namespace net {


class store;


/** Abstract representation of a folder in a message store.
  */

class VMIME_EXPORT folder : public object
{
protected:

	folder(const folder&) : object() { }
	folder() { }

public:

	virtual ~folder() { }

	/** Type used for fully qualified path name of a folder.
	  */
	typedef vmime::utility::path path;


	/** Open mode.
	  */
	enum Modes
	{
		MODE_READ_ONLY,    /**< Read-only mode (no modification to folder or messages is possible). */
		MODE_READ_WRITE    /**< Full access mode (read and write). */
	};

	/** Folder types.
	  */
	enum Types
	{
		TYPE_CONTAINS_FOLDERS  = (1 << 0),   /**< Folder can contain folders. */
		TYPE_CONTAINS_MESSAGES = (1 << 1),   /**< Folder can contain messages. */

		TYPE_UNDEFINED = 9999                /**< Used internally (this should not be returned
		                                          by the type() function). */
	};

	/** Folder flags.
	  */
	enum Flags
	{
		FLAG_CHILDREN = (1 << 0),   /**< Folder contains subfolders. */
		FLAG_NO_OPEN  = (1 << 1),   /**< Folder cannot be open. */

		FLAG_UNDEFINED = 9999       /**< Used internally (this should not be returned
		                                 by the type() function). */
	};

	/** Return the type of this folder.
	  *
	  * @return folder type (see folder::Types)
	  */
	virtual int getType() = 0;

	/** Return the flags of this folder.
	  *
	  * @return folder flags (see folder::Flags)
	  */
	virtual int getFlags() = 0;

	/** Return the mode in which the folder has been open.
	  *
	  * @return folder opening mode (see folder::Modes)
	  */
	virtual int getMode() const = 0;

	/** Return the name of this folder.
	  *
	  * @return folder name
	  */
	virtual const folder::path::component getName() const = 0;

	/** Return the fully qualified path name of this folder.
	  *
	  * @return absolute path of the folder
	  */
	virtual const folder::path getFullPath() const = 0;

	/** Open this folder.
	  *
	  * @param mode open mode (see folder::Modes)
	  * @param failIfModeIsNotAvailable if set to false and if the requested mode
	  * is not available, a more restricted mode will be selected automatically.
	  * If set to true and if the requested mode is not available, the opening
	  * will fail.
	  * @throw exceptions::net_exception if an error occurs
	  * @throw exceptions::folder_already_open if the folder is already open
	  *        in the same session
	  */
	virtual void open(const int mode, bool failIfModeIsNotAvailable = false) = 0;

	/** Close this folder.
	  *
	  * @param expunge if set to true, deleted messages are expunged
	  * @throw exceptions::net_exception if an error occurs
	  */
	virtual void close(const bool expunge) = 0;

	/** Create this folder.
	  *
	  * @param type folder type (see folder::Types)
	  * @throw exceptions::net_exception if an error occurs
	  */
	virtual void create(const int type) = 0;

	/** Test whether this folder exists.
	  *
	  * @return true if the folder exists, false otherwise
	  */
	virtual bool exists() = 0;

	/** Delete this folder.
	  * The folder should be closed before attempting to delete it.
	  *
	  * @throw exceptions::net_exception if an error occurs
	  */
	virtual void destroy() = 0;

	/** Test whether this folder is open.
	  *
	  * @return true if the folder is open, false otherwise
	  */
	virtual bool isOpen() const = 0;

	/** Get a new reference to a message in this folder, given its number.
	  *
	  * @param num message sequence number
	  * @return a new object referencing the specified message
	  * @throw exceptions::net_exception if an error occurs
	  */
	virtual ref <message> getMessage(const int num) = 0;

	/** Get new references to messages in this folder, given either their
	  * sequence numbers or UIDs.
	  *
	  * To retrieve messages by their number, use:
	  * \code{.cpp}
	  *    // Get messages from sequence number 5 to sequence number 8 (including)
	  *    folder->getMessage(vmime::net::messageSet::byNumber(5, 8));
	  *
	  *    // Get all messages in the folder, starting from number 42
	  *    folder->getMessage(vmime::net::messageSet::byNumber(42, -1));
	  * \endcode
	  * Or, to retrieve messages by their UID, use:
	  * \code{.cpp}
	  *    // Get messages from UID 1000 to UID 1042 (including)
	  *    folder->getMessage(vmime::net::messageSet::byUID(1000, 1042));
	  *
	  *    // Get message with UID 1042
	  *    folder->getMessage(vmime::net::messageSet::byUID(1042));
	  *
	  *    // Get all messages in the folder, starting from UID 1000
	  *    folder->getMessage(vmime::net::messageSet::byUID(1000, "*"));
	  * \endcode
	  *
	  * @param msgs index set of messages to retrieve
	  * @return new objects referencing the specified messages
	  * @throw exceptions::net_exception if an error occurs
	  */
	virtual std::vector <ref <message> > getMessages(const messageSet& msgs) = 0;

	/** Return the number of messages in this folder.
	  *
	  * @return number of messages in the folder
	  */
	virtual int getMessageCount() = 0;

	/** Get a new reference to a sub-folder in this folder.
	  *
	  * @param name sub-folder name
	  * @return a new object referencing the specified folder
	  * @throw exceptions::net_exception if an error occurs
	  */
	virtual ref <folder> getFolder(const folder::path::component& name) = 0;

	/** Get the list of all sub-folders in this folder.
	  *
	  * @param recursive if set to true, all the descendant are returned.
	  * If set to false, only the direct children are returned.
	  * @return list of sub-folders
	  * @throw exceptions::net_exception if an error occurs
	  */
	virtual std::vector <ref <folder> > getFolders(const bool recursive = false) = 0;

	/** Rename (move) this folder to another location.
	  *
	  * @param newPath new path of the folder
	  * @throw exceptions::net_exception if an error occurs
	  */
	virtual void rename(const folder::path& newPath) = 0;

	/** Remove one or more messages from this folder.
	  *
	  * @param msgs index set of messages to delete
	  * @throw exceptions::net_exception if an error occurs
	  */
	virtual void deleteMessages(const messageSet& msgs) = 0;

	/** Change the flags for one or more messages in this folder.
	  *
	  * @param msgs index set of messages on which to set the flags
	  * @param flags set of flags (see message::Flags)
	  * @param mode indicate how to treat old and new flags (see message::FlagsModes)
	  * @throw exceptions::net_exception if an error occurs
	  */
	virtual void setMessageFlags(const messageSet& msgs, const int flags, const int mode = message::FLAG_MODE_SET) = 0;

	/** Add a message to this folder.
	  *
	  * @param msg message to add (data: header + body)
	  * @param flags flags for the new message
	  * @param date date/time for the new message (if NULL, the current time is used)
	  * @param progress progress listener, or NULL if not used
	  * @throw exceptions::net_exception if an error occurs
	  */
	virtual void addMessage(ref <vmime::message> msg, const int flags = message::FLAG_UNDEFINED, vmime::datetime* date = NULL, utility::progressListener* progress = NULL) = 0;

	/** Add a message to this folder.
	  *
	  * @param is message to add (data: header + body)
	  * @param size size of the message to add (in bytes)
	  * @param flags flags for the new message
	  * @param date date/time for the new message (if NULL, the current time is used)
	  * @param progress progress listener, or NULL if not used
	  * @throw exceptions::net_exception if an error occurs
	  */
	virtual void addMessage(utility::inputStream& is, const int size, const int flags = message::FLAG_UNDEFINED, vmime::datetime* date = NULL, utility::progressListener* progress = NULL) = 0;

	/** Copy messages from this folder to another folder.
	  *
	  * @param dest destination folder path
	  * @param msgs index set of messages to copy
	  * @throw exceptions::net_exception if an error occurs
	  */
	virtual void copyMessages(const folder::path& dest, const messageSet& msgs) = 0;

	/** Request folder status without opening it.
	  *
	  * \deprecated Use the new getStatus() method
	  *
	  * @param count will receive the number of messages in the folder
	  * @param unseen will receive the number of unseen messages in the folder
	  * @throw exceptions::net_exception if an error occurs
	  */
	virtual void status(int& count, int& unseen) = 0;

	/** Request folder status without opening it.
	  *
	  * @return current folder status
	  * @throw exceptions::net_exception if an error occurs
	  */
	virtual ref <folderStatus> getStatus() = 0;

	/** Expunge deleted messages.
	  *
	  * @throw exceptions::net_exception if an error occurs
	  */
	virtual void expunge() = 0;

	/** Return a new folder object referencing the parent folder of this folder.
	  *
	  * @return parent folder object
	  */
	virtual ref <folder> getParent() = 0;

	/** Return a reference to the store to which this folder belongs.
	  *
	  * @return the store object to which this folder is attached
	  */
	virtual ref <const store> getStore() const = 0;

	/** Return a reference to the store to which this folder belongs.
	  *
	  * @return the store object to which this folder is attached
	  */
	virtual ref <store> getStore() = 0;

	/** Fetchable objects.
	  */
	enum FetchOptions
	{
		FETCH_ENVELOPE = (1 << 0),       /**< Fetch sender, recipients, date, subject. */
		FETCH_STRUCTURE = (1 << 1),      /**< Fetch structure (body parts). */
		FETCH_CONTENT_INFO = (1 << 2),   /**< Fetch top-level content type. */
		FETCH_FLAGS = (1 << 3),          /**< Fetch message flags. */
		FETCH_SIZE = (1 << 4),           /**< Fetch message size (exact or estimated). */
		FETCH_FULL_HEADER = (1 << 5),    /**< Fetch full RFC-[2]822 header. */
		FETCH_UID = (1 << 6),            /**< Fetch unique identifier (protocol specific). */
		FETCH_IMPORTANCE = (1 << 7),     /**< Fetch header fields suitable for use with misc::importanceHelper. */

		FETCH_CUSTOM = (1 << 16)         /**< Reserved for future use. */
	};

	/** Fetch objects for the specified messages.
	  *
	  * @param msg list of message sequence numbers
	  * @param options objects to fetch (combination of folder::FetchOptions flags)
	  * @param progress progress listener, or NULL if not used
	  * @throw exceptions::net_exception if an error occurs
	  */
	virtual void fetchMessages(std::vector <ref <message> >& msg, const int options, utility::progressListener* progress = NULL) = 0;

	/** Fetch objects for the specified message.
	  *
	  * @param msg the message
	  * @param options objects to fetch (combination of folder::FetchOptions flags)
	  * @throw exceptions::net_exception if an error occurs
	  */
	virtual void fetchMessage(ref <message> msg, const int options) = 0;

	/** Return the list of fetchable objects supported by
	  * the underlying protocol (see folder::FetchOptions).
	  *
	  * @return list of supported fetchable objects
	  */
	virtual int getFetchCapabilities() const = 0;

	/** Return the sequence numbers of messages whose UID equal or greater than
	  * the specified UID.
 	  *
 	  * @param uid the uid of the first message
 	  * @throw exceptions::net_exception if an error occurs
 	  */
	virtual std::vector <int> getMessageNumbersStartingOnUID(const message::uid& uid) = 0;

	// Event listeners
	void addMessageChangedListener(events::messageChangedListener* l);
	void removeMessageChangedListener(events::messageChangedListener* l);

	void addMessageCountListener(events::messageCountListener* l);
	void removeMessageCountListener(events::messageCountListener* l);

	void addFolderListener(events::folderListener* l);
	void removeFolderListener(events::folderListener* l);

protected:

	void notifyMessageChanged(ref <events::messageChangedEvent> event);
	void notifyMessageCount(ref <events::messageCountEvent> event);
	void notifyFolder(ref <events::folderEvent> event);
	void notifyEvent(ref <events::event> event);

private:

	std::list <events::messageChangedListener*> m_messageChangedListeners;
	std::list <events::messageCountListener*> m_messageCountListeners;
	std::list <events::folderListener*> m_folderListeners;
};


} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES

#endif // VMIME_NET_FOLDER_HPP_INCLUDED
