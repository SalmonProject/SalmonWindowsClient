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

#ifndef VMIME_NET_IMAP_IMAPPARSER_HPP_INCLUDED
#define VMIME_NET_IMAP_IMAPPARSER_HPP_INCLUDED


#include "../vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP


#include "../vmime/base.hpp"
#include "../vmime/dateTime.hpp"
#include "../vmime/charset.hpp"
#include "../vmime/exception.hpp"

#include "../vmime/utility/smartPtr.hpp"
#include "../vmime/utility/stringUtils.hpp"
#include "../vmime/utility/progressListener.hpp"

#include "../vmime/utility/encoder/b64Encoder.hpp"
#include "../vmime/utility/encoder/qpEncoder.hpp"

#include "../vmime/utility/inputStreamStringAdapter.hpp"
#include "../vmime/utility/outputStreamStringAdapter.hpp"

#include "../vmime/platform.hpp"

#include "../vmime/net/timeoutHandler.hpp"
#include "../vmime/net/socket.hpp"

#include "../vmime/net/imap/IMAPTag.hpp"

#include <vector>
#include <stdexcept>


#define DEBUG_RESPONSE 0

// FIX by Elmue: Removed Console stuff


namespace vmime {
namespace net {
namespace imap {

// FIX by Elmue: Added console independent Trace output
#if DEBUG_RESPONSE && VMIME_TRACE
	static int IMAPParserDebugResponse_level = 0;
	static std::vector <string> IMAPParserDebugResponse_stack;

	class IMAPParserDebugResponse
	{
	public:

		IMAPParserDebugResponse(const string& name, string& line, const size_t currentPos)
			: m_name(name), m_line(line), m_pos(currentPos)
		{
			++IMAPParserDebugResponse_level;
			IMAPParserDebugResponse_stack.push_back(name);

            // FIX by Elmue: Use ostringstream here
            std::ostringstream oss;

			for (int i = 0 ; i < IMAPParserDebugResponse_level ; ++i)
				oss << "  ";

			oss << "ENTER(" << m_name << "), pos=" << m_pos;
			oss << std::endl;

			for (std::vector <string>::iterator it = IMAPParserDebugResponse_stack.begin() ;
			     it != IMAPParserDebugResponse_stack.end() ; ++it)
			{
				oss << "> " << *it << " ";
			}

			oss << std::endl;
			oss << string(m_line.begin() + (m_pos < 30 ? 0U : m_pos - 30),
				m_line.begin() + std::min(m_line.length(), m_pos + 30)) << std::endl;

			for (size_t i = (m_pos < 30 ? m_pos : (m_pos - (m_pos - 30))) ; i != 0 ; --i)
				oss << " ";

			oss << "^";

            TRACE(oss.str().c_str());
		}

		~IMAPParserDebugResponse()
		{
            // FIX by Elmue: Use ostringstream here
            std::ostringstream oss;

			for (int i = 0 ; i < IMAPParserDebugResponse_level ; ++i)
				oss << "  ";

			oss << "LEAVE(" << m_name << "), result=";
			oss << (std::uncaught_exception() ? "FALSE" : "TRUE") << ", pos=" << m_pos;

            TRACE(oss.str().c_str());

			--IMAPParserDebugResponse_level;
			IMAPParserDebugResponse_stack.pop_back();
		}

	private:

		const string& m_name;
		string& m_line;
		size_t m_pos;
	};


	#define DEBUG_ENTER_COMPONENT(x) \
		IMAPParserDebugResponse dbg(x, line, *currentPos)

    // FIX by Elmue: Added console independent Trace output
	#define DEBUG_FOUND(x, y) \
    { \
        std::ostringstream oss; \
		oss << "FOUND: " << x << ": " << y; \
        TRACE(oss.str().c_str()); \
    } \

#else
	#define DEBUG_ENTER_COMPONENT(x)
	#define DEBUG_FOUND(x, y)
#endif


class VMIME_EXPORT IMAPParser : public object
{
public:

    // FIX by Elmue: Added instance counter
	IMAPParser(weak_ref <IMAPTag> tag, weak_ref <socket> sok, weak_ref <timeoutHandler> _timeoutHandler, int _instanceID)
		: m_tag(tag), m_socket(sok), m_progress(NULL), m_strict(false),
		  m_literalHandler(NULL), m_timeoutHandler(_timeoutHandler), m_instanceID(_instanceID)
	{
	}


	ref <const IMAPTag> getTag() const
	{
		return m_tag.acquire();
	}

	void setSocket(ref <socket> sok)
	{
		m_socket = sok;
	}

	/** Set whether we operate in strict mode (this may not work
	  * with some servers which are not fully standard-compliant).
	  *
	  * @param strict true to operate in strict mode, or false
	  * to operate in default, relaxed mode
	  */
	void setStrict(const bool strict)
	{
		m_strict = strict;
	}

	/** Return true if the parser operates in strict mode, or
	  * false otherwise.
	  *
	  * @return true if we are in strict mode, false otherwise
	  */
	bool isStrict() const
	{
		return m_strict;
	}



	//
	// literalHandler : literal content handler
	//

	class component;

	class literalHandler
	{
	public:

		virtual ~literalHandler() { }


		// Abstract target class
		class target
		{
		protected:

			target(utility::progressListener* progress) : m_progress(progress) {}
			target(const target&) {}

		public:

			virtual ~target() { }


			utility::progressListener* progressListener() { return (m_progress); }

			virtual void putData(const string& chunk) = 0;

		private:

			utility::progressListener* m_progress;
		};


		// Target: put in a string
		class targetString : public target
		{
		public:

			targetString(utility::progressListener* progress, vmime::string& str)
				: target(progress), m_string(str) { }

			const vmime::string& string() const { return (m_string); }
			vmime::string& string() { return (m_string); }


			void putData(const vmime::string& chunk)
			{
				m_string += chunk;
			}

		private:

			vmime::string& m_string;
		};


		// Target: redirect to an output stream
		class targetStream : public target
		{
		public:

			targetStream(utility::progressListener* progress, utility::outputStream& stream)
				: target(progress), m_stream(stream) { }

			const utility::outputStream& stream() const { return (m_stream); }
			utility::outputStream& stream() { return (m_stream); }


			void putData(const string& chunk)
			{
				m_stream.write(chunk.data(), chunk.length());
			}

		private:

			utility::outputStream& m_stream;
		};


		// Called when the parser needs to know what to do with a literal
		//    . comp: the component in which we are at this moment
		//    . data: data specific to the component (may not be used)
		//
		// Returns :
		//    . == NULL to put the literal into the response
		//    . != NULL to redirect the literal to the specified target

		virtual target* targetFor(const component& comp, const int data) = 0;
	};


	//
	// Base class for a terminal or a non-terminal
	//

	class component
	{
	public:

		component() { }
		virtual ~component() { }

		virtual void go(IMAPParser& parser, string& line, size_t* currentPos) = 0;


		const string makeResponseLine(const string& comp, const string& line,
		                              const size_t pos)
		{
            // FIX by Elmue: Added console independent Trace output
            #if DEBUG_RESPONSE && VMIME_TRACE
			    if (pos > line.length())
                    TRACE("WARNING: component::makeResponseLine(): pos > line.length()");
            #endif

			string result(line.substr(0, pos));
			result += "[^]";   // indicates current parser position
			result += line.substr(pos, line.length());
			if (!comp.empty()) result += " [" + comp + "]";

			return (result);
		}
	};


#define COMPONENT_ALIAS(parent, name) \
	class name : public parent \
	{ \
		void go(IMAPParser& parser, string& line, size_t* currentPos) \
		{ \
			DEBUG_ENTER_COMPONENT(#name); \
			parent::go(parser, line, currentPos); \
		} \
	}


	//
	// Parse one character
	//

	template <char C>
	class one_char : public component
	{
	public:

		void go(IMAPParser& /* parser */, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT(string("one_char <") + C + ">: current='" + ((*currentPos < line.length() ? line[*currentPos] : '?')) + "'");

			const size_t pos = *currentPos;

			if (pos < line.length() && line[pos] == C)
				*currentPos = pos + 1;
			else
				throw exceptions::invalid_response("", makeResponseLine("", line, pos));
		}
	};


	//
	// SPACE  ::= <ASCII SP, space, 0x20>
	//

	class SPACE : public component
	{
	public:

		void go(IMAPParser& /* parser */, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("SPACE");

			size_t pos = *currentPos;

			while (pos < line.length() && (line[pos] == ' ' || line[pos] == '\t'))
				++pos;

			if (pos > *currentPos)
				*currentPos = pos;
			else
				throw exceptions::invalid_response("", makeResponseLine("SPACE", line, pos));
		}
	};


	//
	// CR    ::= <ASCII CR, carriage return, 0x0D>
	// LF    ::= <ASCII LF, line feed, 0x0A>
	// CRLF  ::= CR LF
	//

	class CRLF : public component
	{
	public:

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("CRLF");

			size_t pos = *currentPos;

			parser.check <SPACE>(line, &pos, true);

			if (pos + 1 < line.length() &&
			    line[pos] == 0x0d && line[pos + 1] == 0x0a)
			{
				*currentPos = pos + 2;
			}
			else
			{
				throw exceptions::invalid_response("", makeResponseLine("CRLF", line, pos));
			}
		}
	};


	//
	// SPACE           ::= <ASCII SP, space, 0x20>
	// CTL             ::= <any ASCII control character and DEL, 0x00 - 0x1f, 0x7f>
	// CHAR            ::= <any 7-bit US-ASCII character except NUL, 0x01 - 0x7f>
	// ATOM_CHAR       ::= <any CHAR except atom_specials>
	// atom_specials   ::= "(" / ")" / "{" / SPACE / CTL / list_wildcards / quoted_specials
	// list_wildcards  ::= "%" / "*"
	// quoted_specials ::= <"> / "\"
	//
	// tag             ::= 1*<any ATOM_CHAR except "+">    (named "xtag")
	//

	class xtag : public component
	{
	public:

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("tag");

			size_t pos = *currentPos;

			bool end = false;

			string tagString;
			tagString.reserve(10);

			while (!end && pos < line.length())
			{
				const unsigned char c = line[pos];

				switch (c)
				{
				case '+':
				case '(':
				case ')':
				case '{':
				case 0x20:  // SPACE
				case '%':   // list_wildcards
				case '*':   // list_wildcards
				case '"':   // quoted_specials
				case '\\':  // quoted_specials

					end = true;
					break;

				default:

					if (c <= 0x1f || c >= 0x7f)
						end = true;
					else
					{
						tagString += c;
						++pos;
					}

					break;
				}
			}

			if (tagString == string(*parser.getTag()))
			{
				*currentPos = pos;
			}
			else
			{
				// Invalid tag
				throw exceptions::invalid_response("", makeResponseLine("tag", line, pos));
			}
		}
	};


	//
	// digit     ::= "0" / digit_nz
	// digit_nz  ::= "1" / "2" / "3" / "4" / "5" / "6" / "7" / "8" / "9"
	//
	// number    ::= 1*digit
	//               ;; Unsigned 32-bit integer
	//               ;; (0 <= n < 4,294,967,296)
	//

	class number : public component
	{
	public:

		number(const bool nonZero = false)
			: m_nonZero(nonZero), m_value(0)
		{
		}

		void go(IMAPParser& /* parser */, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("number");

			size_t pos = *currentPos;

			bool valid = true;
			unsigned int val = 0;

			while (valid && pos < line.length())
			{
				const char c = line[pos];

				if (c >= '0' && c <= '9')
				{
					val = (val * 10) + (c - '0');
					++pos;
				}
				else
				{
					valid = false;
				}
			}

			// Check for non-null length (and for non-zero number)
			if (!(m_nonZero && val == 0) && pos != *currentPos)
			{
				m_value = val;
				*currentPos = pos;
			}
			else
			{
				throw exceptions::invalid_response("", makeResponseLine("number", line, pos));
			}
		}

	private:

		const bool m_nonZero;
		unsigned long m_value;

	public:

		unsigned long value() const { return (m_value); }
	};


	// nz_number  ::= digit_nz *digit
	//                ;; Non-zero unsigned 32-bit integer
	//                ;; (0 < n < 4,294,967,296)
	//

	class nz_number : public number
	{
	public:

		nz_number() : number(true)
		{
		}
	};


	//
	// uniqueid    ::= nz_number
	//                 ;; Strictly ascending
	//

	class uniqueid : public nz_number
	{
	public:

		uniqueid() : nz_number()
		{
		}
	};


	// uid-range       = (uniqueid ":" uniqueid)
	//                   ; two uniqueid values and all values
	//                   ; between these two regards of order.
	//                   ; Example: 2:4 and 4:2 are equivalent.

	class uid_range : public component
	{
	public:

		uid_range()
			: m_uniqueid1(NULL), m_uniqueid2(NULL)
		{
		}

		~uid_range()
		{
			delete m_uniqueid1;
			delete m_uniqueid2;
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("uid_range");

			size_t pos = *currentPos;

			m_uniqueid1 = parser.get <uniqueid>(line, &pos);
			parser.check <one_char <','> >(line, &pos);
			m_uniqueid2 = parser.get <uniqueid>(line, &pos);

			*currentPos = pos;
		}

	private:

		uniqueid* m_uniqueid1;
		uniqueid* m_uniqueid2;

	public:

		uniqueid* uniqueid1() const { return m_uniqueid1; }
		uniqueid* uniqueid2() const { return m_uniqueid2; }
	};


	//
	// uid-set         = (uniqueid / uid-range) *("," uid-set)
	//

	class uid_set : public component
	{
	public:

		uid_set()
			: m_uniqueid(NULL), m_uid_range(NULL), m_next_uid_set(NULL)
		{
		}

		~uid_set()
		{
			delete m_uniqueid;
			delete m_uid_range;
			delete m_next_uid_set;
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("uid_set");

			size_t pos = *currentPos;

			// We have either a 'uid_range' or a 'uniqueid'
			if (!(m_uid_range = parser.get <IMAPParser::uid_range>(line, &pos, true)))
				m_uniqueid = parser.get <IMAPParser::uniqueid>(line, &pos);

			// And maybe another 'uid-set' following
			if (parser.check <one_char <','> >(line, &pos, true))
				m_next_uid_set = parser.get <IMAPParser::uid_set>(line, &pos);

			*currentPos = pos;
		}

	private:

		IMAPParser::uniqueid* m_uniqueid;
		IMAPParser::uid_range* m_uid_range;

		IMAPParser::uid_set* m_next_uid_set;

	public:

		IMAPParser::uniqueid* uniqueid() const { return m_uniqueid; }
		IMAPParser::uid_range* uid_range() const { return m_uid_range; }

		IMAPParser::uid_set* next_uid_set() const { return m_next_uid_set; }
	};


	//
	// text       ::= 1*TEXT_CHAR
	//
	// CHAR       ::= <any 7-bit US-ASCII character except NUL, 0x01 - 0x7f>
	// TEXT_CHAR  ::= <any CHAR except CR and LF>
	//

	class text : public component
	{
	public:

		text(bool allow8bits = false, const char except = 0)
			: m_allow8bits(allow8bits), m_except(except)
		{
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("text");

			size_t pos = *currentPos;
			size_t len = 0;

			if (m_allow8bits || !parser.isStrict())
			{
				const unsigned char except = m_except;

				for (bool end = false ; !end && pos < line.length() ; )
				{
					const unsigned char c = line[pos];

					if (c == 0x00 || c == 0x0d || c == 0x0a || c == except)
					{
						end = true;
					}
					else
					{
						++pos;
						++len;
					}
				}
			}
			else
			{
				const unsigned char except = m_except;

				for (bool end = false ; !end && pos < line.length() ; )
				{
					const unsigned char c = line[pos];

					if (c < 0x01 || c > 0x7f || c == 0x0d || c == 0x0a || c == except)
					{
						end = true;
					}
					else
					{
						++pos;
						++len;
					}
				}
			}

			if (len != 0)
			{
				m_value.resize(len);
				std::copy(line.begin() + *currentPos, line.begin() + pos, m_value.begin());

				*currentPos = pos;
			}
			else
			{
				throw exceptions::invalid_response("", makeResponseLine("text", line, pos));
			}
		}

	private:

		string m_value;
		const bool m_allow8bits;
		const char m_except;

	public:

		const string& value() const { return (m_value); }
	};


	class text8 : public text
	{
	public:

		text8() : text(true)
		{
		}
	};


	template <char C>
	class text_except : public text
	{
	public:

		text_except() : text(false, C)
		{
		}
	};


	template <char C>
	class text8_except : public text
	{
	public:

		text8_except() : text(true, C)
		{
		}
	};


	//
	// QUOTED_CHAR     ::= <any TEXT_CHAR except quoted_specials> / "\" quoted_specials
	// quoted_specials ::= <"> / "\"
	// TEXT_CHAR       ::= <any CHAR except CR and LF>
	// CHAR            ::= <any 7-bit US-ASCII character except NUL, 0x01 - 0x7f>
	//

	class QUOTED_CHAR : public component
	{
	public:

