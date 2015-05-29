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

#include "../vmime/textPartFactory.hpp"
#include "../vmime/exception.hpp"


#include "../vmime/plainTextPart.hpp"
#include "../vmime/htmlTextPart.hpp"


namespace vmime
{


textPartFactory::textPartFactory()
{
	// Register some default names
	registerType <plainTextPart>(mediaType(mediaTypes::TEXT, mediaTypes::TEXT_PLAIN));
    // FIX by Elmue:
    // Added support for Microsoft Outlook Calendar events, treat them as plain text
    registerType <plainTextPart>(mediaType(mediaTypes::TEXT, mediaTypes::TEXT_CALENDAR));
	registerType <htmlTextPart> (mediaType(mediaTypes::TEXT, mediaTypes::TEXT_HTML));
}


textPartFactory::~textPartFactory()
{
}


textPartFactory* textPartFactory::getInstance()
{
	static textPartFactory instance;
	return (&instance);
}


ref <textPart> textPartFactory::create(const mediaType& type)
{
	for (MapType::const_iterator it = m_map.begin() ;
	     it != m_map.end() ; ++it)
	{
		if ((*it).first == type)
			return ((*it).second)();
	}

    // FIX by Elmue: Added usefull information for the user.
    // ("No factory available" is a completely meaningless error message!)
	throw exceptions::no_factory_available("Unknown media type '" + type.generate() + "'.");
}


} // vmime
