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

#include "../vmime/htmlTextPart.hpp"
#include "../vmime/exception.hpp"

#include "../vmime/contentTypeField.hpp"
#include "../vmime/contentDisposition.hpp"
#include "../vmime/text.hpp"

#include "../vmime/emptyContentHandler.hpp"
#include "../vmime/stringContentHandler.hpp"

#include "../vmime/utility/outputStreamAdapter.hpp"

// FIX by Elmue: Added
#include "../vmime/streamContentHandler.hpp"
#include "../vmime/platform.hpp"

namespace vmime
{


htmlTextPart::htmlTextPart()
	: m_plainText(vmime::create <emptyContentHandler>()),
	  m_text(vmime::create <emptyContentHandler>())
{
    // FIX by Elmue: Missing intitialization
    m_charset = charset::getLocalCharset();
}


htmlTextPart::~htmlTextPart()
{
}


const mediaType htmlTextPart::getType() const
{
	return mediaType(mediaTypes::TEXT, mediaTypes::TEXT_HTML);
}


size_t htmlTextPart::getPartCount() const
{
	return (m_plainText->isEmpty() ? 1 : 2);
}


void htmlTextPart::generateIn(ref <bodyPart> /* message */, ref <bodyPart> parent) const
{
	// Plain text
	if (!m_plainText->isEmpty())
	{
		// -- Create a new part
		ref <bodyPart> part = vmime::create <bodyPart>();
		parent->getBody()->appendPart(part);

		// -- Set contents
		part->getBody()->setContents(m_plainText,
			mediaType(mediaTypes::TEXT, mediaTypes::TEXT_PLAIN), m_charset,
			encoding::decide(m_plainText, m_charset, encoding::USAGE_TEXT));
	}

	// HTML text
	// -- Create a new part
	ref <bodyPart> htmlPart = vmime::create <bodyPart>();

	// -- Set contents
	htmlPart->getBody()->setContents(m_text,
		mediaType(mediaTypes::TEXT, mediaTypes::TEXT_HTML), m_charset,
		encoding::decide(m_text, m_charset, encoding::USAGE_TEXT));

	// Handle the case we have embedded objects
	if (!m_objects.empty())
	{
		// Create a "multipart/related" body part
		ref <bodyPart> relPart = vmime::create <bodyPart>();
		parent->getBody()->appendPart(relPart);

		relPart->getHeader()->ContentType()->
			setValue(mediaType(mediaTypes::MULTIPART, mediaTypes::MULTIPART_RELATED));

		// Add the HTML part into this part
		relPart->getBody()->appendPart(htmlPart);

		// Also add objects into this part
		for (std::vector <ref <embeddedObject> >::const_iterator it = m_objects.begin() ;
		     it != m_objects.end() ; ++it)
		{
			ref <bodyPart> objPart = vmime::create <bodyPart>();
			relPart->getBody()->appendPart(objPart);

			string id = (*it)->getId();

            // FIX by Elmue: "cid" may be lower-case or mixed-case
            id = embeddedObject::cleanId(id);

			objPart->getHeader()->ContentType()->setValue((*it)->getType());
			objPart->getHeader()->ContentId()->setValue(messageId("<" + id + ">"));
			objPart->getHeader()->ContentDisposition()->setValue(contentDisposition(contentDispositionTypes::INLINE));
			objPart->getHeader()->ContentTransferEncoding()->setValue((*it)->getEncoding());
				//encoding(encodingTypes::BASE64);

			objPart->getBody()->setContents((*it)->getData()->clone());
		}
	}
	else
	{
		// Add the HTML part into the parent part
		parent->getBody()->appendPart(htmlPart);
	}
}


void htmlTextPart::findEmbeddedParts(const bodyPart& part,
	std::vector <ref <const bodyPart> >& cidParts, std::vector <ref <const bodyPart> >& locParts)
{
	for (size_t i = 0 ; i < part.getBody()->getPartCount() ; ++i)
	{
		ref <const bodyPart> p = part.getBody()->getPartAt(i);

		// For a part to be an embedded object, it must have a
		// Content-Id field or a Content-Location field.
		try
		{
			p->getHeader()->findField(fields::CONTENT_ID);
			cidParts.push_back(p);
		}
		catch (exceptions::no_such_field)
		{
			// No "Content-id" field.
		}

		try
		{
			p->getHeader()->findField(fields::CONTENT_LOCATION);
			locParts.push_back(p);
		}
		catch (exceptions::no_such_field)
		{
			// No "Content-Location" field.
		}

		findEmbeddedParts(*p, cidParts, locParts);
	}
}


void htmlTextPart::addEmbeddedObject(const bodyPart& part, const string& id,
	const embeddedObject::ReferenceType refType)
{
	// The object may already exists. This can happen if an object is
	// identified by both a Content-Id and a Content-Location. In this
	// case, there will be two embedded objects with two different IDs
	// but referencing the same content.

	mediaType type;

	try
	{
		const ref <const headerField> ctf = part.getHeader()->ContentType();
		type = *ctf->getValue().dynamicCast <const mediaType>();
	}
	catch (exceptions::no_such_field)
	{
		// No "Content-type" field: assume "application/octet-stream".
	}

	m_objects.push_back(vmime::create <embeddedObject>
		(part.getBody()->getContents()->clone().dynamicCast <contentHandler>(),
		 part.getBody()->getEncoding(), id, type, refType));
}


void htmlTextPart::parse(ref <const bodyPart> message, ref <const bodyPart> parent, ref <const bodyPart> textPart)
{
	// Search for possible embedded objects in the _whole_ message.
	std::vector <ref <const bodyPart> > cidParts;
	std::vector <ref <const bodyPart> > locParts;

	findEmbeddedParts(*message, cidParts, locParts);

	// Extract HTML text
	std::ostringstream oss;
	utility::outputStreamAdapter adapter(oss);

	textPart->getBody()->getContents()->extract(adapter);

	const string data = oss.str();

	m_text = textPart->getBody()->getContents()->clone();

	try
	{
		const ref <const contentTypeField> ctf =
			textPart->getHeader()->findField(fields::CONTENT_TYPE).dynamicCast <contentTypeField>();

		m_charset = ctf->getCharset();
	}
	catch (exceptions::no_such_field)
	{
		// No "Content-type" field.
	}
	catch (exceptions::no_such_parameter)
	{
		// No "charset" parameter.
	}

	// Extract embedded objects. The algorithm is quite simple: for each previously
	// found inline part, we check if its CID/Location is contained in the HTML text.
	for (std::vector <ref <const bodyPart> >::const_iterator p = cidParts.begin() ; p != cidParts.end() ; ++p)
	{
		const ref <const headerField> midField =
			(*p)->getHeader()->findField(fields::CONTENT_ID);

		const messageId mid = *midField->getValue().dynamicCast <const messageId>();

		if (data.find("CID:" + mid.getId()) != string::npos ||
		    data.find("cid:" + mid.getId()) != string::npos)
		{
			// This part is referenced in the HTML text.
			// Add it to the embedded object list.
			addEmbeddedObject(**p, mid.getId(), embeddedObject::REFERENCED_BY_ID);
		}
	}

	for (std::vector <ref <const bodyPart> >::const_iterator p = locParts.begin() ; p != locParts.end() ; ++p)
	{
		const ref <const headerField> locField =
			(*p)->getHeader()->findField(fields::CONTENT_LOCATION);

		const text loc = *locField->getValue().dynamicCast <const text>();
		const string locStr = loc.getWholeBuffer();

		if (data.find(locStr) != string::npos)
		{
			// This part is referenced in the HTML text.
			// Add it to the embedded object list.
			addEmbeddedObject(**p, locStr, embeddedObject::REFERENCED_BY_LOCATION);
		}
	}

	// Extract plain text, if any.
	if (!findPlainTextPart(*message, *parent, *textPart))
	{
		m_plainText = vmime::create <emptyContentHandler>();
	}
}


bool htmlTextPart::findPlainTextPart(const bodyPart& part, const bodyPart& parent, const bodyPart& textPart)
{
	// We search for the nearest "multipart/alternative" part.
	try
	{
		const ref <const headerField> ctf =
			part.getHeader()->findField(fields::CONTENT_TYPE);

		const mediaType type = *ctf->getValue().dynamicCast <const mediaType>();

		if (type.getType() == mediaTypes::MULTIPART &&
		    type.getSubType() == mediaTypes::MULTIPART_ALTERNATIVE)
		{
			ref <const bodyPart> foundPart = NULL;

			for (size_t i = 0 ; i < part.getBody()->getPartCount() ; ++i)
			{
				const ref <const bodyPart> p = part.getBody()->getPartAt(i);

				if (p == &parent ||     // if "text/html" is in "multipart/related"
				    p == &textPart)     // if not...
				{
					foundPart = p;
				}
			}

			if (foundPart)
			{
				bool found = false;

				// Now, search for the alternative plain text part
				for (size_t i = 0 ; !found && i < part.getBody()->getPartCount() ; ++i)
				{
					const ref <const bodyPart> p = part.getBody()->getPartAt(i);

					try
					{
						const ref <const headerField> ctf =
							p->getHeader()->findField(fields::CONTENT_TYPE);

						const mediaType type = *ctf->getValue().dynamicCast <const mediaType>();

						if (type.getType() == mediaTypes::TEXT &&
						    type.getSubType() == mediaTypes::TEXT_PLAIN)
						{
							m_plainText = p->getBody()->getContents()->clone();
							found = true;
						}
					}
					catch (exceptions::no_such_field)
					{
						// No "Content-type" field.
					}
				}

				// If we don't have found the plain text part here, it means that
				// it does not exists (the MUA which built this message probably
				// did not include it...).
				return found;
			}
		}
	}
	catch (exceptions::no_such_field)
	{
		// No "Content-type" field.
	}

	bool found = false;

	for (size_t i = 0 ; !found && i < part.getBody()->getPartCount() ; ++i)
	{
		found = findPlainTextPart(*part.getBody()->getPartAt(i), parent, textPart);
	}

	return found;
}


const charset& htmlTextPart::getCharset() const
{
	return m_charset;
}


void htmlTextPart::setCharset(const charset& ch)
{
	m_charset = ch;
}


ref <const contentHandler> htmlTextPart::getPlainText() const
{
	return m_plainText;
}


void htmlTextPart::setPlainText(ref <contentHandler> plainText)
{
	m_plainText = plainText->clone();
}


const ref <const contentHandler> htmlTextPart::getText() const
{
	return m_text;
}


void htmlTextPart::setText(ref <contentHandler> text)
{
	m_text = text->clone();
}

size_t htmlTextPart::getObjectCount() const
{
	return m_objects.size();
}


ref <const htmlTextPart::embeddedObject> htmlTextPart::getObjectAt(const size_t pos) const
{
	return m_objects[pos];
}


ref <const htmlTextPart::embeddedObject> htmlTextPart::findObject(const string& id) const
{
	for (std::vector <ref <embeddedObject> >::const_iterator o = m_objects.begin() ;
	     o != m_objects.end() ; ++o)
	{
		if ((*o)->matchesId(id))
			return *o;
	}

	throw exceptions::no_object_found();
}


bool htmlTextPart::hasObject(const string& id) const
{
	for (std::vector <ref <embeddedObject> >::const_iterator o = m_objects.begin() ;
	     o != m_objects.end() ; ++o)
	{
		if ((*o)->matchesId(id))
			return true;
	}

	return false;
}

// FIX by Elmue: Instead of generating a cryptic random ID, let the user pass his own ID here
// (In the constructor of embeddedObject the id is checked for valid charaters)
ref <const htmlTextPart::embeddedObject> htmlTextPart::addObject
	(ref <contentHandler> data, const vmime::encoding& enc, const mediaType& type, const string& id)
{
    if (id.length() < 3)
        throw exception("An ID for the embedded object is mandatory (min 3 characters).");

    for (int p=0; p<(int)id.length(); p++)
    {
        char C = id[p];
        if (!isalnum(C) && C != '.' && C != '@' && C != '-' && C != '_')
            throw exception("Invalid ID for embedded object: '"+id+"'. Character '"+C+"' not allowed.");
    }       

	if (hasObject(id))
        throw exception("An embedded objects with the ID '" +id+ "' has already been added. The ID's must be unique.");

	ref <embeddedObject> obj = vmime::create <embeddedObject>
		(data, enc, id, type, embeddedObject::REFERENCED_BY_ID);

	m_objects.push_back(obj);
	return obj;
}


ref <const htmlTextPart::embeddedObject> htmlTextPart::addObject
	(ref <contentHandler> data, const mediaType& type, const string& id)
{
	return addObject(data, encoding::decide(data), type, id);
}


ref <const htmlTextPart::embeddedObject> htmlTextPart::addObject
	(const string& data, const mediaType& type, const string& id)
{
	ref <stringContentHandler> cts = vmime::create <stringContentHandler>(data);
	return addObject(cts, encoding::decide(cts), type, id);
}

// FIX by Elmue: Added a function that allows to do in one line what otherwise are multiple lines of code
ref <const htmlTextPart::embeddedObject> htmlTextPart::addObjectFile
	(const string& file, const mediaType& type, const string& id)
{
    ref <utility::fileSystemFactory> factory = platform::getHandler()->getFileSystemFactory();
    ref <utility::file> objFile = factory->create(factory->stringToPath(file));

    ref <utility::fileReader> reader = objFile->getFileReader();
    ref <contentHandler> handler = create <streamContentHandler>(reader->getInputStream(), objFile->getLength());

    return addObject(handler, type, id);
}


//
// htmlTextPart::embeddedObject
//

htmlTextPart::embeddedObject::embeddedObject
	(ref <contentHandler> data, const encoding& enc,
	 const string& id, const mediaType& type, const ReferenceType refType)
	: m_data(data->clone().dynamicCast <contentHandler>()),
	  m_encoding(enc), m_id(id), m_type(type), m_refType(refType)
{
}


ref <const contentHandler> htmlTextPart::embeddedObject::getData() const
{
	return m_data;
}


const vmime::encoding htmlTextPart::embeddedObject::getEncoding() const
{
	return m_encoding;
}


const string htmlTextPart::embeddedObject::getId() const
{
	return m_id;
}


const string htmlTextPart::embeddedObject::getReferenceId() const
{
	if (m_refType == REFERENCED_BY_ID)
		return string("cid:") + m_id; // FIX by Elmue: Apple Mail requires lower-case
	else
		return m_id;
}


const mediaType htmlTextPart::embeddedObject::getType() const
{
	return m_type;
}


htmlTextPart::embeddedObject::ReferenceType htmlTextPart::embeddedObject::getReferenceType() const
{
	return m_refType;
}


bool htmlTextPart::embeddedObject::matchesId(const string& id) const
{
	if (m_refType == REFERENCED_BY_ID)
		return m_id == cleanId(id);
	else
		return m_id == id;
}


// static
const string htmlTextPart::embeddedObject::cleanId(const string& id)
{
	if (id.length() >= 4 &&
	    (id[0] == 'c' || id[0] == 'C') &&
	    (id[1] == 'i' || id[1] == 'I') &&
	    (id[2] == 'd' || id[2] == 'D') &&
	    id[3] == ':')
	{
		return id.substr(4);
	}
	else
	{
		return id;
	}
}


} // vmime