		void go(IMAPParser& /* parser */, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("quoted_char");

			size_t pos = *currentPos;

			const unsigned char c = static_cast <unsigned char>(pos < line.length() ? line[pos] : 0);

			if (c >= 0x01 && c <= 0x7f &&   // 0x01 - 0x7f
			    c != '"' && c != '\\' &&    // quoted_specials
			    c != '\r' && c != '\n')     // CR and LF
			{
				m_value = c;
				*currentPos = pos + 1;
			}
			else if (c == '\\' && pos + 1 < line.length() &&
			         (line[pos + 1] == '"' || line[pos + 1] == '\\'))
			{
				m_value = line[pos + 1];
				*currentPos = pos + 2;
			}
			else
			{
				throw exceptions::invalid_response("", makeResponseLine("QUOTED_CHAR", line, pos));
			}
		}

	private:

		char m_value;

	public:

		char value() const { return (m_value); }
	};


	//
	// quoted          ::= <"> *QUOTED_CHAR <">
	// QUOTED_CHAR     ::= <any TEXT_CHAR except quoted_specials> / "\" quoted_specials
	// quoted_specials ::= <"> / "\"
	// TEXT_CHAR       ::= <any CHAR except CR and LF>
	// CHAR            ::= <any 7-bit US-ASCII character except NUL, 0x01 - 0x7f>
	//

	class quoted_text : public component
	{
	public:

		void go(IMAPParser& /* parser */, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("quoted_text");

			size_t pos = *currentPos;
			size_t len = 0;
			bool valid = false;

			m_value.reserve(line.length() - pos);

			for (bool end = false, quoted = false ; !end && pos < line.length() ; )
			{
				const unsigned char c = line[pos];

				if (quoted)
				{
					if (c == '"' || c == '\\')
						m_value += c;
					else
					{
						m_value += '\\';
						m_value += c;
					}

					quoted = false;

					++pos;
					++len;
				}
				else
				{
					if (c == '\\')
					{
						quoted = true;

						++pos;
						++len;
					}
					else if (c == '"')
					{
						valid = true;
						end = true;
					}
					else if (c >= 0x01 && c <= 0x7f &&  // CHAR
					         c != 0x0a && c != 0x0d)    // CR and LF
					{
						m_value += c;

						++pos;
						++len;
					}
					else
					{
						valid = false;
						end = true;
					}
				}
			}

			if (valid)
			{
				*currentPos = pos;
			}
			else
			{
				throw exceptions::invalid_response("", makeResponseLine("quoted_text", line, pos));
			}
		}

	private:

		string m_value;

	public:

		const string& value() const { return (m_value); }
	};


	//
	// nil  ::= "NIL"
	//

	class NIL : public component
	{
	public:

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("NIL");

			size_t pos = *currentPos;

			parser.checkWithArg <special_atom>(line, &pos, "nil");

			*currentPos = pos;
		}
	};


	//
	// string          ::= quoted / literal    ----> named 'xstring'
	//
	// nil             ::= "NIL"
	// quoted          ::= <"> *QUOTED_CHAR <">
	// QUOTED_CHAR     ::= <any TEXT_CHAR except quoted_specials> / "\" quoted_specials
	// quoted_specials ::= <"> / "\"
	// TEXT_CHAR       ::= <any CHAR except CR and LF>
	// CHAR            ::= <any 7-bit US-ASCII character except NUL, 0x01 - 0x7f>
	// literal         ::= "{" number "}" CRLF *CHAR8
	//                     ;; Number represents the number of CHAR8 octets
	// CHAR8           ::= <any 8-bit octet except NUL, 0x01 - 0xff>
	//

	class xstring : public component
	{
	public:

		xstring(const bool canBeNIL = false, component* comp = NULL, const int data = 0)
			: m_canBeNIL(canBeNIL), m_component(comp), m_data(data)
		{
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("string");

			size_t pos = *currentPos;

			if (m_canBeNIL &&
			    parser.checkWithArg <special_atom>(line, &pos, "nil", true))
			{
				// NIL
			}
			else
			{
				pos = *currentPos;

				// quoted ::= <"> *QUOTED_CHAR <">
				if (parser.check <one_char <'"'> >(line, &pos, true))
				{
					std::auto_ptr <quoted_text> text(parser.get <quoted_text>(line, &pos));
					parser.check <one_char <'"'> >(line, &pos);

					if (parser.m_literalHandler != NULL)
					{
						literalHandler::target* target =
							parser.m_literalHandler->targetFor(*m_component, m_data);

						if (target != NULL)
						{
							m_value = "[literal-handler]";

							const size_t length = text->value().length();
							utility::progressListener* progress = target->progressListener();

							if (progress)
							{
								progress->start(length);
							}

							target->putData(text->value());

							if (progress)
							{
								progress->progress(length, length);
								progress->stop(length);
							}

							delete (target);
						}
						else
						{
							m_value = text->value();
						}
					}
					else
					{
						m_value = text->value();
					}

					DEBUG_FOUND("string[quoted]", "<length=" << m_value.length() << ", value='" << m_value << "'>");
				}
				// literal ::= "{" number "}" CRLF *CHAR8
				else
				{
					parser.check <one_char <'{'> >(line, &pos);

					number* num = parser.get <number>(line, &pos);

					const size_t length = num->value();
					delete (num);

					parser.check <one_char <'}'> >(line, &pos);

					parser.check <CRLF>(line, &pos);


					if (parser.m_literalHandler != NULL)
					{
						literalHandler::target* target =
							parser.m_literalHandler->targetFor(*m_component, m_data);

						if (target != NULL)
						{
							m_value = "[literal-handler]";

							parser.m_progress = target->progressListener();
							parser.readLiteral(*target, length);
							parser.m_progress = NULL;

							delete (target);
						}
						else
						{
							literalHandler::targetString target(NULL, m_value);
							parser.readLiteral(target, length);
						}
					}
					else
					{
						literalHandler::targetString target(NULL, m_value);
						parser.readLiteral(target, length);
					}

					line += parser.readLine();

					DEBUG_FOUND("string[literal]", "<length=" << length << ", value='" << m_value << "'>");
				}
			}

			*currentPos = pos;
		}

	private:

		bool m_canBeNIL;
		string m_value;

		component* m_component;
		const int m_data;

	public:

		const string& value() const { return (m_value); }
		void setValue(const string& val) { m_value = val; }
	};


	//
	// nstring         ::= string / nil
	//

	class nstring : public xstring
	{
	public:

		nstring(component* comp = NULL, const int data = 0)
			: xstring(true, comp, data)
		{
		}
	};


	//
	// astring ::= atom / string
	//

	class astring : public component
	{
	public:

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("astring");

			size_t pos = *currentPos;

			xstring* str = NULL;

			if ((str = parser.get <xstring>(line, &pos, true)))
			{
				m_value = str->value();
				delete (str);
			}
			else
			{
				atom* at = parser.get <atom>(line, &pos);
				m_value = at->value();
				delete (at);
			}

			*currentPos = pos;
		}

	private:

		string m_value;

	public:

		const string& value() const { return (m_value); }
	};


	//
	// atom            ::= 1*ATOM_CHAR
	//
	// ATOM_CHAR       ::= <any CHAR except atom_specials>
	// atom_specials   ::= "(" / ")" / "{" / SPACE / CTL / list_wildcards / quoted_specials
	// CHAR            ::= <any 7-bit US-ASCII character except NUL, 0x01 - 0x7f>
	// CTL             ::= <any ASCII control character and DEL, 0x00 - 0x1f, 0x7f>
	// list_wildcards  ::= "%" / "*"
	// quoted_specials ::= <"> / "\"
	// SPACE           ::= <ASCII SP, space, 0x20>
	//

	class atom : public component
	{
	public:

		void go(IMAPParser& /* parser */, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("atom");

			size_t pos = *currentPos;
			size_t len = 0;

			for (bool end = false ; !end && pos < line.length() ; )
			{
				const unsigned char c = line[pos];

				switch (c)
				{
				case '(':
				case ')':
				case '{':
				case 0x20:  // SPACE
				case '%':   // list_wildcards
				case '*':   // list_wildcards
				case '"':   // quoted_specials
				case '\\':  // quoted_specials

				case '[':
				case ']':   // for "special_atom"

					end = true;
					break;

				default:

					if (c <= 0x1f || c >= 0x7f)
						end = true;
					else
					{
						++pos;
						++len;
					}
				}
			}

			if (len != 0)
			{
				m_value.resize(len);
				std::copy(line.begin() + *currentPos, line.begin() + pos, m_value.begin());

				*currentPos = pos;
			}
			else
			{
				throw exceptions::invalid_response("", makeResponseLine("atom", line, pos));
			}
		}

	private:

		string m_value;

	public:

		const string& value() const { return (m_value); }
	};


	//
	// special atom (eg. "CAPABILITY", "FLAGS", "STATUS"...)
	//
	//  " Except as noted otherwise, all alphabetic characters are case-
	//    insensitive. The use of upper or lower case characters to define
	//    token strings is for editorial clarity only. Implementations MUST
	//    accept these strings in a case-insensitive fashion. "
	//

	class special_atom : public atom
	{
	public:

		special_atom(const char* str)
			: m_string(str)   // 'string' must be in lower-case
		{
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT(string("special_atom(") + m_string + ")");

			size_t pos = *currentPos;

			atom::go(parser, line, &pos);

			const char* cmp = value().c_str();
			const char* with = m_string;

			bool ok = true;

			while (ok && *cmp && *with)
			{
				ok = (std::tolower(*cmp, std::locale()) == *with);

				++cmp;
				++with;
			}

			if (!ok || *cmp || *with)
			{
				throw exceptions::invalid_response("", makeResponseLine(string("special_atom <") + m_string + ">", line, pos));
			}
			else
			{
				*currentPos = pos;
			}
		}

	private:

		const char* m_string;
	};


	//
	// text_mime2 ::= "=?" <charset> "?" <encoding> "?" <encoded-text> "?="
	//                ;; Syntax defined in [MIME-HDRS]
	//

	class text_mime2 : public component
	{
	public:

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("text_mime2");

			size_t pos = *currentPos;

			atom* theCharset = NULL, *theEncoding = NULL;
			text* theText = NULL;

			try
			{
				parser.check <one_char <'='> >(line, &pos);
				parser.check <one_char <'?'> >(line, &pos);

				theCharset = parser.get <atom>(line, &pos);

				parser.check <one_char <'?'> >(line, &pos);

				theEncoding = parser.get <atom>(line, &pos);

				parser.check <one_char <'?'> >(line, &pos);

				theText = parser.get <text8_except <'?'> >(line, &pos);

				parser.check <one_char <'?'> >(line, &pos);
				parser.check <one_char <'='> >(line, &pos);
			}
			catch (std::exception&)
			{
				delete (theCharset);
				delete (theEncoding);
				delete (theText);

				throw;
			}

			m_charset = theCharset->value();
			delete (theCharset);

			// Decode text
			utility::encoder::encoder* theEncoder = NULL;

			if (theEncoding->value()[0] == 'q' || theEncoding->value()[0] == 'Q')
			{
				// Quoted-printable
				theEncoder = new utility::encoder::qpEncoder();
				theEncoder->getProperties()["rfc2047"] = true;
			}
			else if (theEncoding->value()[0] == 'b' || theEncoding->value()[0] == 'B')
			{
				// Base64
				theEncoder = new utility::encoder::b64Encoder();
			}

			if (theEncoder)
			{
				utility::inputStreamStringAdapter in(theText->value());
				utility::outputStreamStringAdapter out(m_value);

				theEncoder->decode(in, out);
				delete (theEncoder);
			}
			// No decoder available
			else
			{
				m_value = theText->value();
			}

			delete (theEncoding);
			delete (theText);

			*currentPos = pos;
		}

	private:

		vmime::charset m_charset;
		string m_value;

	public:

