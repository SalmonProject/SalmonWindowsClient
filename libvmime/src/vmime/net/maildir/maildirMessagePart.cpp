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


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_MAILDIR


#include "../vmime/net/maildir/maildirMessagePart.hpp"
#include "../vmime/net/maildir/maildirMessageStructure.hpp"


namespace vmime {
namespace net {
namespace maildir {


maildirMessagePart::maildirMessagePart(ref <maildirMessagePart> parent, const int number, const bodyPart& part)
	: m_parent(parent), m_header(NULL), m_number(number)
{
	m_headerParsedOffset = part.getHeader()->getParsedOffset();
	m_headerParsedLength = part.getHeader()->getParsedLength();

	m_bodyParsedOffset = part.getBody()->getParsedOffset();
	m_bodyParsedLength = part.getBody()->getParsedLength();

	m_size = part.getBody()->getContents()->getLength();

	m_mediaType = part.getBody()->getContentType();
}


maildirMessagePart::~maildirMessagePart()
{
}


void maildirMessagePart::initStructure(const bodyPart& part)
{
	if (part.getBody()->getPartList().size() == 0)
		m_structure = NULL;
	else
	{
		m_structure = vmime::create <maildirMessageStructure>
			(thisRef().dynamicCast <maildirMessagePart>(),
			 part.getBody()->getPartList());
	}
}


ref <const messageStructure> maildirMessagePart::getStructure() const
{
	if (m_structure != NULL)
		return m_structure;
	else
		return maildirMessageStructure::emptyStructure();
}


ref <messageStructure> maildirMessagePart::getStructure()
{
	if (m_structure != NULL)
		return m_structure;
	else
		return maildirMessageStructure::emptyStructure();
}


const mediaType& maildirMessagePart::getType() const
{
	return m_mediaType;
}


int maildirMessagePart::getSize() const
{
	return m_size;
}


int maildirMessagePart::getNumber() const
{
	return m_number;
}


ref <const header> maildirMessagePart::getHeader() const
{
	if (m_header == NULL)
		throw exceptions::unfetched_object();
	else
		return m_header;
}


header& maildirMessagePart::getOrCreateHeader()
{
	if (m_header != NULL)
		return *m_header;
	else
		return *(m_header = vmime::create <header>());
}


int maildirMessagePart::getHeaderParsedOffset() const
{
	return m_headerParsedOffset;
}


int maildirMessagePart::getHeaderParsedLength() const
{
	return m_headerParsedLength;
}


int maildirMessagePart::getBodyParsedOffset() const
{
	return m_bodyParsedOffset;
}


int maildirMessagePart::getBodyParsedLength() const
{
	return m_bodyParsedLength;
}


} // maildir
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_MAILDIR
