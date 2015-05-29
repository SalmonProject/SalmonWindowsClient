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

#include "../vmime/component.hpp"
#include "../vmime/base.hpp"

#include "../vmime/utility/streamUtils.hpp"
#include "../vmime/utility/inputStreamStringAdapter.hpp"
#include "../vmime/utility/outputStreamAdapter.hpp"

#include <sstream>


namespace vmime
{


component::component()
	: m_parsedOffset(0), m_parsedLength(0)
{
}


component::~component()
{
}


void component::parse
	(ref <utility::inputStream> inputStream, const utility::stream::size_type length)
{
	parse(inputStream, 0, length, NULL);
}


void component::parse
	(ref <utility::inputStream> inputStream, const utility::stream::size_type position,
	 const utility::stream::size_type end, utility::stream::size_type* newPosition)
{
	parse(parsingContext::getDefaultContext(), inputStream, position, end, newPosition);
}


void component::parse
	(const parsingContext& ctx,
	 ref <utility::inputStream> inputStream, const utility::stream::size_type position,
	 const utility::stream::size_type end, utility::stream::size_type* newPosition)
{
	m_parsedOffset = m_parsedLength = 0;

	ref <utility::seekableInputStream> seekableStream =
		inputStream.dynamicCast <utility::seekableInputStream>();

	if (seekableStream == NULL || end == 0)
	{
		// Read the whole stream into a buffer
		std::ostringstream oss;
		utility::outputStreamAdapter ossAdapter(oss);

		utility::bufferedStreamCopyRange(*inputStream, ossAdapter, position, end - position);

		const string buffer = oss.str();
		parseImpl(ctx, buffer, 0, buffer.length(), NULL);
	}
	else
	{
		ref <utility::parserInputStreamAdapter> parser =
			vmime::create <utility::parserInputStreamAdapter>(seekableStream);

		parseImpl(ctx, parser, position, end, newPosition);
	}
}


void component::parse(const string& buffer)
{
	m_parsedOffset = m_parsedLength = 0;

	parseImpl(parsingContext::getDefaultContext(), buffer, 0, buffer.length(), NULL);
}


void component::parse(const parsingContext& ctx, const string& buffer)
{
	m_parsedOffset = m_parsedLength = 0;

	parseImpl(ctx, buffer, 0, buffer.length(), NULL);
}


void component::parse
	(const string& buffer, const string::size_type position,
	 const string::size_type end, string::size_type* newPosition)
{
	m_parsedOffset = m_parsedLength = 0;

	parseImpl(parsingContext::getDefaultContext(), buffer, position, end, newPosition);
}


void component::parse
	(const parsingContext& ctx,
	 const string& buffer, const string::size_type position,
	 const string::size_type end, string::size_type* newPosition)
{
	m_parsedOffset = m_parsedLength = 0;

	parseImpl(ctx, buffer, position, end, newPosition);
}


void component::offsetParsedBounds(const utility::stream::size_type offset)
{
	// Offset parsed bounds of this component
	if (m_parsedLength != 0)
		m_parsedOffset += offset;

	// Offset parsed bounds of our children
	std::vector <ref <component> > children = getChildComponents();

	for (size_t i = 0, n = children.size() ; i < n ; ++i)
		children[i]->offsetParsedBounds(offset);
}


void component::parseImpl
	(const parsingContext& ctx, ref <utility::parserInputStreamAdapter> parser,
	 const utility::stream::size_type position,
	 const utility::stream::size_type end, utility::stream::size_type* newPosition)
{
	// This is the default implementation for parsing from an input stream:
	// actually, we extract the substring and use the "parse from string" implementation
	const string buffer = parser->extract(position, end);
	parseImpl(ctx, buffer, 0, buffer.length(), newPosition);

	// Recursivey offset parsed bounds on children
	if (position != 0)
		offsetParsedBounds(position);

	if (newPosition != NULL)
		*newPosition += position;
}


void component::parseImpl
	(const parsingContext& ctx, const string& buffer, const string::size_type position,
	 const string::size_type end, string::size_type* newPosition)
{
	// This is the default implementation for parsing from a string:
	// actually, we encapsulate the string buffer in an input stream, then use
	// the "parse from input stream" implementation
	ref <utility::seekableInputStream> stream =
		vmime::create <utility::inputStreamStringAdapter>(buffer);

	ref <utility::parserInputStreamAdapter> parser =
		vmime::create <utility::parserInputStreamAdapter>(stream);

	parseImpl(ctx, parser, position, end, newPosition);
}


const string component::generate(const string::size_type maxLineLength,
	const string::size_type curLinePos) const
{
	std::ostringstream oss;
	utility::outputStreamAdapter adapter(oss);

	generationContext ctx(generationContext::getDefaultContext());
	ctx.setMaxLineLength(maxLineLength);

	generateImpl(ctx, adapter, curLinePos, NULL);

	return (oss.str());
}


void component::generate
	(utility::outputStream& os,
	 const string::size_type curLinePos,
	 string::size_type* newLinePos) const
{
	generateImpl(generationContext::getDefaultContext(),
		os, curLinePos, newLinePos);
}


void component::generate
	(const generationContext& ctx,
	 utility::outputStream& outputStream,
	 const string::size_type curLinePos,
	 string::size_type* newLinePos) const
{
	generateImpl(ctx, outputStream, curLinePos, newLinePos);
}


string::size_type component::getParsedOffset() const
{
	return (m_parsedOffset);
}


string::size_type component::getParsedLength() const
{
	return (m_parsedLength);
}


void component::setParsedBounds(const string::size_type start, const string::size_type end)
{
	m_parsedOffset = start;
	m_parsedLength = end - start;
}


utility::stream::size_type component::getGeneratedSize(const generationContext& ctx)
{
	std::vector <ref <component> > children = getChildComponents();
	utility::stream::size_type totalSize = 0;

	for (std::vector <ref <component> >::iterator it = children.begin() ; it != children.end() ; ++it)
		totalSize += (*it)->getGeneratedSize(ctx);

	return totalSize;
}


} // vmime