		const vmime::charset& charset() const { return (m_charset); }
		const string& value() const { return (m_value); }
	};


	// seq-number      = nz-number / "*"
	//                    ; message sequence number (COPY, FETCH, STORE
	//                    ; commands) or unique identifier (UID COPY,
	//                    ; UID FETCH, UID STORE commands).

	class seq_number : public component
	{
	public:

		seq_number()
			: m_number(NULL), m_star(false)
		{
		}

		~seq_number()
		{
			delete m_number;
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("seq_number");

			size_t pos = *currentPos;

			if (parser.check <one_char <'*'> >(line, &pos, true))
			{
				m_star = true;
				m_number = NULL;
			}
			else
			{
				m_star = false;
				m_number = parser.get <IMAPParser::number>(line, &pos);
			}

			*currentPos = pos;
		}

	private:

		IMAPParser::number* m_number;
		bool m_star;

	public:

		const IMAPParser::number* number() const { return m_number; }
		bool star() const { return m_star; }
	};


	// seq-range       = seq-number ":" seq-number
	//                    ; two seq-number values and all values between
	//                    ; these two regardless of order.
	//                    ; Example: 2:4 and 4:2 are equivalent and indicate
	//                    ; values 2, 3, and 4.

	class seq_range : public component
	{
	public:

		seq_range()
			: m_first(NULL), m_last(NULL)
		{
		}

		~seq_range()
		{
			delete m_first;
			delete m_last;
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("seq_range");

			size_t pos = *currentPos;

			m_first = parser.get <seq_number>(line, &pos);

			parser.check <one_char <'*'> >(line, &pos);

			m_last = parser.get <seq_number>(line, &pos);

			*currentPos = pos;
		}

	private:

		IMAPParser::seq_number* m_first;
		IMAPParser::seq_number* m_last;

	public:

		const IMAPParser::seq_number* first() const { return m_first; }
		const IMAPParser::seq_number* last() const { return m_last; }
	};


	// sequence-set    = (seq-number / seq-range) *("," sequence-set)
	//                    ; set of seq-number values, regardless of order.
	//                    ; Servers MAY coalesce overlaps and/or execute the
	//                    ; sequence in any order.
	//                    ; Example: a message sequence number set of
	//                    ; 2,4:7,9,12:* for a mailbox with 15 messages is
	//                    ; equivalent to 2,4,5,6,7,9,12,13,14,15

	class sequence_set : public component
	{
	public:

		sequence_set()
			: m_number(NULL), m_range(NULL), m_nextSet(NULL)
		{
		}

		~sequence_set()
		{
			delete m_number;
			delete m_range;
			delete m_nextSet;
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("sequence_set");

			size_t pos = *currentPos;

			if ((m_range = parser.get <IMAPParser::seq_range>(line, &pos, true)) == NULL)
				m_number = parser.get <IMAPParser::seq_number>(line, &pos);

			if (parser.check <one_char <','> >(line, &pos, true))
				m_nextSet = parser.get <sequence_set>(line, &pos);

			*currentPos = pos;
		}

	private:

		IMAPParser::seq_number* m_number;
		IMAPParser::seq_range* m_range;
		IMAPParser::sequence_set* m_nextSet;

	public:

		const IMAPParser::seq_number* seq_number() const { return m_number; }
		const IMAPParser::seq_range* seq_range() const { return m_range; }
		const IMAPParser::sequence_set* next_sequence_set() const { return m_nextSet; }
	};


	// mod-sequence-value  = 1*DIGIT
	//                        ;; Positive unsigned 64-bit integer
	//                        ;; (mod-sequence)
	//                        ;; (1 <= n < 18,446,744,073,709,551,615)

	class mod_sequence_value : public component
	{
	public:

		mod_sequence_value()
			: m_value(0)
		{
		}

		void go(IMAPParser& /* parser */, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("mod_sequence_value");

			size_t pos = *currentPos;

			bool valid = true;
			vmime_uint64 val = 0;

			while (valid && pos < line.length())
			{
				const char c = line[pos];

				if (c >= '0' && c <= '9')
				{
					val = (val * 10) + (c - '0');
					++pos;
				}
				else
				{
					valid = false;
				}
			}

			m_value = val;

			*currentPos = pos;
		}

	private:

		vmime_uint64 m_value;

	public:

		vmime_uint64 value() const { return m_value; }
	};


	//
	// flag            ::= "\Answered" / "\Flagged" / "\Deleted" /
	//                     "\Seen" / "\Draft" / flag_keyword / flag_extension
	//
	// flag_extension  ::= "\" atom
	//                     ;; Future expansion.  Client implementations
	//                     ;; MUST accept flag_extension flags.  Server
	//                     ;; implementations MUST NOT generate
	//                     ;; flag_extension flags except as defined by
	//                     ;; future standard or standards-track
	//                     ;; revisions of this specification.
	//
	// flag_keyword    ::= atom
	//

	class flag : public component
	{
	public:

		flag()
			: m_type(UNKNOWN), m_flag_keyword(NULL)
		{
		}

		~flag()
		{
			delete (m_flag_keyword);
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("flag_keyword");

			size_t pos = *currentPos;

			if (parser.check <one_char <'\\'> >(line, &pos, true))
			{
				if (parser.check <one_char <'*'> >(line, &pos, true))
				{
					m_type = STAR;
				}
				else
				{
					atom* at = parser.get <atom>(line, &pos);
					const string name = utility::stringUtils::toLower(at->value());
					delete (at);

					if (name == "answered")
						m_type = ANSWERED;
					else if (name == "flagged")
						m_type = FLAGGED;
					else if (name == "deleted")
						m_type = DELETED;
					else if (name == "seen")
						m_type = SEEN;
					else if (name == "draft")
						m_type = DRAFT;
					else
					{
						m_type = UNKNOWN;
						m_name = name;
					}
				}
			}
			else
			{
				m_type = KEYWORD_OR_EXTENSION;
				m_flag_keyword = parser.get <atom>(line, &pos);
			}

			*currentPos = pos;
		}


		enum Type
		{
			UNKNOWN,
			ANSWERED,
			FLAGGED,
			DELETED,
			SEEN,
			DRAFT,
			KEYWORD_OR_EXTENSION,
			STAR       // * = custom flags allowed
		};

	private:

		Type m_type;
		string m_name;

		IMAPParser::atom* m_flag_keyword;

	public:

		Type type() const { return (m_type); }
		const string& name() const { return (m_name); }

		const IMAPParser::atom* flag_keyword() const { return (m_flag_keyword); }
	};


	//
	// flag_list ::= "(" #flag ")"
	//

	class flag_list : public component
	{
	public:

		~flag_list()
		{
			for (std::vector <flag*>::iterator it = m_flags.begin() ;
			     it != m_flags.end() ; ++it)
			{
				delete (*it);
			}
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("flag_list");

			size_t pos = *currentPos;

			parser.check <one_char <'('> >(line, &pos);

			while (!parser.check <one_char <')'> >(line, &pos, true))
			{
				m_flags.push_back(parser.get <flag>(line, &pos));
				parser.check <SPACE>(line, &pos, true);
			}

			*currentPos = pos;
		}

	private:

		std::vector <flag*> m_flags;

	public:

		const std::vector <flag*>& flags() const { return (m_flags); }
	};


	//
	// mailbox ::= "INBOX" / astring
	//             ;; INBOX is case-insensitive.  All case variants of
	//             ;; INBOX (e.g. "iNbOx") MUST be interpreted as INBOX
	//             ;; not as an astring.  Refer to section 5.1 for
	//             ;; further semantic details of mailbox names.
	//

	class mailbox : public component
	{
	public:

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("mailbox");

			size_t pos = *currentPos;

			if (parser.checkWithArg <special_atom>(line, &pos, "inbox", true))
			{
				m_type = INBOX;
				m_name = "INBOX";
			}
			else
			{
				m_type = OTHER;

				astring* astr = parser.get <astring>(line, &pos);
				m_name = astr->value();
				delete (astr);
			}

			*currentPos = pos;
		}


		enum Type
		{
			INBOX,
			OTHER
		};

	private:

		Type m_type;
		string m_name;

	public:

		Type type() const { return (m_type); }
		const string& name() const { return (m_name); }
	};


	//
	// mailbox_flag := "\Marked" / "\Noinferiors" /
	//                 "\Noselect" / "\Unmarked" / flag_extension
	//

	class mailbox_flag : public component
	{
	public:

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("mailbox_flag");

			size_t pos = *currentPos;

			if (parser.check <one_char <'\\'> >(line, &pos, true))
			{
				atom* at = parser.get <atom>(line, &pos);
				const string name = utility::stringUtils::toLower(at->value());
				delete (at);

				m_type = UNKNOWN;  // default

				switch (name[0])
				{
				case 'a':

					if (name == "all")
						m_type = SPECIALUSE_ALL;
					else if (name == "archive")
						m_type = SPECIALUSE_ARCHIVE;

					break;

				case 'd':

					if (name == "drafts")
						m_type = SPECIALUSE_DRAFTS;

					break;

				case 'f':

					if (name == "flagged")
						m_type = SPECIALUSE_FLAGGED;

					break;

				case 'h':

					if (name == "haschildren")
						m_type = HASCHILDREN;
					else if (name == "hasnochildren")
						m_type = HASNOCHILDREN;

					break;

				case 'i':

					if (name == "important")
						m_type = SPECIALUSE_IMPORTANT;

					break;

				case 'j':

					if (name == "junk")
						m_type = SPECIALUSE_JUNK;

					break;

				case 'm':

				if (name == "marked")
					m_type = MARKED;

				case 'n':

					if (name == "noinferiors")
					m_type = NOINFERIORS;
				else if (name == "noselect")
					m_type = NOSELECT;

				case 's':

					if (name == "sent")
						m_type = SPECIALUSE_SENT;

					break;

				case 't':

					if (name == "trash")
						m_type = SPECIALUSE_TRASH;

					break;

				case 'u':

					if (name == "unmarked")
					m_type = UNMARKED;

					break;
				}

				if (m_type == UNKNOWN)
					m_name = "\\" + name;
			}
			else
			{
				atom* at = parser.get <atom>(line, &pos);
				const string name = utility::stringUtils::toLower(at->value());
				delete (at);

				m_type = UNKNOWN;
				m_name = name;
			}

			*currentPos = pos;
		}


		enum Type
		{
			// RFC-3348 - Child Mailbox Extension
			HASCHILDREN,
			HASNOCHILDREN,

			// RFC-6154 - Special-Use Mailboxes
			SPECIALUSE_ALL,
			SPECIALUSE_ARCHIVE,
			SPECIALUSE_DRAFTS,
			SPECIALUSE_FLAGGED,
			SPECIALUSE_JUNK,
			SPECIALUSE_SENT,
			SPECIALUSE_TRASH,
			SPECIALUSE_IMPORTANT,    // draft

			// Standard mailbox flags
			UNKNOWN,
			MARKED,
			NOINFERIORS,
			NOSELECT,
			UNMARKED
		};

	private:

		Type m_type;
		string m_name;

	public:

		Type type() const { return (m_type); }
		const string& name() const { return (m_name); }
	};


	//
	// mailbox_flag_list ::= "(" #(mailbox_flag) ")"
	//

	class mailbox_flag_list : public component
	{
	public:

		~mailbox_flag_list()
		{
			for (std::vector <mailbox_flag*>::iterator it = m_flags.begin() ;
			     it != m_flags.end() ; ++it)
			{
				delete (*it);
			}
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("mailbox_flag_list");

			size_t pos = *currentPos;

			parser.check <one_char <'('> >(line, &pos);

			while (!parser.check <one_char <')'> >(line, &pos, true))
			{
				m_flags.push_back(parser.get <mailbox_flag>(line, &pos));
				parser.check <SPACE>(line, &pos, true);
			}

			*currentPos = pos;
		}

	private:

		std::vector <mailbox_flag*> m_flags;

	public:

		const std::vector <mailbox_flag*>& flags() const { return (m_flags); }
	};


	//
	// mailbox_list ::= mailbox_flag_list SPACE
	//                  (<"> QUOTED_CHAR <"> / nil) SPACE mailbox
	//

	class mailbox_list : public component
	{
	public:

		mailbox_list()
			: m_mailbox_flag_list(NULL),
			  m_mailbox(NULL), m_quoted_char('\0')
		{
		}

		~mailbox_list()
		{
			delete (m_mailbox_flag_list);
			delete (m_mailbox);
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("mailbox_list");

			size_t pos = *currentPos;

			m_mailbox_flag_list = parser.get <IMAPParser::mailbox_flag_list>(line, &pos);

			parser.check <SPACE>(line, &pos);

			if (!parser.check <NIL>(line, &pos, true))
			{
				parser.check <one_char <'"'> >(line, &pos);

				QUOTED_CHAR* qc = parser.get <QUOTED_CHAR>(line, &pos);
				m_quoted_char = qc->value();
				delete (qc);

				parser.check <one_char <'"'> >(line, &pos);
			}

			parser.check <SPACE>(line, &pos);

			m_mailbox = parser.get <IMAPParser::mailbox>(line, &pos);

			*currentPos = pos;
		}

	private:

		IMAPParser::mailbox_flag_list* m_mailbox_flag_list;
		IMAPParser::mailbox* m_mailbox;
		char m_quoted_char;

	public:

		const IMAPParser::mailbox_flag_list* mailbox_flag_list() const { return (m_mailbox_flag_list); }
		const IMAPParser::mailbox* mailbox() const { return (m_mailbox); }
		char quoted_char() const { return (m_quoted_char); }
	};


	//
	// auth_type ::= atom
	//               ;; Defined by [IMAP-AUTH]
	//

	class auth_type : public component
	{
	public:

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("auth_type");

			atom* at = parser.get <atom>(line, currentPos);
			m_name = utility::stringUtils::toLower(at->value());
			delete (at);

			if (m_name == "kerberos_v4")
				m_type = KERBEROS_V4;
			else if (m_name == "gssapi")
				m_type = GSSAPI;
			else if (m_name == "skey")
				m_type = SKEY;
			else
				m_type = UNKNOWN;
		}


		enum Type
		{
			UNKNOWN,

			// RFC 1731 - IMAP4 Authentication Mechanisms
			KERBEROS_V4,
			GSSAPI,
			SKEY
		};

	private:

		Type m_type;
		string m_name;

	public:

		Type type() const { return (m_type); }
		const string name() const { return (m_name); }
	};


	//
	// status-att-val  = ("MESSAGES" SP number) /
	//                   ("RECENT" SP number) /
	//                   ("UIDNEXT" SP nz-number) /
	//                   ("UIDVALIDITY" SP nz-number) /
	//                   ("UNSEEN" SP number)
	//
	// IMAP Extension for Conditional STORE (RFC-4551):
	//
	//   status-att-val      =/ "HIGHESTMODSEQ" SP mod-sequence-valzer
	//                          ;; extends non-terminal defined in [IMAPABNF].
	//                          ;; Value 0 denotes that the mailbox doesn't
	//                          ;; support persistent mod-sequences
	//

	class status_att_val : public component
	{
	public:

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("status_att");

			size_t pos = *currentPos;

			// "HIGHESTMODSEQ" SP mod-sequence-valzer
			if (parser.checkWithArg <special_atom>(line, &pos, "highestmodseq", true))
			{
				m_type = HIGHESTMODSEQ;

				parser.check <SPACE>(line, &pos);
				m_value = parser.get <IMAPParser::mod_sequence_value>(line, &pos);
			}
			else
			{
				if (parser.checkWithArg <special_atom>(line, &pos, "messages", true))
				{
					m_type = MESSAGES;
				}
				else if (parser.checkWithArg <special_atom>(line, &pos, "recent", true))
				{
					m_type = RECENT;
				}
				else if (parser.checkWithArg <special_atom>(line, &pos, "uidnext", true))
				{
					m_type = UIDNEXT;
				}
				else if (parser.checkWithArg <special_atom>(line, &pos, "uidvalidity", true))
				{
					m_type = UIDVALIDITY;
				}
				else
				{
					parser.checkWithArg <special_atom>(line, &pos, "unseen");
					m_type = UNSEEN;
				}

				parser.check <SPACE>(line, &pos);
				m_value = parser.get <IMAPParser::number>(line, &pos);
			}

			*currentPos = pos;
		}


		enum Type
		{
			// Extensions
			HIGHESTMODSEQ,

			// Standard IMAP
			MESSAGES,
			RECENT,
			UIDNEXT,
			UIDVALIDITY,
			UNSEEN
		};

	private:

		Type m_type;
		IMAPParser::component* m_value;

	public:

		Type type() const { return (m_type); }

		const IMAPParser::number* value_as_number() const
		{
			return dynamic_cast <IMAPParser::number *>(m_value);
		}

		const IMAPParser::mod_sequence_value* value_as_mod_sequence_value() const
		{
			return dynamic_cast <IMAPParser::mod_sequence_value *>(m_value);
		}
	};


	// status-att-list = status-att-val *(SP status-att-val)

	class status_att_list : public component
	{
	public:

		~status_att_list()
		{
			for (std::vector <status_att_val*>::iterator it = m_values.begin() ;
			     it != m_values.end() ; ++it)
			{
				delete *it;
			}
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("status_att_list");

			size_t pos = *currentPos;

			m_values.push_back(parser.get <IMAPParser::status_att_val>(line, &pos));

			while (parser.check <SPACE>(line, &pos, true))
				m_values.push_back(parser.get <IMAPParser::status_att_val>(line, &pos));

			*currentPos = pos;
		}

	private:

		std::vector <status_att_val*> m_values;

	public:

		const std::vector <status_att_val*>& values() const { return m_values; }
	};


	//
	// capability ::= "AUTH=" auth_type / atom
	//                ;; New capabilities MUST begin with "X" or be
	//                ;; registered with IANA as standard or standards-track
	//

	class capability : public component
	{
	public:

		capability()
			: m_auth_type(NULL), m_atom(NULL)
		{
		}

		~capability()
		{
			delete (m_auth_type);
			delete (m_atom);
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("capability");

			size_t pos = *currentPos;

			class atom* at = parser.get <IMAPParser::atom>(line, &pos);

			string value = at->value();
			const char* str = value.c_str();

			if ((str[0] == 'a' || str[0] == 'A') &&
			    (str[1] == 'u' || str[1] == 'U') &&
			    (str[2] == 't' || str[2] == 'T') &&
			    (str[3] == 'h' || str[3] == 'H') &&
			    (str[4] == '='))
			{
				size_t pos = 5;
				m_auth_type = parser.get <IMAPParser::auth_type>(value, &pos);
				delete (at);
			}
			else
			{
				m_atom = at;
			}

			*currentPos = pos;
		}

	private:

		IMAPParser::auth_type* m_auth_type;
		IMAPParser::atom* m_atom;

	public:

		const IMAPParser::auth_type* auth_type() const { return (m_auth_type); }
		const IMAPParser::atom* atom() const { return (m_atom); }
	};


	//
	// capability_data ::= "CAPABILITY" SPACE [1#capability SPACE] "IMAP4rev1"
	//                     [SPACE 1#capability]
	//                     ;; IMAP4rev1 servers which offer RFC 1730
	//                     ;; compatibility MUST list "IMAP4" as the first
	//                     ;; capability.
	//

	class capability_data : public component
	{
	public:

		~capability_data()
		{
			for (std::vector <capability*>::iterator it = m_capabilities.begin() ;
			     it != m_capabilities.end() ; ++it)
			{
				delete (*it);
			}
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("capability_data");

			size_t pos = *currentPos;

			parser.checkWithArg <special_atom>(line, &pos, "capability");

			while (parser.check <SPACE>(line, &pos, true))
			{
				capability* cap;

				if (parser.isStrict() || m_capabilities.empty())
					cap = parser.get <capability>(line, &pos);
				else
					cap = parser.get <capability>(line, &pos, /* noThrow */ true);  // allow SPACE at end of line (Apple iCloud IMAP server)

				if (cap == NULL) break;

				m_capabilities.push_back(cap);
			}

			*currentPos = pos;
		}

	private:

		std::vector <capability*> m_capabilities;

	public:

		const std::vector <capability*>& capabilities() const { return (m_capabilities); }
	};


	//
	// date_day_fixed  ::= (SPACE digit) / 2digit
	//                    ;; Fixed-format version of date_day
	//
	// date_month      ::= "Jan" / "Feb" / "Mar" / "Apr" / "May" / "Jun" /
	//                    "Jul" / "Aug" / "Sep" / "Oct" / "Nov" / "Dec"
	//
	// date_year       ::= 4digit
	//
	// time            ::= 2digit ":" 2digit ":" 2digit
	//                     ;; Hours minutes seconds
	//
	// zone            ::= ("+" / "-") 4digit
	//                     ;; Signed four-digit value of hhmm representing
	//                     ;; hours and minutes west of Greenwich (that is,
	//                     ;; (the amount that the given time differs from
	//                     ;; Universal Time).  Subtracting the timezone
	//                     ;; from the given time will give the UT form.
	//                     ;; The Universal Time zone is "+0000".
	//
	// date_time       ::= <"> date_day_fixed "-" date_month "-" date_year
	//                     SPACE time SPACE zone <">
	//

	class date_time : public component
	{
	public:

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("date_time");

			size_t pos = *currentPos;

			// <"> date_day_fixed "-" date_month "-" date_year
			parser.check <one_char <'"'> >(line, &pos);
			parser.check <SPACE>(line, &pos, true);

			std::auto_ptr <number> nd(parser.get <number>(line, &pos));

			parser.check <one_char <'-'> >(line, &pos);

			std::auto_ptr <atom> amo(parser.get <atom>(line, &pos));

			parser.check <one_char <'-'> >(line, &pos);

			std::auto_ptr <number> ny(parser.get <number>(line, &pos));

			parser.check <SPACE>(line, &pos, true);

			// 2digit ":" 2digit ":" 2digit
			std::auto_ptr <number> nh(parser.get <number>(line, &pos));

			parser.check <one_char <':'> >(line, &pos);

			std::auto_ptr <number> nmi(parser.get <number>(line, &pos));

			parser.check <one_char <':'> >(line, &pos);

			std::auto_ptr <number> ns(parser.get <number>(line, &pos));

			parser.check <SPACE>(line, &pos, true);

			// ("+" / "-") 4digit
			int sign = 1;

			if (!(parser.check <one_char <'+'> >(line, &pos, true)))
				parser.check <one_char <'-'> >(line, &pos);

			std::auto_ptr <number> nz(parser.get <number>(line, &pos));

			parser.check <one_char <'"'> >(line, &pos);


			m_datetime.setHour(static_cast <int>(std::min(std::max(nh->value(), 0ul), 23ul)));
			m_datetime.setMinute(static_cast <int>(std::min(std::max(nmi->value(), 0ul), 59ul)));
			m_datetime.setSecond(static_cast <int>(std::min(std::max(ns->value(), 0ul), 59ul)));

			const int zone = static_cast <int>(nz->value());
			const int zh = zone / 100;   // hour offset
			const int zm = zone % 100;   // minute offset

			m_datetime.setZone(((zh * 60) + zm) * sign);

			m_datetime.setDay(static_cast <int>(std::min(std::max(nd->value(), 1ul), 31ul)));
			m_datetime.setYear(static_cast <int>(ny->value()));

			const string month(utility::stringUtils::toLower(amo->value()));
			int mon = vmime::datetime::JANUARY;

			if (month.length() >= 3)
			{
				switch (month[0])
				{
				case 'j':
				{
					switch (month[1])
					{
					case 'a': mon = vmime::datetime::JANUARY; break;
					case 'u':
					{
						switch (month[2])
						{
						case 'n': mon = vmime::datetime::JUNE; break;
						default:  mon = vmime::datetime::JULY; break;
						}

						break;
					}

					}

					break;
				}
				case 'f': mon = vmime::datetime::FEBRUARY; break;
				case 'm':
				{
					switch (month[2])
					{
					case 'r': mon = vmime::datetime::MARCH; break;
					default:  mon = vmime::datetime::MAY; break;
					}

					break;
				}
				case 'a':
				{
					switch (month[1])
					{
					case 'p': mon = vmime::datetime::APRIL; break;
					default:  mon = vmime::datetime::AUGUST; break;
					}

					break;
				}
				case 's': mon = vmime::datetime::SEPTEMBER; break;
				case 'o': mon = vmime::datetime::OCTOBER; break;
				case 'n': mon = vmime::datetime::NOVEMBER; break;
				case 'd': mon = vmime::datetime::DECEMBER; break;
				}
			}

			m_datetime.setMonth(mon);

			*currentPos = pos;
		}

	private:

		vmime::datetime m_datetime;
	};


	//
	// header_fld_name ::= astring
	//

	typedef astring header_fld_name;


	//
	// header_list     ::= "(" 1#header_fld_name ")"
	//

	class header_list : public component
	{
	public:

		~header_list()
		{
			for (std::vector <header_fld_name*>::iterator it = m_fld_names.begin() ;
			     it != m_fld_names.end() ; ++it)
			{
				delete (*it);
			}
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("header_list");

			size_t pos = *currentPos;

			parser.check <one_char <'('> >(line, &pos);

			while (!parser.check <one_char <')'> >(line, &pos, true))
			{
				m_fld_names.push_back(parser.get <header_fld_name>(line, &pos));
				parser.check <SPACE>(line, &pos, true);
			}

			*currentPos = pos;
		}

	private:

		std::vector <header_fld_name*> m_fld_names;

	public:

		const std::vector <header_fld_name*>& fld_names() const { return (m_fld_names); }
	};


	//
	// body_extension  ::= nstring / number / "(" 1#body_extension ")"
	//                     ;; Future expansion.  Client implementations
	//                     ;; MUST accept body_extension fields.  Server
	//                     ;; implementations MUST NOT generate
	//                     ;; body_extension fields except as defined by
	//                     ;; future standard or standards-track
	//                     ;; revisions of this specification.
	//

	class body_extension : public component
	{
	public:

		body_extension()
			: m_nstring(NULL), m_number(NULL)
		{
		}

		~body_extension()
		{
			delete (m_nstring);
			delete (m_number);

			for (std::vector <body_extension*>::iterator it = m_body_extensions.begin() ;
			     it != m_body_extensions.end() ; ++it)
			{
				delete (*it);
			}
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			if (parser.check <one_char <'('> >(line, &pos, true))
			{
				m_body_extensions.push_back
					(parser.get <body_extension>(line, &pos));

				while (!parser.check <one_char <')'> >(line, &pos, true))
				{
					m_body_extensions.push_back(parser.get <body_extension>(line, &pos));
					parser.check <SPACE>(line, &pos, true);
				}
			}
			else
			{
				if (!(m_nstring = parser.get <IMAPParser::nstring>(line, &pos, true)))
					m_number = parser.get <IMAPParser::number>(line, &pos);
			}

			*currentPos = pos;
		}

	private:

		IMAPParser::nstring* m_nstring;
		IMAPParser::number* m_number;

		std::vector <body_extension*> m_body_extensions;

	public:

		IMAPParser::nstring* nstring() const { return (m_nstring); }
		IMAPParser::number* number() const { return (m_number); }

		const std::vector <body_extension*>& body_extensions() const { return (m_body_extensions); }
	};


	//
	// section_text    ::= "HEADER" / "HEADER.FIELDS" [".NOT"]
	//                     SPACE header_list / "TEXT" / "MIME"
	//

	class section_text : public component
	{
	public:

		section_text()
			: m_header_list(NULL)
		{
		}

		~section_text()
		{
			delete (m_header_list);
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("section_text");

			size_t pos = *currentPos;

			// "HEADER.FIELDS" [".NOT"] SPACE header_list
			const bool b1 = parser.checkWithArg <special_atom>(line, &pos, "header.fields.not", true);
			const bool b2 = (b1 ? false : parser.checkWithArg <special_atom>(line, &pos, "header.fields", true));

			if (b1 || b2)
			{
				m_type = b1 ? HEADER_FIELDS_NOT : HEADER_FIELDS;

				parser.check <SPACE>(line, &pos);
				m_header_list = parser.get <IMAPParser::header_list>(line, &pos);
			}
			// "HEADER"
			else if (parser.checkWithArg <special_atom>(line, &pos, "header", true))
			{
				m_type = HEADER;
			}
			// "MIME"
			else if (parser.checkWithArg <special_atom>(line, &pos, "mime", true))
			{
				m_type = MIME;
			}
			// "TEXT"
			else
			{
				m_type = TEXT;

				parser.checkWithArg <special_atom>(line, &pos, "text");
			}

			*currentPos = pos;
		}


		enum Type
		{
			HEADER,
			HEADER_FIELDS,
			HEADER_FIELDS_NOT,
			MIME,
			TEXT
		};

	private:

		Type m_type;
		IMAPParser::header_list* m_header_list;

	public:

		Type type() const { return (m_type); }
		const IMAPParser::header_list* header_list() const { return (m_header_list); }
	};


	//
	// section         ::= "[" [section_text / (nz_number *["." nz_number]
	//                     ["." (section_text / "MIME")])] "]"
	//

	class section : public component
	{
	public:

		section()
			: m_section_text1(NULL), m_section_text2(NULL)
		{
		}

		~section()
		{
			delete (m_section_text1);
			delete (m_section_text2);
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("section");

			size_t pos = *currentPos;

			parser.check <one_char <'['> >(line, &pos);

			if (!parser.check <one_char <']'> >(line, &pos, true))
			{
				if (!(m_section_text1 = parser.get <section_text>(line, &pos, true)))
				{
					nz_number* num = parser.get <nz_number>(line, &pos);
					m_nz_numbers.push_back(static_cast <unsigned int>(num->value()));
					delete (num);

					while (parser.check <one_char <'.'> >(line, &pos, true))
					{
						if ((num = parser.get <nz_number>(line, &pos, true)))
						{
							m_nz_numbers.push_back(static_cast <unsigned int>(num->value()));
							delete (num);
						}
						else
						{
							m_section_text2 = parser.get <section_text>(line, &pos);
							break;
						}
					}
				}

				parser.check <one_char <']'> >(line, &pos);
			}

			*currentPos = pos;
		}

	private:

		section_text* m_section_text1;
		section_text* m_section_text2;
		std::vector <unsigned int> m_nz_numbers;

	public:

		const section_text* section_text1() const { return (m_section_text1); }
		const section_text* section_text2() const { return (m_section_text2); }
		const std::vector <unsigned int>& nz_numbers() const { return (m_nz_numbers); }
	};


	//
	// addr_adl        ::= nstring
	//                     ;; Holds route from [RFC-822] route-addr if
	//                     ;; non-NIL
	//
	// addr_host       ::= nstring
	//                     ;; NIL indicates [RFC-822] group syntax.
	//                     ;; Otherwise, holds [RFC-822] domain name
	//
	// addr_mailbox    ::= nstring
	//                     ;; NIL indicates end of [RFC-822] group; if
	//                     ;; non-NIL and addr_host is NIL, holds
	//                     ;; [RFC-822] group name.
	//                     ;; Otherwise, holds [RFC-822] local-part
	//
	// addr_name       ::= nstring
	//                     ;; Holds phrase from [RFC-822] mailbox if
	//                     ;; non-NIL
	//
	// address         ::= "(" addr_name SPACE addr_adl SPACE addr_mailbox
	//                     SPACE addr_host ")"
	//

	class address : public component
	{
	public:

		address()
			: m_addr_name(NULL), m_addr_adl(NULL),
			  m_addr_mailbox(NULL), m_addr_host(NULL)
		{
		}

		~address()
		{
			delete (m_addr_name);
			delete (m_addr_adl);
			delete (m_addr_mailbox);
			delete (m_addr_host);
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("address");

			size_t pos = *currentPos;

			parser.check <one_char <'('> >(line, &pos);
			m_addr_name = parser.get <nstring>(line, &pos);
			parser.check <SPACE>(line, &pos);
			m_addr_adl = parser.get <nstring>(line, &pos);
			parser.check <SPACE>(line, &pos);
			m_addr_mailbox = parser.get <nstring>(line, &pos);
			parser.check <SPACE>(line, &pos);
			m_addr_host = parser.get <nstring>(line, &pos);
			parser.check <one_char <')'> >(line, &pos);

			*currentPos = pos;
		}

	private:

		nstring* m_addr_name;
		nstring* m_addr_adl;
		nstring* m_addr_mailbox;
		nstring* m_addr_host;

	public:

		nstring* addr_name() const { return (m_addr_name); }
		nstring* addr_adl() const { return (m_addr_adl); }
		nstring* addr_mailbox() const { return (m_addr_mailbox); }
		nstring* addr_host() const { return (m_addr_host); }
	};


	//
	// address_list ::= "(" 1*address ")" / nil
	//

	class address_list : public component
	{
	public:

		~address_list()
		{
			for (std::vector <address*>::iterator it = m_addresses.begin() ;
			     it != m_addresses.end() ; ++it)
			{
				delete (*it);
			}
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("address_list");

			size_t pos = *currentPos;

			if (!parser.check <NIL>(line, &pos, true))
			{
				parser.check <one_char <'('> >(line, &pos);

				while (!parser.check <one_char <')'> >(line, &pos, true))
				{
					m_addresses.push_back(parser.get <address>(line, &pos));
					parser.check <SPACE>(line, &pos, true);
				}
			}

			*currentPos = pos;
		}

	private:

		std::vector <address*> m_addresses;

	public:

		const std::vector <address*>& addresses() const { return (m_addresses); }
	};


	//
	// env_bcc         ::= "(" 1*address ")" / nil
	//

	COMPONENT_ALIAS(address_list, env_bcc);


	//
	// env_cc          ::= "(" 1*address ")" / nil
	//

	COMPONENT_ALIAS(address_list, env_cc);


	//
	// env_date        ::= nstring
	//

	COMPONENT_ALIAS(nstring, env_date);


	//
	// env_from        ::= "(" 1*address ")" / nil
	//

	COMPONENT_ALIAS(address_list, env_from);


	//
	// env_in_reply_to ::= nstring
	//

	COMPONENT_ALIAS(nstring, env_in_reply_to);


	//
	// env_message_id  ::= nstring
	//

	COMPONENT_ALIAS(nstring, env_message_id);


	//
	// env_reply_to    ::= "(" 1*address ")" / nil
	//

	COMPONENT_ALIAS(address_list, env_reply_to);


	//
	// env_sender      ::= "(" 1*address ")" / nil
	//

	COMPONENT_ALIAS(address_list, env_sender);


	//
	// env_subject     ::= nstring
	//

	COMPONENT_ALIAS(nstring, env_subject);


	//
	// env_to          ::= "(" 1*address ")" / nil
	//

	COMPONENT_ALIAS(address_list, env_to);


	//
	// envelope ::= "(" env_date SPACE env_subject SPACE env_from
	//              SPACE env_sender SPACE env_reply_to SPACE env_to
	//              SPACE env_cc SPACE env_bcc SPACE env_in_reply_to
	//              SPACE env_message_id ")"
	//

	class envelope : public component
	{
	public:

		envelope()
			: m_env_date(NULL), m_env_subject(NULL),
			  m_env_from(NULL), m_env_sender(NULL), m_env_reply_to(NULL),
			  m_env_to(NULL), m_env_cc(NULL), m_env_bcc(NULL),
			  m_env_in_reply_to(NULL), m_env_message_id(NULL)
		{
		}

		~envelope()
		{
			delete (m_env_date);
			delete (m_env_subject);
			delete (m_env_from);
			delete (m_env_sender);
			delete (m_env_reply_to);
			delete (m_env_to);
			delete (m_env_cc);
			delete (m_env_bcc);
			delete (m_env_in_reply_to);
			delete (m_env_message_id);
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("envelope");

			size_t pos = *currentPos;

			parser.check <one_char <'('> >(line, &pos);

			m_env_date = parser.get <IMAPParser::env_date>(line, &pos);
			parser.check <SPACE>(line, &pos);

			m_env_subject = parser.get <IMAPParser::env_subject>(line, &pos);
			parser.check <SPACE>(line, &pos);

			m_env_from = parser.get <IMAPParser::env_from>(line, &pos);
			parser.check <SPACE>(line, &pos);

			m_env_sender = parser.get <IMAPParser::env_sender>(line, &pos);
			parser.check <SPACE>(line, &pos);

			m_env_reply_to = parser.get <IMAPParser::env_reply_to>(line, &pos);
			parser.check <SPACE>(line, &pos);

			m_env_to = parser.get <IMAPParser::env_to>(line, &pos);
			parser.check <SPACE>(line, &pos);

			m_env_cc = parser.get <IMAPParser::env_cc>(line, &pos);
			parser.check <SPACE>(line, &pos);

			m_env_bcc = parser.get <IMAPParser::env_bcc>(line, &pos);
			parser.check <SPACE>(line, &pos);

			m_env_in_reply_to = parser.get <IMAPParser::env_in_reply_to>(line, &pos);
			parser.check <SPACE>(line, &pos);

			m_env_message_id = parser.get <IMAPParser::env_message_id>(line, &pos);

			parser.check <one_char <')'> >(line, &pos);

			*currentPos = pos;
		}

	private:

		IMAPParser::env_date* m_env_date;
		IMAPParser::env_subject* m_env_subject;
		IMAPParser::env_from* m_env_from;
		IMAPParser::env_sender* m_env_sender;
		IMAPParser::env_reply_to* m_env_reply_to;
		IMAPParser::env_to* m_env_to;
		IMAPParser::env_cc* m_env_cc;
		IMAPParser::env_bcc* m_env_bcc;
		IMAPParser::env_in_reply_to* m_env_in_reply_to;
		IMAPParser::env_message_id* m_env_message_id;

	public:

		const IMAPParser::env_date* env_date() const { return (m_env_date); }
		const IMAPParser::env_subject* env_subject() const { return (m_env_subject); }
		const IMAPParser::env_from* env_from() const { return (m_env_from); }
		const IMAPParser::env_sender* env_sender() const { return (m_env_sender); }
		const IMAPParser::env_reply_to* env_reply_to() const { return (m_env_reply_to); }
		const IMAPParser::env_to* env_to() const { return (m_env_to); }
		const IMAPParser::env_cc* env_cc() const { return (m_env_cc); }
		const IMAPParser::env_bcc* env_bcc() const { return (m_env_bcc); }
		const IMAPParser::env_in_reply_to* env_in_reply_to() const { return (m_env_in_reply_to); }
		const IMAPParser::env_message_id* env_message_id() const { return (m_env_message_id); }
	};


	//
	// body_fld_desc   ::= nstring
	//

	typedef nstring body_fld_desc;


	//
	// body_fld_id     ::= nstring
	//

	typedef nstring body_fld_id;


	//
	// body_fld_md5    ::= nstring
	//

	typedef nstring body_fld_md5;


	//
	// body_fld_octets ::= number
	//

	typedef number body_fld_octets;


	//
	// body_fld_lines  ::= number
	//

	typedef number body_fld_lines;


	//
	// body_fld_enc    ::= (<"> ("7BIT" / "8BIT" / "BINARY" / "BASE64"/
	//                     "QUOTED-PRINTABLE") <">) / string
	//

	typedef xstring body_fld_enc;


	//
	// body_fld_param_item ::= string SPACE string
	//

	class body_fld_param_item : public component
	{
	public:

		body_fld_param_item()
			: m_string1(NULL), m_string2(NULL)
		{
		}

		~body_fld_param_item()
		{
			delete (m_string1);
			delete (m_string2);
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("body_fld_param_item");

			size_t pos = *currentPos;

			if (!parser.isStrict())
			{
				// Some servers send an <atom> instead of a <string> here:
				// eg. ... (CHARSET "X-UNKNOWN") ...
				if (!(m_string1 = parser.get <xstring>(line, &pos, true)))
				{
					std::auto_ptr <atom> at(parser.get <atom>(line, &pos));

					m_string1 = new xstring();
					m_string1->setValue(at->value());
				}
			}
			else
			{
				m_string1 = parser.get <xstring>(line, &pos);
			}

			parser.check <SPACE>(line, &pos);
			m_string2 = parser.get <xstring>(line, &pos);

			DEBUG_FOUND("body_fld_param_item", "<" << m_string1->value() << ", " << m_string2->value() << ">");

			*currentPos = pos;
		}

	private:

		xstring* m_string1;
		xstring* m_string2;

	public:

		const xstring* string1() const { return (m_string1); }
		const xstring* string2() const { return (m_string2); }
	};


	//
	// body_fld_param  ::= "(" 1#(body_fld_param_item) ")" / nil
	//

	class body_fld_param : public component
	{
	public:

		~body_fld_param()
		{
			for (std::vector <body_fld_param_item*>::iterator it = m_items.begin() ;
			     it != m_items.end() ; ++it)
			{
				delete (*it);
			}
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("body_fld_param");

			size_t pos = *currentPos;

			if (parser.check <one_char <'('> >(line, &pos, true))
			{
				m_items.push_back(parser.get <body_fld_param_item>(line, &pos));

				while (!parser.check <one_char <')'> >(line, &pos, true))
				{
					parser.check <SPACE>(line, &pos);
					m_items.push_back(parser.get <body_fld_param_item>(line, &pos));
				}
			}
			else
			{
				parser.check <NIL>(line, &pos);
			}

			*currentPos = pos;
		}

	private:

		std::vector <body_fld_param_item*> m_items;

	public:

		const std::vector <body_fld_param_item*>& items() const { return (m_items); }
	};


	//
	// body_fld_dsp    ::= "(" string SPACE body_fld_param ")" / nil
	//

	class body_fld_dsp : public component
	{
	public:

		body_fld_dsp()
			: m_string(NULL), m_body_fld_param(NULL)
		{
		}

		~body_fld_dsp()
		{
			delete (m_string);
			delete (m_body_fld_param);
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("body_fld_dsp");

			size_t pos = *currentPos;

			if (parser.check <one_char <'('> >(line, &pos, true))
			{
				m_string = parser.get <xstring>(line, &pos);
				parser.check <SPACE>(line, &pos);
				m_body_fld_param = parser.get <class body_fld_param>(line, &pos);
				parser.check <one_char <')'> >(line, &pos);
			}
			else
			{
				parser.check <NIL>(line, &pos);
			}

			*currentPos = pos;
		}

	private:

		class xstring* m_string;
		class body_fld_param* m_body_fld_param;

	public:

		const class xstring* str() const { return (m_string); }
		const class body_fld_param* body_fld_param() const { return (m_body_fld_param); }
	};


	//
	// body_fld_lang   ::= nstring / "(" 1#string ")"
	//

	class body_fld_lang : public component
	{
	public:

		~body_fld_lang()
		{
			for (std::vector <xstring*>::iterator it = m_strings.begin() ;
			     it != m_strings.end() ; ++it)
			{
				delete (*it);
			}
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("body_fld_lang");

			size_t pos = *currentPos;

			if (parser.check <one_char <'('> >(line, &pos, true))
			{
				m_strings.push_back(parser.get <class xstring>(line, &pos));

				while (!parser.check <one_char <')'> >(line, &pos, true))
				{
					parser.check <SPACE>(line, &pos);
					m_strings.push_back(parser.get <class xstring>(line, &pos));
				}
			}
			else
			{
				m_strings.push_back(parser.get <class nstring>(line, &pos));
			}

			*currentPos = pos;
		}

	private:

		std::vector <xstring*> m_strings;

	public:

		const std::vector <xstring*>& strings() const { return (m_strings); }
	};


	//
	// body_fields     ::= body_fld_param SPACE body_fld_id SPACE
	//                     body_fld_desc SPACE body_fld_enc SPACE
	//                     body_fld_octets
	//

	class body_fields : public component
	{
	public:

		body_fields()
			: m_body_fld_param(NULL), m_body_fld_id(NULL),
			  m_body_fld_desc(NULL), m_body_fld_enc(NULL), m_body_fld_octets(NULL)
		{
		}

		~body_fields()
		{
			delete (m_body_fld_param);
			delete (m_body_fld_id);
			delete (m_body_fld_desc);
			delete (m_body_fld_enc);
			delete (m_body_fld_octets);
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("body_fields");

			size_t pos = *currentPos;

			m_body_fld_param = parser.get <IMAPParser::body_fld_param>(line, &pos);
			parser.check <SPACE>(line, &pos);
			m_body_fld_id = parser.get <IMAPParser::body_fld_id>(line, &pos);
			parser.check <SPACE>(line, &pos);
			m_body_fld_desc = parser.get <IMAPParser::body_fld_desc>(line, &pos);
			parser.check <SPACE>(line, &pos);
			m_body_fld_enc = parser.get <IMAPParser::body_fld_enc>(line, &pos);
			parser.check <SPACE>(line, &pos);
			m_body_fld_octets = parser.get <IMAPParser::body_fld_octets>(line, &pos);

			*currentPos = pos;
		}

	private:

		IMAPParser::body_fld_param* m_body_fld_param;
		IMAPParser::body_fld_id* m_body_fld_id;
		IMAPParser::body_fld_desc* m_body_fld_desc;
		IMAPParser::body_fld_enc* m_body_fld_enc;
		IMAPParser::body_fld_octets* m_body_fld_octets;

	public:

		const IMAPParser::body_fld_param* body_fld_param() const { return (m_body_fld_param); }
		const IMAPParser::body_fld_id* body_fld_id() const { return (m_body_fld_id); }
		const IMAPParser::body_fld_desc* body_fld_desc() const { return (m_body_fld_desc); }
		const IMAPParser::body_fld_enc* body_fld_enc() const { return (m_body_fld_enc); }
		const IMAPParser::body_fld_octets* body_fld_octets() const { return (m_body_fld_octets); }
	};


	//
	// media_subtype   ::= string
	//                     ;; Defined in [MIME-IMT]
	//

	typedef xstring media_subtype;


	//
	// media_text      ::= <"> "TEXT" <"> SPACE media_subtype
	//                     ;; Defined in [MIME-IMT]
	//

	class media_text : public component
	{
	public:

		media_text()
			: m_media_subtype(NULL)
		{
		}

		~media_text()
		{
			delete (m_media_subtype);
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("media_text");

			size_t pos = *currentPos;

			parser.check <one_char <'"'> >(line, &pos);
			parser.checkWithArg <special_atom>(line, &pos, "text");
			parser.check <one_char <'"'> >(line, &pos);
			parser.check <SPACE>(line, &pos);

			m_media_subtype = parser.get <IMAPParser::media_subtype>(line, &pos);

			*currentPos = pos;
		}

	private:

		IMAPParser::media_subtype* m_media_subtype;

	public:

		const IMAPParser::media_subtype* media_subtype() const { return (m_media_subtype); }
	};


	//
	// media_message   ::= <"> "MESSAGE" <"> SPACE <"> "RFC822" <">
	//                     ;; Defined in [MIME-IMT]
	//

	class media_message : public component
	{
	public:

		media_message()
			: m_media_subtype(NULL)
		{
		}

		~media_message()
		{
			delete m_media_subtype;
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("media_message");

			size_t pos = *currentPos;

			parser.check <one_char <'"'> >(line, &pos);
			parser.checkWithArg <special_atom>(line, &pos, "message");
			parser.check <one_char <'"'> >(line, &pos);
			parser.check <SPACE>(line, &pos);

			//parser.check <one_char <'"'> >(line, &pos);
			//parser.checkWithArg <special_atom>(line, &pos, "rfc822");
			//parser.check <one_char <'"'> >(line, &pos);

			m_media_subtype = parser.get <IMAPParser::media_subtype>(line, &pos);

			*currentPos = pos;
		}

	private:

		IMAPParser::media_subtype* m_media_subtype;

	public:

		const IMAPParser::media_subtype* media_subtype() const { return (m_media_subtype); }
	};


	//
	// media_basic     ::= (<"> ("APPLICATION" / "AUDIO" / "IMAGE" /
	//                     "MESSAGE" / "VIDEO") <">) / string)
	//                     SPACE media_subtype
	//                     ;; Defined in [MIME-IMT]

	class media_basic : public component
	{
	public:

		media_basic()
			: m_media_type(NULL), m_media_subtype(NULL)
		{
		}

		~media_basic()
		{
			delete (m_media_type);
			delete (m_media_subtype);
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("media_basic");

			size_t pos = *currentPos;

			m_media_type = parser.get <xstring>(line, &pos);

			parser.check <SPACE>(line, &pos);

			m_media_subtype = parser.get <IMAPParser::media_subtype>(line, &pos);

			*currentPos = pos;
		}

	private:

		IMAPParser::xstring* m_media_type;
		IMAPParser::media_subtype* m_media_subtype;

	public:

		const IMAPParser::xstring* media_type() const { return (m_media_type); }
		const IMAPParser::media_subtype* media_subtype() const { return (m_media_subtype); }
	};


	//
	// body_ext_1part  ::= body_fld_md5 [SPACE body_fld_dsp
	//                     [SPACE body_fld_lang
	//                     [SPACE 1#body_extension]]]
	//                     ;; MUST NOT be returned on non-extensible
	//                     ;; "BODY" fetch
	//

	class body_ext_1part : public component
	{
	public:

		body_ext_1part()
			: m_body_fld_md5(NULL), m_body_fld_dsp(NULL), m_body_fld_lang(NULL)
		{
		}

		~body_ext_1part()
		{
			delete (m_body_fld_md5);
			delete (m_body_fld_dsp);
			delete (m_body_fld_lang);

			for (std::vector <body_extension*>::iterator it = m_body_extensions.begin() ;
			     it != m_body_extensions.end() ; ++it)
			{
				delete (*it);
			}
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("body_ext_1part");

			size_t pos = *currentPos;

			m_body_fld_md5 = parser.get <IMAPParser::body_fld_md5>(line, &pos);

			// [SPACE body_fld_dsp
			if (parser.check <SPACE>(line, &pos, true))
			{
				m_body_fld_dsp = parser.get <IMAPParser::body_fld_dsp>(line, &pos);

				// [SPACE body_fld_lang
				if (parser.check <SPACE>(line, &pos, true))
				{
					m_body_fld_lang = parser.get <IMAPParser::body_fld_lang>(line, &pos);

					// [SPACE 1#body_extension]
					if (parser.check <SPACE>(line, &pos, true))
					{
						m_body_extensions.push_back
							(parser.get <body_extension>(line, &pos));

						parser.check <SPACE>(line, &pos, true);

						body_extension* ext = NULL;

						while ((ext = parser.get <body_extension>(line, &pos, true)) != NULL)
						{
							m_body_extensions.push_back(ext);
							parser.check <SPACE>(line, &pos, true);
						}
					}
				}
			}

			*currentPos = pos;
		}

	private:

		IMAPParser::body_fld_md5* m_body_fld_md5;
		IMAPParser::body_fld_dsp* m_body_fld_dsp;
		IMAPParser::body_fld_lang* m_body_fld_lang;

		std::vector <body_extension*> m_body_extensions;

	public:

		const IMAPParser::body_fld_md5* body_fld_md5() const { return (m_body_fld_md5); }
		const IMAPParser::body_fld_dsp* body_fld_dsp() const { return (m_body_fld_dsp); }
		const IMAPParser::body_fld_lang* body_fld_lang() const { return (m_body_fld_lang); }

		const std::vector <body_extension*> body_extensions() const { return (m_body_extensions); }
	};


	//
	// body_ext_mpart  ::= body_fld_param
	//                     [SPACE body_fld_dsp SPACE body_fld_lang
	//                     [SPACE 1#body_extension]]
	//                     ;; MUST NOT be returned on non-extensible
	//                     ;; "BODY" fetch

	class body_ext_mpart : public component
	{
	public:

		body_ext_mpart()
			: m_body_fld_param(NULL), m_body_fld_dsp(NULL), m_body_fld_lang(NULL)
		{
		}

		~body_ext_mpart()
		{
			delete (m_body_fld_param);
			delete (m_body_fld_dsp);
			delete (m_body_fld_lang);

			for (std::vector <body_extension*>::iterator it = m_body_extensions.begin() ;
			     it != m_body_extensions.end() ; ++it)
			{
				delete (*it);
			}
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("body_ext_mpart");

			size_t pos = *currentPos;

			m_body_fld_param = parser.get <IMAPParser::body_fld_param>(line, &pos);

			// [SPACE body_fld_dsp SPACE body_fld_lang [SPACE 1#body_extension]]
			if (parser.check <SPACE>(line, &pos, true))
			{
				m_body_fld_dsp = parser.get <IMAPParser::body_fld_dsp>(line, &pos);
				parser.check <SPACE>(line, &pos);
				m_body_fld_lang = parser.get <IMAPParser::body_fld_lang>(line, &pos);

				// [SPACE 1#body_extension]
				if (parser.check <SPACE>(line, &pos, true))
				{
					m_body_extensions.push_back
						(parser.get <body_extension>(line, &pos));

					parser.check <SPACE>(line, &pos, true);

					body_extension* ext = NULL;

					while ((ext = parser.get <body_extension>(line, &pos, true)) != NULL)
					{
						m_body_extensions.push_back(ext);
						parser.check <SPACE>(line, &pos, true);
					}
				}
			}

			*currentPos = pos;
		}

	private:

		IMAPParser::body_fld_param* m_body_fld_param;
		IMAPParser::body_fld_dsp* m_body_fld_dsp;
		IMAPParser::body_fld_lang* m_body_fld_lang;

		std::vector <body_extension*> m_body_extensions;

	public:

		const IMAPParser::body_fld_param* body_fld_param() const { return (m_body_fld_param); }
		const IMAPParser::body_fld_dsp* body_fld_dsp() const { return (m_body_fld_dsp); }
		const IMAPParser::body_fld_lang* body_fld_lang() const { return (m_body_fld_lang); }

		const std::vector <body_extension*> body_extensions() const { return (m_body_extensions); }
	};


	//
	// body_type_basic ::= media_basic SPACE body_fields
	//                     ;; MESSAGE subtype MUST NOT be "RFC822"
	//

	class body_type_basic : public component
	{
	public:

		body_type_basic()
			: m_media_basic(NULL), m_body_fields(NULL)
		{
		}

		~body_type_basic()
		{
			delete (m_media_basic);
			delete (m_body_fields);
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("body_type_basic");

			size_t pos = *currentPos;

			m_media_basic = parser.get <IMAPParser::media_basic>(line, &pos);
			parser.check <SPACE>(line, &pos);
			m_body_fields = parser.get <IMAPParser::body_fields>(line, &pos);

			*currentPos = pos;
		}

	private:

		IMAPParser::media_basic* m_media_basic;
		IMAPParser::body_fields* m_body_fields;

	public:

		const IMAPParser::media_basic* media_basic() const { return (m_media_basic); }
		const IMAPParser::body_fields* body_fields() const { return (m_body_fields); }
	};


	//
	// body_type_msg   ::= media_message SPACE body_fields SPACE envelope
	//                     SPACE body SPACE body_fld_lines
	//

	class xbody;
	typedef xbody body;

	class body_type_msg : public component
	{
	public:

		body_type_msg()
			: m_media_message(NULL), m_body_fields(NULL),
			  m_envelope(NULL), m_body(NULL), m_body_fld_lines(NULL)
		{
		}

		~body_type_msg()
		{
			delete (m_media_message);
			delete (m_body_fields);
			delete (m_envelope);
			delete (m_body);
			delete (m_body_fld_lines);
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("body_type_msg");

			size_t pos = *currentPos;

			m_media_message = parser.get <IMAPParser::media_message>(line, &pos);
			parser.check <SPACE>(line, &pos);
			m_body_fields = parser.get <IMAPParser::body_fields>(line, &pos);
			parser.check <SPACE>(line, &pos);

			// BUGFIX: made SPACE optional. This is not standard, but some servers
			// seem to return responses like that...
			m_envelope = parser.get <IMAPParser::envelope>(line, &pos);
			parser.check <SPACE>(line, &pos, true);
			m_body = parser.get <IMAPParser::xbody>(line, &pos);
			parser.check <SPACE>(line, &pos, true);
			m_body_fld_lines = parser.get <IMAPParser::body_fld_lines>(line, &pos);

			*currentPos = pos;
		}

	private:

		IMAPParser::media_message* m_media_message;
		IMAPParser::body_fields* m_body_fields;
		IMAPParser::envelope* m_envelope;
		IMAPParser::xbody* m_body;
		IMAPParser::body_fld_lines* m_body_fld_lines;

	public:

		const IMAPParser::media_message* media_message() const { return (m_media_message); }
		const IMAPParser::body_fields* body_fields() const { return (m_body_fields); }
		const IMAPParser::envelope* envelope() const { return (m_envelope); }
		const IMAPParser::xbody* body() const { return (m_body); }
		const IMAPParser::body_fld_lines* body_fld_lines() const { return (m_body_fld_lines); }
	};


	//
	// body_type_text  ::= media_text SPACE body_fields SPACE body_fld_lines
	//

	class body_type_text : public component
	{
	public:

		body_type_text()
			: m_media_text(NULL),
			  m_body_fields(NULL), m_body_fld_lines(NULL)
		{
		}

		~body_type_text()
		{
			delete (m_media_text);
			delete (m_body_fields);
			delete (m_body_fld_lines);
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("body_type_text");

			size_t pos = *currentPos;

			m_media_text = parser.get <IMAPParser::media_text>(line, &pos);
			parser.check <SPACE>(line, &pos);
			m_body_fields = parser.get <IMAPParser::body_fields>(line, &pos);
			parser.check <SPACE>(line, &pos);
			m_body_fld_lines = parser.get <IMAPParser::body_fld_lines>(line, &pos);

			*currentPos = pos;
		}

	private:

		IMAPParser::media_text* m_media_text;
		IMAPParser::body_fields* m_body_fields;
		IMAPParser::body_fld_lines* m_body_fld_lines;

	public:

		const IMAPParser::media_text* media_text() const { return (m_media_text); }
		const IMAPParser::body_fields* body_fields() const { return (m_body_fields); }
		const IMAPParser::body_fld_lines* body_fld_lines() const { return (m_body_fld_lines); }
	};


	//
	// body_type_1part ::= (body_type_basic / body_type_msg / body_type_text)
	//                     [SPACE body_ext_1part]
	//

	class body_type_1part : public component
	{
	public:

		body_type_1part()
			: m_body_type_basic(NULL), m_body_type_msg(NULL),
			  m_body_type_text(NULL), m_body_ext_1part(NULL)
		{
		}

		~body_type_1part()
		{
			delete (m_body_type_basic);
			delete (m_body_type_msg);
			delete (m_body_type_text);

			delete (m_body_ext_1part);
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("body_type_1part");

			size_t pos = *currentPos;

			if (!(m_body_type_text = parser.get <IMAPParser::body_type_text>(line, &pos, true)))
				if (!(m_body_type_msg = parser.get <IMAPParser::body_type_msg>(line, &pos, true)))
					m_body_type_basic = parser.get <IMAPParser::body_type_basic>(line, &pos);

			if (parser.check <SPACE>(line, &pos, true))
			{
				m_body_ext_1part = parser.get <IMAPParser::body_ext_1part>(line, &pos, true);

				if (!m_body_ext_1part)
					--pos;
			}

			*currentPos = pos;
		}

	private:

		IMAPParser::body_type_basic* m_body_type_basic;
		IMAPParser::body_type_msg* m_body_type_msg;
		IMAPParser::body_type_text* m_body_type_text;

		IMAPParser::body_ext_1part* m_body_ext_1part;

	public:

		const IMAPParser::body_type_basic* body_type_basic() const { return (m_body_type_basic); }
		const IMAPParser::body_type_msg* body_type_msg() const { return (m_body_type_msg); }
		const IMAPParser::body_type_text* body_type_text() const { return (m_body_type_text); }

		const IMAPParser::body_ext_1part* body_ext_1part() const { return (m_body_ext_1part); }
	};


	//
	// body_type_mpart ::= 1*body SPACE media_subtype
	//                     [SPACE body_ext_mpart]
	//

	class body_type_mpart : public component
	{
	public:

		body_type_mpart()
			: m_media_subtype(NULL), m_body_ext_mpart(NULL)
		{
		}

		~body_type_mpart()
		{
			delete (m_media_subtype);
			delete (m_body_ext_mpart);

			for (std::vector <xbody*>::iterator it = m_list.begin() ;
			     it != m_list.end() ; ++it)
			{
				delete (*it);
			}
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("body_type_mpart");

			size_t pos = *currentPos;

			m_list.push_back(parser.get <xbody>(line, &pos));

			for (xbody* b ; (b = parser.get <xbody>(line, &pos, true)) ; )
				m_list.push_back(b);

			parser.check <SPACE>(line, &pos);

			m_media_subtype = parser.get <IMAPParser::media_subtype>(line, &pos);

			if (parser.check <SPACE>(line, &pos, true))
				m_body_ext_mpart = parser.get <IMAPParser::body_ext_mpart>(line, &pos);

			*currentPos = pos;
		}

	private:

		IMAPParser::media_subtype* m_media_subtype;
		IMAPParser::body_ext_mpart* m_body_ext_mpart;

		std::vector <xbody*> m_list;

	public:

		const std::vector <IMAPParser::xbody*>& list() const { return (m_list); }

		const IMAPParser::media_subtype* media_subtype() const { return (m_media_subtype); }
		const IMAPParser::body_ext_mpart* body_ext_mpart() const { return (m_body_ext_mpart); }
	};


	//
	// xbody ::= "(" body_type_1part / body_type_mpart ")"
	//

	class xbody : public component
	{
	public:

		xbody()
			: m_body_type_1part(NULL), m_body_type_mpart(NULL)
		{
		}

		~xbody()
		{
			delete (m_body_type_1part);
			delete (m_body_type_mpart);
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("body");

			size_t pos = *currentPos;

			parser.check <one_char <'('> >(line, &pos);

			if (!(m_body_type_mpart = parser.get <IMAPParser::body_type_mpart>(line, &pos, true)))
				m_body_type_1part = parser.get <IMAPParser::body_type_1part>(line, &pos);

			parser.check <one_char <')'> >(line, &pos);

			*currentPos = pos;
		}

	private:

		IMAPParser::body_type_1part* m_body_type_1part;
		IMAPParser::body_type_mpart* m_body_type_mpart;

	public:

		const IMAPParser::body_type_1part* body_type_1part() const { return (m_body_type_1part); }
		const IMAPParser::body_type_mpart* body_type_mpart() const { return (m_body_type_mpart); }
	};


	// msg_att_item ::= "ENVELOPE" SPACE envelope /
	//                  "FLAGS" SPACE "(" #(flag / "\Recent") ")" /
	//                  "INTERNALDATE" SPACE date_time /
	//                  "RFC822" [".HEADER" / ".TEXT"] SPACE nstring /
	//                  "RFC822.SIZE" SPACE number /
	//                  "BODY" ["STRUCTURE"] SPACE body /
	//                  "BODY" section ["<" number ">"] SPACE nstring /
	//                  "UID" SPACE uniqueid
	//
	// IMAP Extension for Conditional STORE (RFC-4551):
	//
	//   msg_att_item      /= "MODSEQ" SP "(" mod_sequence_value ")"

	class msg_att_item : public component
	{
	public:

		msg_att_item()
			: m_date_time(NULL), m_number(NULL), m_envelope(NULL),
			  m_uniqueid(NULL), m_nstring(NULL), m_body(NULL), m_flag_list(NULL),
			  m_section(NULL), m_mod_sequence_value(NULL)

		{
		}

		~msg_att_item()
		{
			delete (m_date_time);
			delete (m_number);
			delete (m_envelope);
			delete (m_uniqueid);
			delete (m_nstring);
			delete (m_body);
			delete (m_flag_list);
 			delete (m_section);
 			delete m_mod_sequence_value;
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("msg_att_item");

			size_t pos = *currentPos;

			// "ENVELOPE" SPACE envelope
			if (parser.checkWithArg <special_atom>(line, &pos, "envelope", true))
			{
				m_type = ENVELOPE;

				parser.check <SPACE>(line, &pos);
				m_envelope = parser.get <IMAPParser::envelope>(line, &pos);
			}
			// "FLAGS" SPACE "(" #(flag / "\Recent") ")"
			else if (parser.checkWithArg <special_atom>(line, &pos, "flags", true))
			{
				m_type = FLAGS;

				parser.check <SPACE>(line, &pos);

				m_flag_list = parser.get <IMAPParser::flag_list>(line, &pos);
			}
			// "INTERNALDATE" SPACE date_time
			else if (parser.checkWithArg <special_atom>(line, &pos, "internaldate", true))
			{
				m_type = INTERNALDATE;

				parser.check <SPACE>(line, &pos);
				m_date_time = parser.get <IMAPParser::date_time>(line, &pos);
			}
			// "RFC822" ".HEADER" SPACE nstring
			else if (parser.checkWithArg <special_atom>(line, &pos, "rfc822.header", true))
			{
				m_type = RFC822_HEADER;

				parser.check <SPACE>(line, &pos);

				m_nstring = parser.get <IMAPParser::nstring>(line, &pos);
			}
			// "RFC822" ".TEXT" SPACE nstring
			else if (parser.checkWithArg <special_atom>(line, &pos, "rfc822.text", true))
			{
				m_type = RFC822_TEXT;

				parser.check <SPACE>(line, &pos);

				m_nstring = parser.getWithArgs <IMAPParser::nstring>
					(line, &pos, this, RFC822_TEXT);
			}
			// "RFC822.SIZE" SPACE number
			else if (parser.checkWithArg <special_atom>(line, &pos, "rfc822.size", true))
			{
				m_type = RFC822_SIZE;

				parser.check <SPACE>(line, &pos);
				m_number = parser.get <IMAPParser::number>(line, &pos);
			}
			// "RFC822" SPACE nstring
			else if (parser.checkWithArg <special_atom>(line, &pos, "rfc822", true))
			{
				m_type = RFC822;

				parser.check <SPACE>(line, &pos);

				m_nstring = parser.get <IMAPParser::nstring>(line, &pos);
			}
			// "BODY" "STRUCTURE" SPACE body
			else if (parser.checkWithArg <special_atom>(line, &pos, "bodystructure", true))
			{
				m_type = BODY_STRUCTURE;

				parser.check <SPACE>(line, &pos);

				m_body = parser.get <IMAPParser::body>(line, &pos);
			}
			// "BODY" section ["<" number ">"] SPACE nstring
			// "BODY" SPACE body
			else if (parser.checkWithArg <special_atom>(line, &pos, "body", true))
			{
				m_section = parser.get <IMAPParser::section>(line, &pos, true);

				// "BODY" section ["<" number ">"] SPACE nstring
				if (m_section != NULL)
				{
					m_type = BODY_SECTION;

					if (parser.check <one_char <'<'> >(line, &pos, true))
					{
						m_number = parser.get <IMAPParser::number>(line, &pos);
						parser.check <one_char <'>'> >(line, &pos);
					}

					parser.check <SPACE>(line, &pos);

					m_nstring = parser.getWithArgs <IMAPParser::nstring>
						(line, &pos, this, BODY_SECTION);
				}
				// "BODY" SPACE body
				else
				{
					m_type = BODY;

					parser.check <SPACE>(line, &pos);

					m_body = parser.get <IMAPParser::body>(line, &pos);
				}
			}
			// "MODSEQ" SP "(" mod_sequence_value ")"
			else if (parser.checkWithArg <special_atom>(line, &pos, "modseq", true))
			{
				m_type = MODSEQ;

				parser.check <SPACE>(line, &pos);
				parser.check <one_char <'('> >(line, &pos);

				m_mod_sequence_value = parser.get <IMAPParser::mod_sequence_value>(line, &pos);

				parser.check <one_char <')'> >(line, &pos);
			}
			// "UID" SPACE uniqueid
			else
			{
				m_type = UID;

				parser.checkWithArg <special_atom>(line, &pos, "uid");
				parser.check <SPACE>(line, &pos);

				m_uniqueid = parser.get <uniqueid>(line, &pos);
			}

			*currentPos = pos;
		}


		enum Type
		{
			ENVELOPE,
			FLAGS,
			INTERNALDATE,
			RFC822,
			RFC822_SIZE,
			RFC822_HEADER,
			RFC822_TEXT,
			BODY,
			BODY_SECTION,
			BODY_STRUCTURE,
			UID,
			MODSEQ
		};

	private:

		Type m_type;

		IMAPParser::date_time* m_date_time;
		IMAPParser::number* m_number;
		IMAPParser::envelope* m_envelope;
		IMAPParser::uniqueid* m_uniqueid;
		IMAPParser::nstring* m_nstring;
		IMAPParser::xbody* m_body;
		IMAPParser::flag_list* m_flag_list;
		IMAPParser::section* m_section;
		IMAPParser::mod_sequence_value* m_mod_sequence_value;

	public:

		Type type() const { return (m_type); }

		const IMAPParser::date_time* date_time() const { return (m_date_time); }
		const IMAPParser::number* number() const { return (m_number); }
		const IMAPParser::envelope* envelope() const { return (m_envelope); }
		const IMAPParser::uniqueid* unique_id() const { return (m_uniqueid); }
		const IMAPParser::nstring* nstring() const { return (m_nstring); }
		const IMAPParser::xbody* body() const { return (m_body); }
		const IMAPParser::flag_list* flag_list() const { return (m_flag_list); }
		const IMAPParser::section* section() const { return (m_section); }
		const IMAPParser::mod_sequence_value* mod_sequence_value() { return m_mod_sequence_value; }
	};


	//
	// msg_att ::= "(" 1#(msg_att_item) ")"
	//

	class msg_att : public component
	{
	public:

		~msg_att()
		{
			for (std::vector <msg_att_item*>::iterator it = m_items.begin() ;
			     it != m_items.end() ; ++it)
			{
				delete (*it);
			}
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("msg_att");

			size_t pos = *currentPos;

			parser.check <one_char <'('> >(line, &pos);

			m_items.push_back(parser.get <msg_att_item>(line, &pos));

			while (!parser.check <one_char <')'> >(line, &pos, true))
			{
				parser.check <SPACE>(line, &pos);
				m_items.push_back(parser.get <msg_att_item>(line, &pos));
			}

			*currentPos = pos;
		}

	private:

		std::vector <msg_att_item*> m_items;

	public:

		const std::vector <msg_att_item*>& items() const { return (m_items); }
	};


	//
	// message_data ::= nz_number SPACE ("EXPUNGE" /
	//                  ("FETCH" SPACE msg_att))
	//

	class message_data : public component
	{
	public:

		message_data()
			: m_number(0), m_msg_att(NULL)
		{
		}

		~message_data()
		{
			delete (m_msg_att);
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("message_data");

			size_t pos = *currentPos;

			nz_number* num = parser.get <nz_number>(line, &pos);
			m_number = static_cast <unsigned int>(num->value());
			delete (num);

			parser.check <SPACE>(line, &pos);

			if (parser.checkWithArg <special_atom>(line, &pos, "expunge", true))
			{
				m_type = EXPUNGE;
			}
			else
			{
				parser.checkWithArg <special_atom>(line, &pos, "fetch");

				parser.check <SPACE>(line, &pos);

				m_type = FETCH;
				m_msg_att = parser.get <IMAPParser::msg_att>(line, &pos);
			}

			*currentPos = pos;
		}


		enum Type
		{
			EXPUNGE,
			FETCH
		};

	private:

		Type m_type;
		unsigned int m_number;
		IMAPParser::msg_att* m_msg_att;

	public:

		Type type() const { return (m_type); }
		unsigned int number() const { return (m_number); }
		const IMAPParser::msg_att* msg_att() const { return (m_msg_att); }
	};


	//
	// resp_text_code ::= "ALERT" / "PARSE" /
	//                    capability-data /
	//                    "PERMANENTFLAGS" SPACE "(" #(flag / "\*") ")" /
	//                    "READ-ONLY" / "READ-WRITE" / "TRYCREATE" /
	//                    "UIDVALIDITY" SPACE nz_number /
	//                    "UNSEEN" SPACE nz_number /
	//                    "UIDNEXT" SPACE nz-number /
	//                    atom [SPACE 1*<any TEXT_CHAR except "]">]
	//
	// IMAP Extension for Conditional STORE (RFC-4551):
	//
	//   resp-text-code      =/ "HIGHESTMODSEQ" SP mod-sequence-value /
	//                          "NOMODSEQ" /
	//                          "MODIFIED" SP set
	//
	// IMAP UIDPLUS Extension (RFC-4315):
	//
	//   resp-text-code      =/ "APPENDUID" SP nz-number SP append-uid /
	//                          "COPYUID" SP nz-number SP uid-set SP uid-set /
	//                          "UIDNOTSTICKY"

	class resp_text_code : public component
	{
	public:

		resp_text_code()
			: m_nz_number(NULL), m_atom(NULL), m_flag_list(NULL),
			  m_text(NULL), m_capability_data(NULL)
		{
		}

		~resp_text_code()
		{
			delete (m_nz_number);
			delete (m_atom);
			delete (m_flag_list);
			delete (m_text);
			delete m_capability_data;
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("resp_text_code");

			size_t pos = *currentPos;

			// "ALERT"
			if (parser.checkWithArg <special_atom>(line, &pos, "alert", true))
			{
				m_type = ALERT;
			}
			// "PARSE"
			else if (parser.checkWithArg <special_atom>(line, &pos, "parse", true))
			{
				m_type = PARSE;
			}
			// capability_data
			else if ((m_capability_data = parser.get <IMAPParser::capability_data>(line, &pos, true)))
			{
				m_type = CAPABILITY;
			}
			// "PERMANENTFLAGS" SPACE flag_list
			else if (parser.checkWithArg <special_atom>(line, &pos, "permanentflags", true))
			{
				m_type = PERMANENTFLAGS;

				parser.check <SPACE>(line, &pos);

				m_flag_list = parser.get <IMAPParser::flag_list>(line, &pos);
			}
			// "READ-ONLY"
			else if (parser.checkWithArg <special_atom>(line, &pos, "read-only", true))
			{
				m_type = READ_ONLY;
			}
			// "READ-WRITE"
			else if (parser.checkWithArg <special_atom>(line, &pos, "read-write", true))
			{
				m_type = READ_WRITE;
			}
			// "TRYCREATE"
			else if (parser.checkWithArg <special_atom>(line, &pos, "trycreate", true))
			{
				m_type = TRYCREATE;
			}
			// "UIDVALIDITY" SPACE nz_number
			else if (parser.checkWithArg <special_atom>(line, &pos, "uidvalidity", true))
			{
				m_type = UIDVALIDITY;

				parser.check <SPACE>(line, &pos);
				m_nz_number = parser.get <IMAPParser::nz_number>(line, &pos);
			}
			// "UIDNEXT" SPACE nz_number
			else if (parser.checkWithArg <special_atom>(line, &pos, "uidnext", true))
			{
				m_type = UIDNEXT;

				parser.check <SPACE>(line, &pos);
				m_nz_number = parser.get <IMAPParser::nz_number>(line, &pos);
			}
			// "UNSEEN" SPACE nz_number
			else if (parser.checkWithArg <special_atom>(line, &pos, "unseen", true))
			{
				m_type = UNSEEN;

				parser.check <SPACE>(line, &pos);
				m_nz_number = parser.get <IMAPParser::nz_number>(line, &pos);
			}
			// "HIGHESTMODSEQ" SP mod-sequence-value
			else if (parser.checkWithArg <special_atom>(line, &pos, "highestmodseq", true))
			{
				m_type = HIGHESTMODSEQ;

				parser.check <SPACE>(line, &pos);
				m_mod_sequence_value = parser.get <IMAPParser::mod_sequence_value>(line, &pos);
			}
			// "NOMODSEQ"
			else if (parser.checkWithArg <special_atom>(line, &pos, "nomodseq", true))
			{
				m_type = NOMODSEQ;
			}
			// "MODIFIED" SP sequence-set
			else if (parser.checkWithArg <special_atom>(line, &pos, "modified", true))
			{
				m_type = MODIFIED;

				parser.check <SPACE>(line, &pos);

				m_sequence_set = parser.get <IMAPParser::sequence_set>(line, &pos);
			}
			// "APPENDUID" SP nz-number SP append-uid
			else if (parser.checkWithArg <special_atom>(line, &pos, "appenduid", true))
			{
				m_type = APPENDUID;

				parser.check <SPACE>(line, &pos);
				m_nz_number = parser.get <IMAPParser::nz_number>(line, &pos);
				parser.check <SPACE>(line, &pos);
				m_uid_set = parser.get <IMAPParser::uid_set>(line, &pos);
			}
			// "COPYUID" SP nz-number SP uid-set SP uid-set
			else if (parser.checkWithArg <special_atom>(line, &pos, "copyuid", true))
			{
				m_type = COPYUID;

				parser.check <SPACE>(line, &pos);
				m_nz_number = parser.get <IMAPParser::nz_number>(line, &pos);
				parser.check <SPACE>(line, &pos);
				m_uid_set = parser.get <IMAPParser::uid_set>(line, &pos);
				parser.check <SPACE>(line, &pos);
				m_uid_set2 = parser.get <IMAPParser::uid_set>(line, &pos);
			}
			// "UIDNOTSTICKY"
			else if (parser.checkWithArg <special_atom>(line, &pos, "uidnotsticky", true))
			{
				m_type = UIDNOTSTICKY;
			}
			// atom [SPACE 1*<any TEXT_CHAR except "]">]
			else
			{
				m_type = OTHER;

				m_atom = parser.get <IMAPParser::atom>(line, &pos);

				if (parser.check <SPACE>(line, &pos, true))
					m_text = parser.get <text_except <']'> >(line, &pos);
			}

			*currentPos = pos;
		}


		enum Type
		{
			// Extensions
			HIGHESTMODSEQ,
			NOMODSEQ,
			MODIFIED,
			APPENDUID,
			COPYUID,
			UIDNOTSTICKY,

			// Standard IMAP
			ALERT,
			PARSE,
			CAPABILITY,
			PERMANENTFLAGS,
			READ_ONLY,
			READ_WRITE,
			TRYCREATE,
			UIDVALIDITY,
			UIDNEXT,
			UNSEEN,
			OTHER
		};

	private:

		Type m_type;

		IMAPParser::nz_number* m_nz_number;
		IMAPParser::atom* m_atom;
		IMAPParser::flag_list* m_flag_list;
		IMAPParser::text* m_text;
		IMAPParser::mod_sequence_value* m_mod_sequence_value;
		IMAPParser::sequence_set* m_sequence_set;
		IMAPParser::capability_data* m_capability_data;
		IMAPParser::uid_set* m_uid_set;
		IMAPParser::uid_set* m_uid_set2;

	public:

		Type type() const { return (m_type); }

		const IMAPParser::nz_number* nz_number() const { return (m_nz_number); }
		const IMAPParser::atom* atom() const { return (m_atom); }
		const IMAPParser::flag_list* flag_list() const { return (m_flag_list); }
		const IMAPParser::text* text() const { return (m_text); }
		const IMAPParser::mod_sequence_value* mod_sequence_value() const { return m_mod_sequence_value; }
		const IMAPParser::sequence_set* sequence_set() const { return m_sequence_set; }
		const IMAPParser::capability_data* capability_data() const { return m_capability_data; }
		const IMAPParser::uid_set* uid_set() const { return (m_uid_set); }
		const IMAPParser::uid_set* uid_set2() const { return (m_uid_set2); }
	};


	//
	// resp_text ::= ["[" resp_text_code "]" SPACE] (text_mime2 / text)
	//               ;; text SHOULD NOT begin with "[" or "="

	class resp_text : public component
	{
	public:

		resp_text()
			: m_resp_text_code(NULL)
		{
		}

		~resp_text()
		{
			delete (m_resp_text_code);
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("resp_text");

			size_t pos = *currentPos;

			if (parser.check <one_char <'['> >(line, &pos, true))
			{
				m_resp_text_code = parser.get <IMAPParser::resp_text_code>(line, &pos);

				parser.check <one_char <']'> >(line, &pos);
				parser.check <SPACE>(line, &pos, true);
			}

			text_mime2* text1 = parser.get <text_mime2>(line, &pos, true);

			if (text1 != NULL)
			{
				m_text = text1->value();
				delete (text1);
			}
			else
			{
				IMAPParser::text* text2 =
					parser.get <IMAPParser::text>(line, &pos, true);

				if (text2 != NULL)
				{
					m_text = text2->value();
					delete (text2);
				}
				else
				{
					// Empty response text
				}
			}

			*currentPos = pos;
		}

	private:

		IMAPParser::resp_text_code* m_resp_text_code;
		string m_text;

	public:

		const IMAPParser::resp_text_code* resp_text_code() const { return (m_resp_text_code); }
		const string& text() const { return (m_text); }
	};


	//
	// continue_req   ::= "+" SPACE (resp_text / base64)
	//

	class continue_req : public component
	{
	public:

		continue_req()
			: m_resp_text(NULL)
		{
		}

		~continue_req()
		{
			delete (m_resp_text);
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("continue_req");

			size_t pos = *currentPos;

			parser.check <one_char <'+'> >(line, &pos);

			if (!parser.isStrict())
			{
				// Some servers do not send SPACE when response text is empty
				if (parser.check <SPACE>(line, &pos, true))
					m_resp_text = parser.get <IMAPParser::resp_text>(line, &pos);
				else
					m_resp_text = new IMAPParser::resp_text();  // empty
			}
			else
			{
				parser.check <SPACE>(line, &pos);

				m_resp_text = parser.get <IMAPParser::resp_text>(line, &pos);
			}

			parser.check <CRLF>(line, &pos);

			*currentPos = pos;
		}

	private:

		IMAPParser::resp_text* m_resp_text;

	public:

		const IMAPParser::resp_text* resp_text() const { return (m_resp_text); }
	};


	//
	// resp_cond_state ::= ("OK" / "NO" / "BAD") SPACE resp_text
	//                     ;; Status condition
	//

	class resp_cond_state : public component
	{
	public:

		resp_cond_state()
			: m_resp_text(NULL), m_status(BAD)
		{
		}

		~resp_cond_state()
		{
			delete (m_resp_text);
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("resp_cond_state");

			size_t pos = *currentPos;

			if (parser.checkWithArg <special_atom>(line, &pos, "ok", true))
			{
				m_status = OK;
			}
			else if (parser.checkWithArg <special_atom>(line, &pos, "no", true))
			{
				m_status = NO;
			}
			else
			{
				parser.checkWithArg <special_atom>(line, &pos, "bad");
				m_status = BAD;
			}

			parser.check <SPACE>(line, &pos);

			m_resp_text = parser.get <IMAPParser::resp_text>(line, &pos);

			*currentPos = pos;
		}


		enum Status
		{
			OK,
			NO,
			BAD
		};

	private:

		IMAPParser::resp_text* m_resp_text;
		Status m_status;

	public:

		const IMAPParser::resp_text* resp_text() const { return (m_resp_text); }
		Status status() const { return (m_status); }
	};


	//
	// resp_cond_bye ::= "BYE" SPACE resp_text
	//

	class resp_cond_bye : public component
	{
	public:

		resp_cond_bye()
			: m_resp_text(NULL)
		{
		}

		~resp_cond_bye()
		{
			delete (m_resp_text);
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("resp_cond_bye");

			size_t pos = *currentPos;

			parser.checkWithArg <special_atom>(line, &pos, "bye");

			parser.check <SPACE>(line, &pos);

			m_resp_text = parser.get <IMAPParser::resp_text>(line, &pos);

			*currentPos = pos;
		}

	private:

		IMAPParser::resp_text* m_resp_text;

	public:

		const IMAPParser::resp_text* resp_text() const { return (m_resp_text); }
	};


	//
	// resp_cond_auth  ::= ("OK" / "PREAUTH") SPACE resp_text
	//                     ;; Authentication condition
	//

	class resp_cond_auth : public component
	{
	public:

		resp_cond_auth()
			: m_resp_text(NULL)
		{
		}

		~resp_cond_auth()
		{
			delete (m_resp_text);
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("resp_cond_auth");

			size_t pos = *currentPos;

			if (parser.checkWithArg <special_atom>(line, &pos, "ok", true))
			{
				m_cond = OK;
			}
			else
			{
				parser.checkWithArg <special_atom>(line, &pos, "preauth");

				m_cond = PREAUTH;
			}

			parser.check <SPACE>(line, &pos);

			m_resp_text = parser.get <IMAPParser::resp_text>(line, &pos);

			*currentPos = pos;
		}


		enum Condition
		{
			OK,
			PREAUTH
		};

	private:

		Condition m_cond;
		IMAPParser::resp_text* m_resp_text;

	public:

		Condition condition() const { return (m_cond); }
		const IMAPParser::resp_text* resp_text() const { return (m_resp_text); }
	};


	//
	// mailbox_data ::= "FLAGS" SPACE mailbox_flag_list /
	//                  "LIST" SPACE mailbox_list /
	//                  "LSUB" SPACE mailbox_list /
	//                  "MAILBOX" SPACE text /
	//                  "SEARCH" [SPACE 1#nz_number] /
	//                  "STATUS" SPACE mailbox SPACE
	//                    "(" [status-att-list] ")" /
	//                  number SPACE "EXISTS" /
	//                  number SPACE "RECENT"
	//

	class mailbox_data : public component
	{
	public:

		mailbox_data()
			: m_number(NULL), m_mailbox_flag_list(NULL), m_mailbox_list(NULL),
			  m_mailbox(NULL), m_text(NULL), m_status_att_list(NULL)
		{
		}

		~mailbox_data()
		{
			delete (m_number);
			delete (m_mailbox_flag_list);
			delete (m_mailbox_list);
			delete (m_mailbox);
			delete (m_text);

			for (std::vector <nz_number*>::iterator it = m_search_nz_number_list.begin() ;
			     it != m_search_nz_number_list.end() ; ++it)
			{
				delete (*it);
			}

			delete m_status_att_list;
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("mailbox_data");

			size_t pos = *currentPos;

			m_number = parser.get <IMAPParser::number>(line, &pos, true);

			if (m_number)
			{
				parser.check <SPACE>(line, &pos);

				if (parser.checkWithArg <special_atom>(line, &pos, "exists", true))
				{
					m_type = EXISTS;
				}
				else
				{
					parser.checkWithArg <special_atom>(line, &pos, "recent");

					m_type = RECENT;
				}
			}
			else
			{
				// "FLAGS" SPACE mailbox_flag_list
				if (parser.checkWithArg <special_atom>(line, &pos, "flags", true))
				{
					parser.check <SPACE>(line, &pos);

					m_mailbox_flag_list = parser.get <IMAPParser::mailbox_flag_list>(line, &pos);

					m_type = FLAGS;
				}
				// "LIST" SPACE mailbox_list
				else if (parser.checkWithArg <special_atom>(line, &pos, "list", true))
				{
					parser.check <SPACE>(line, &pos);

					m_mailbox_list = parser.get <IMAPParser::mailbox_list>(line, &pos);

					m_type = LIST;
				}
				// "LSUB" SPACE mailbox_list
				else if (parser.checkWithArg <special_atom>(line, &pos, "lsub", true))
				{
					parser.check <SPACE>(line, &pos);

					m_mailbox_list = parser.get <IMAPParser::mailbox_list>(line, &pos);

					m_type = LSUB;
				}
				// "MAILBOX" SPACE text
				else if (parser.checkWithArg <special_atom>(line, &pos, "mailbox", true))
				{
					parser.check <SPACE>(line, &pos);

					m_text = parser.get <IMAPParser::text>(line, &pos);

					m_type = MAILBOX;
				}
				// "SEARCH" [SPACE 1#nz_number]
				else if (parser.checkWithArg <special_atom>(line, &pos, "search", true))
				{
					if (parser.check <SPACE>(line, &pos, true))
					{
						m_search_nz_number_list.push_back
							(parser.get <nz_number>(line, &pos));

						while (parser.check <SPACE>(line, &pos, true))
						{
							m_search_nz_number_list.push_back
								(parser.get <nz_number>(line, &pos));
						}
					}

					m_type = SEARCH;
				}
				// "STATUS" SPACE mailbox SPACE
				// "(" [status_att_list] ")"
				else
				{
					parser.checkWithArg <special_atom>(line, &pos, "status");
					parser.check <SPACE>(line, &pos);

					m_mailbox = parser.get <IMAPParser::mailbox>(line, &pos);

					parser.check <SPACE>(line, &pos);

					parser.check <one_char <'('> >(line, &pos);

					m_status_att_list = parser.get <IMAPParser::status_att_list>(line, &pos, true);

					parser.check <one_char <')'> >(line, &pos);

					m_type = STATUS;
				}
			}

			*currentPos = pos;
		}


		enum Type
		{
			FLAGS,
			LIST,
			LSUB,
			MAILBOX,
			SEARCH,
			STATUS,
			EXISTS,
			RECENT
		};

	private:

		Type m_type;

		IMAPParser::number* m_number;
		IMAPParser::mailbox_flag_list* m_mailbox_flag_list;
		IMAPParser::mailbox_list* m_mailbox_list;
		IMAPParser::mailbox* m_mailbox;
		IMAPParser::text* m_text;
		std::vector <nz_number*> m_search_nz_number_list;
		IMAPParser::status_att_list* m_status_att_list;

	public:

		Type type() const { return (m_type); }

		const IMAPParser::number* number() const { return (m_number); }
		const IMAPParser::mailbox_flag_list* mailbox_flag_list() const { return (m_mailbox_flag_list); }
		const IMAPParser::mailbox_list* mailbox_list() const { return (m_mailbox_list); }
		const IMAPParser::mailbox* mailbox() const { return (m_mailbox); }
		const IMAPParser::text* text() const { return (m_text); }
		const std::vector <nz_number*>& search_nz_number_list() const { return (m_search_nz_number_list); }
		const IMAPParser::status_att_list* status_att_list() const { return m_status_att_list; }
	};


	//
	// response_data  ::= "*" SPACE (resp_cond_state / resp_cond_bye /
	//                    mailbox_data / message_data / capability_data) CRLF
	//

	class response_data : public component
	{
	public:

		response_data()
			: m_resp_cond_state(NULL), m_resp_cond_bye(NULL),
			  m_mailbox_data(NULL), m_message_data(NULL), m_capability_data(NULL)
		{
		}

		~response_data()
		{
			delete (m_resp_cond_state);
			delete (m_resp_cond_bye);
			delete (m_mailbox_data);
			delete (m_message_data);
			delete (m_capability_data);
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("response_data");

			size_t pos = *currentPos;

			parser.check <one_char <'*'> >(line, &pos);
			parser.check <SPACE>(line, &pos);

			if (!(m_resp_cond_state = parser.get <IMAPParser::resp_cond_state>(line, &pos, true)))
				if (!(m_resp_cond_bye = parser.get <IMAPParser::resp_cond_bye>(line, &pos, true)))
					if (!(m_mailbox_data = parser.get <IMAPParser::mailbox_data>(line, &pos, true)))
						if (!(m_message_data = parser.get <IMAPParser::message_data>(line, &pos, true)))
							m_capability_data = parser.get <IMAPParser::capability_data>(line, &pos);

			if (!parser.isStrict())
			{
				// Allow SPACEs at end of line
				while (parser.check <SPACE>(line, &pos, /* noThrow */ true))
					;
			}

			parser.check <CRLF>(line, &pos);

			*currentPos = pos;
		}

	private:

		IMAPParser::resp_cond_state* m_resp_cond_state;
		IMAPParser::resp_cond_bye* m_resp_cond_bye;
		IMAPParser::mailbox_data* m_mailbox_data;
		IMAPParser::message_data* m_message_data;
		IMAPParser::capability_data* m_capability_data;

	public:

		const IMAPParser::resp_cond_state* resp_cond_state() const { return (m_resp_cond_state); }
		const IMAPParser::resp_cond_bye* resp_cond_bye() const { return (m_resp_cond_bye); }
		const IMAPParser::mailbox_data* mailbox_data() const { return (m_mailbox_data); }
		const IMAPParser::message_data* message_data() const { return (m_message_data); }
		const IMAPParser::capability_data* capability_data() const { return (m_capability_data); }
	};


	class continue_req_or_response_data : public component
	{
	public:

		continue_req_or_response_data()
			: m_continue_req(NULL), m_response_data(NULL)
		{
		}

		~continue_req_or_response_data()
		{
			delete (m_continue_req);
			delete (m_response_data);
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("continue_req_or_response_data");

			size_t pos = *currentPos;

			if (!(m_continue_req = parser.get <IMAPParser::continue_req>(line, &pos, true)))
				m_response_data = parser.get <IMAPParser::response_data>(line, &pos);

			*currentPos = pos;
		}

	private:

		IMAPParser::continue_req* m_continue_req;
		IMAPParser::response_data* m_response_data;

	public:

		const IMAPParser::continue_req* continue_req() const { return (m_continue_req); }
		const IMAPParser::response_data* response_data() const { return (m_response_data); }
	};


	//
	// response_fatal ::= "*" SPACE resp_cond_bye CRLF
	//                    ;; Server closes connection immediately
	//

	class response_fatal : public component
	{
	public:

		response_fatal()
			: m_resp_cond_bye(NULL)
		{
		}

		~response_fatal()
		{
			delete (m_resp_cond_bye);
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("response_fatal");

			size_t pos = *currentPos;

			parser.check <one_char <'*'> >(line, &pos);
			parser.check <SPACE>(line, &pos);

			m_resp_cond_bye = parser.get <IMAPParser::resp_cond_bye>(line, &pos);

			if (!parser.isStrict())
			{
				// Allow SPACEs at end of line
				while (parser.check <SPACE>(line, &pos, /* noThrow */ true))
					;
			}

			parser.check <CRLF>(line, &pos);

			*currentPos = pos;
		}

	private:

		IMAPParser::resp_cond_bye* m_resp_cond_bye;

	public:

		const IMAPParser::resp_cond_bye* resp_cond_bye() const { return (m_resp_cond_bye); }
	};


	//
	// response_tagged ::= tag SPACE resp_cond_state CRLF
	//

	class response_tagged : public component
	{
	public:

		response_tagged()
			: m_resp_cond_state(NULL)
		{
		}

		~response_tagged()
		{
			delete (m_resp_cond_state);
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("response_tagged");

			size_t pos = *currentPos;

			parser.check <IMAPParser::xtag>(line, &pos);
			parser.check <SPACE>(line, &pos);
			m_resp_cond_state = parser.get <IMAPParser::resp_cond_state>(line, &pos);

			if (!parser.isStrict())
			{
				// Allow SPACEs at end of line
				while (parser.check <SPACE>(line, &pos, /* noThrow */ true))
					;
			}

			parser.check <CRLF>(line, &pos);

			*currentPos = pos;
		}

	private:

		IMAPParser::resp_cond_state* m_resp_cond_state;

	public:

		const IMAPParser::resp_cond_state* resp_cond_state() const { return (m_resp_cond_state); }
	};


	//
	// response_done ::= response_tagged / response_fatal
	//

	class response_done : public component
	{
	public:

		response_done()
			: m_response_tagged(NULL), m_response_fatal(NULL)
		{
		}

		~response_done()
		{
			delete (m_response_tagged);
			delete (m_response_fatal);
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("response_done");

			size_t pos = *currentPos;

			if (!(m_response_tagged = parser.get <IMAPParser::response_tagged>(line, &pos, true)))
				m_response_fatal = parser.get <IMAPParser::response_fatal>(line, &pos);

			*currentPos = pos;
		}

	private:

		IMAPParser::response_tagged* m_response_tagged;
		IMAPParser::response_fatal* m_response_fatal;

	public:

		const IMAPParser::response_tagged* response_tagged() const { return (m_response_tagged); }
		const IMAPParser::response_fatal* response_fatal() const { return (m_response_fatal); }
	};


	//
	// response ::= *(continue_req / response_data) response_done
	//

	class response : public component
	{
	public:

		response()
			: m_response_done(NULL)
		{
		}

		~response()
		{
			for (std::vector <IMAPParser::continue_req_or_response_data*>::iterator
			     it = m_continue_req_or_response_data.begin() ;
			     it != m_continue_req_or_response_data.end() ; ++it)
			{
				delete (*it);
			}

			delete (m_response_done);
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("response");

			size_t pos = *currentPos;
			string curLine = line;
			bool partial = false;  // partial response

			IMAPParser::continue_req_or_response_data* resp = NULL;

			while ((resp = parser.get <IMAPParser::continue_req_or_response_data>(curLine, &pos, true)) != NULL)
			{
				m_continue_req_or_response_data.push_back(resp);

				// Partial response (continue_req)
				if (resp->continue_req())
				{
					partial = true;
					break;
				}

				// We have read a CRLF, read another line
				curLine = parser.readLine();
				pos = 0;
			}

			if (!partial)
				m_response_done = parser.get <IMAPParser::response_done>(curLine, &pos);

			*currentPos = pos;
		}


		bool isBad() const
		{
			if (!response_done())  // incomplete (partial) response
				return (true);

			if (response_done()->response_fatal())
				return (true);

			if (response_done()->response_tagged()->resp_cond_state()->
				status() == IMAPParser::resp_cond_state::BAD)
			{
				return (true);
			}

			return (false);
		}

		void setErrorLog(const string& errorLog)
		{
			m_errorLog = errorLog;
		}

		const string& getErrorLog() const
		{
			return m_errorLog;
		}

	private:

		std::vector <IMAPParser::continue_req_or_response_data*> m_continue_req_or_response_data;
		IMAPParser::response_done* m_response_done;

		string m_errorLog;

	public:

		const std::vector <IMAPParser::continue_req_or_response_data*>& continue_req_or_response_data() const { return (m_continue_req_or_response_data); }
		const IMAPParser::response_done* response_done() const { return (m_response_done); }
	};


	//
	// greeting ::= "*" SPACE (resp_cond_auth / resp_cond_bye) CRLF
	//

	class greeting : public component
	{
	public:

		greeting()
			: m_resp_cond_auth(NULL), m_resp_cond_bye(NULL)
		{
		}

		~greeting()
		{
			delete (m_resp_cond_auth);
			delete (m_resp_cond_bye);
		}

		void go(IMAPParser& parser, string& line, size_t* currentPos)
		{
			DEBUG_ENTER_COMPONENT("greeting");

			size_t pos = *currentPos;

			parser.check <one_char <'*'> >(line, &pos);
			parser.check <SPACE>(line, &pos);

			if (!(m_resp_cond_auth = parser.get <IMAPParser::resp_cond_auth>(line, &pos, true)))
				m_resp_cond_bye = parser.get <IMAPParser::resp_cond_bye>(line, &pos);

			parser.check <CRLF>(line, &pos);

			*currentPos = pos;
		}

		void setErrorLog(const string& errorLog)
		{
			m_errorLog = errorLog;
		}

		const string& getErrorLog() const
		{
			return m_errorLog;
		}

	private:

		IMAPParser::resp_cond_auth* m_resp_cond_auth;
		IMAPParser::resp_cond_bye* m_resp_cond_bye;

		string m_errorLog;

	public:

		const IMAPParser::resp_cond_auth* resp_cond_auth() const { return (m_resp_cond_auth); }
		const IMAPParser::resp_cond_bye* resp_cond_bye() const { return (m_resp_cond_bye); }
	};



	//
	// The main functions used to parse a response
	//

	response* readResponse(literalHandler* lh = NULL)
	{
		size_t pos = 0;
		string line = readLine();

		m_literalHandler = lh;
		response* resp = get <response>(line, &pos);
		m_literalHandler = NULL;

		resp->setErrorLog(lastLine());

		return (resp);
	}


	greeting* readGreeting()
	{
		size_t pos = 0;
		string line = readLine();

		greeting* greet = get <greeting>(line, &pos);

		greet->setErrorLog(lastLine());

		return greet;
	}


	//
	// Get a token and advance
	//

	template <class TYPE>
	TYPE* get(string& line, size_t* currentPos,
	          const bool noThrow = false)
	{
		component* resp = new TYPE;
		return internalGet <TYPE>(resp, line, currentPos, noThrow);
	}


	template <class TYPE, class ARG1_TYPE, class ARG2_TYPE>
	TYPE* getWithArgs(string& line, size_t* currentPos,
	                  ARG1_TYPE arg1, ARG2_TYPE arg2, const bool noThrow = false)
	{
		component* resp = new TYPE(arg1, arg2);
		return internalGet <TYPE>(resp, line, currentPos, noThrow);
	}


private:

	template <class TYPE>
	TYPE* internalGet(component* resp, string& line, size_t* currentPos,
	                  const bool noThrow = false)
	{
		const size_t oldPos = *currentPos;

		try
		{
			resp->go(*this, line, currentPos);
		}
		catch (exceptions::operation_timed_out&)
		{
			// Always rethrow
			throw;
		}
		catch (exception&)
		{
			*currentPos = oldPos;

			delete (resp);
			if (!noThrow) throw;
			return (NULL);
		}

		return static_cast <TYPE*>(resp);
	}

	const string lastLine() const
	{
		// Remove blanks and new lines at the end of the line.
		string line(m_lastLine);

		string::const_iterator it = line.end();
		int count = 0;

		while (it != line.begin())
		{
			const unsigned char c = *(it - 1);

			if (!(c == ' ' || c == '\t' || c == '\n' || c == '\r'))
				break;

			++count;
			--it;
		}

		line.resize(line.length() - count);

		return (line);
	}

public:

	//
	// Check a token and advance
	//

	template <class TYPE>
	bool check(string& line, size_t* currentPos,
	                 const bool noThrow = false)
	{
		const size_t oldPos = *currentPos;

		try
		{
			TYPE term;
			term.go(*this, line, currentPos);
		}
		catch (exceptions::operation_timed_out&)
		{
			// Always rethrow
			throw;
		}
		catch (exception&)
		{
			*currentPos = oldPos;

			if (!noThrow) throw;
			return false;
		}

		return true;
	}

	template <class TYPE, class ARG_TYPE>
	bool checkWithArg(string& line, size_t* currentPos,
	                        const ARG_TYPE arg, const bool noThrow = false)
	{
		const size_t oldPos = *currentPos;

		try
		{
			TYPE term(arg);
			term.go(*this, line, currentPos);
		}
		catch (exceptions::operation_timed_out&)
		{
			// Always rethrow
			throw;
		}
		catch (exception&)
		{
			*currentPos = oldPos;

			if (!noThrow) throw;
			return false;
		}

		return true;
	}


private:

	weak_ref <IMAPTag> m_tag;
	weak_ref <socket> m_socket;

	utility::progressListener* m_progress;

	bool m_strict;

	literalHandler* m_literalHandler;

	weak_ref <timeoutHandler> m_timeoutHandler;


	string m_buffer;

	string m_lastLine;

    // FIX by Elmue: Added instance counter for Trace output
    int m_instanceID;

public:

	//
	// Read one line
	//

	const string readLine()
	{
		size_t pos;

		while ((pos = m_buffer.find('\n')) == string::npos)
		{
			read();
		}

		string line;
		line.resize(pos + 1);
		std::copy(m_buffer.begin(), m_buffer.begin() + pos + 1, line.begin());

		m_buffer.erase(m_buffer.begin(), m_buffer.begin() + pos + 1);

		m_lastLine = line;

        // FIX by Elmue: Added Console independent Trace
        #if VMIME_TRACE
            int len = (int)line.size();
            while (len >= 0 && (line[len-1] == '\r' || line[len-1] == '\n'))
            {
                len --;
            }
            // cut trailing "\r\n"
            string trace;
            trace.assign(line.c_str(), 0, len);
            TRACE("IMAP (%d) read < \"%s\"", m_instanceID, trace.c_str());
        #endif

		return (line);
	}


	//
	// Read available data from socket stream
	//

	void read()
	{
		string receiveBuffer;

		ref <timeoutHandler> toh = m_timeoutHandler.acquire();
		ref <socket> sok = m_socket.acquire();

		if (toh)
			toh->resetTimeOut();

		while (receiveBuffer.empty())
		{
			// Check whether the time-out delay is elapsed
			if (toh && toh->isTimeOut())
			{
				if (!toh->handleTimeOut())
					throw exceptions::operation_timed_out();
			}

			// We have received data: reset the time-out counter
			sok->receive(receiveBuffer);

			if (receiveBuffer.empty())   // buffer is empty
			{
				platform::getHandler()->wait();
				continue;
			}

			// We have received data ...
			if (toh)
				toh->resetTimeOut();
		}

		m_buffer += receiveBuffer;
	}


	void readLiteral(literalHandler::target& buffer, size_t count)
	{
		size_t len = 0;
		string receiveBuffer;

		ref <timeoutHandler> toh = m_timeoutHandler.acquire();
		ref <socket> sok = m_socket.acquire();

		if (m_progress)
			m_progress->start(count);

		if (toh)
			toh->resetTimeOut();

		if (!m_buffer.empty())
		{
			if (m_buffer.length() > count)
			{
				buffer.putData(string(m_buffer.begin(), m_buffer.begin() + count));
				m_buffer.erase(m_buffer.begin(), m_buffer.begin() + count);
				len = count;
			}
			else
			{
				len += m_buffer.length();
				buffer.putData(m_buffer);
				m_buffer.clear();
			}
		}

		while (len < count)
		{
			// Check whether the time-out delay is elapsed
			if (toh && toh->isTimeOut())
			{
				if (!toh->handleTimeOut())
					throw exceptions::operation_timed_out();

				toh->resetTimeOut();
			}

			// Receive data from the socket
			sok->receive(receiveBuffer);

			if (receiveBuffer.empty())   // buffer is empty
			{
				platform::getHandler()->wait();
				continue;
			}

			// We have received data: reset the time-out counter
			if (toh)
				toh->resetTimeOut();

			if (len + receiveBuffer.length() > count)
			{
				const size_t remaining = count - len;

				// Get the needed amount of data
				buffer.putData(string(receiveBuffer.begin(), receiveBuffer.begin() + remaining));

				// Put the remaining data into the internal response buffer
				receiveBuffer.erase(receiveBuffer.begin(), receiveBuffer.begin() + remaining);
				m_buffer += receiveBuffer;

				len = count;
			}
			else
			{
				buffer.putData(receiveBuffer);
				len += receiveBuffer.length();
			}

			// Notify progress
			if (m_progress)
				m_progress->progress(len, count);
		}

		if (m_progress)
			m_progress->stop(count);
	}
};


} // imap
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP

#endif // VMIME_NET_IMAP_IMAPPARSER_HPP_INCLUDED
