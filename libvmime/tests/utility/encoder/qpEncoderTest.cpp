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

#include "tests/testUtils.hpp"

#include "encoderTestUtils.hpp"


VMIME_TEST_SUITE_BEGIN(qpEncoderTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testQuotedPrintable)
		VMIME_TEST(testQuotedPrintable_SoftLineBreaks)
		VMIME_TEST(testQuotedPrintable_CRLF)
		VMIME_TEST(testQuotedPrintable_RFC2047)
	VMIME_TEST_LIST_END


	void testQuotedPrintable()
	{
		static const vmime::string testSuites[] =
		{
			// Test 1
			"",

			"",

			// Test 2
			"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",

			"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",

			// Test 3
			"0123456789012345678901234567890123456789012345678901234567890123456789012"
			"3456789012345678901234567890123456789012345678901234567890123456789012345"
			"6789",

			"0123456789012345678901234567890123456789012345678901234567890123456789012=\r\n"
			"3456789012345678901234567890123456789012345678901234567890123456789012345=\r\n"
			"6789",

			// Test 4
			vmime::string(
			"\x89\xe8\x24\x04\x2f\xe8\xff\xfb\xeb\xff\x90\xd7\x74\x8d\x00\x26\x89\x55"
			"\x83\xe5\x08\xec\x04\xc7\xa0\x24\x05\xa2\xe8\x08\x43\xee\x00\x00\xc0\x85"
			"\x0a\x74\xec\x89\xc3\x5d\xb6\x8d\x00\x00\x00\x00\x04\xc7\xa8\x24\x05\xa2"
			"\xe8\x08\x43\xd4\x00\x00\xe8\xeb\xf6\x89\x89\x55\x81\xe5\xa8\xec\x00\x00"
			"\x89\x00\xfc\x75\x75\x8b\x89\x08\xf8\x5d\xb9\xe8\xff\xff\x83\xff\x14\xfe"
			"\x47\x74\xc0\x31\x9d\x8d\xff\x68\xff\xff\x85\x89\xff\x68\xff\xff\x85\x8d"
			"\xff\x6c\xff\xff\x04\x89\xe8\x24\xfa\x50\xff\xff\x45\xc7\x00\xec\x00\x00"
			"\x31\x00\x89\xc0\x24\x44\x89\x08\x24\x5c\x89\x04\x24\x34\x87\xe8\xff\xf6"
			"\x89\xff\x24\x34\x2f\xe8\xff\xf9\x8b\xff\xf8\x5d\x75\x8b\x89\xfc\x5d\xec"
			"\xbe\xc3\x00\x13\x00\x00\xe7\xeb\xb6\x8d\x00\x00\x00\x00\x89\x55\x57\xe5"
			"\x53\x56\xec\x81\x01\xdc\x00\x00\x45\xbb\x05\x5c\x8b\x08\x0c\x55\xe4\x83"
			"\x8b\xf0\x89\x02\x24\x5c\xc7\x04\x24\x04\x00\x06\x00\x00\xec\xa3\x05\xa9"
			"\xe8\x08\xf7\x2a\xff\xff\x04\xc7\x46\x24\x05\x5c\xb9\x08\x5c\x50\x08\x05"
			"\x4c\x89\x04\x24\xf5\xe8\xff\xf7\xc7\xff\x24\x04\x5c\x46\x08\x05\xe9\xe8"
			"\xff\xf8\xc7\xff\x24\x04\x1d\x70\x08\x05\x55\xe8\x00\xbb\xb8\x00\x00\x01"
			"\x00\x00\xd2\x31\x08\xa3\x05\xa7\xb8\x08\x00\x01\x00\x00\x0c\xa3\x05\xa7",
			18 * 16),

			"=89=E8$=04/=E8=FF=FB=EB=FF=90=D7t=8D=00&=89U=83=E5=08=EC=04=C7=A0$=05=A2=E8=\r\n"
			"=08C=EE=00=00=C0=85=0At=EC=89=C3]=B6=8D=00=00=00=00=04=C7=A8$=05=A2=E8=08=\r\n"
			"C=D4=00=00=E8=EB=F6=89=89U=81=E5=A8=EC=00=00=89=00=FCuu=8B=89=08=F8]=B9=E8=\r\n"
			"=FF=FF=83=FF=14=FEGt=C01=9D=8D=FFh=FF=FF=85=89=FFh=FF=FF=85=8D=FFl=FF=FF=04=\r\n"
			"=89=E8$=FAP=FF=FFE=C7=00=EC=00=001=00=89=C0$D=89=08$\\=89=04$4=87=E8=FF=F6=\r\n"
			"=89=FF$4/=E8=FF=F9=8B=FF=F8]u=8B=89=FC]=EC=BE=C3=00=13=00=00=E7=EB=B6=8D=00=\r\n"
			"=00=00=00=89UW=E5SV=EC=81=01=DC=00=00E=BB=05\\=8B=08=0CU=E4=83=8B=F0=89=02=\r\n"
			"$\\=C7=04$=04=00=06=00=00=EC=A3=05=A9=E8=08=F7*=FF=FF=04=C7F$=05\\=B9=08\\P=08=\r\n"
			"=05L=89=04$=F5=E8=FF=F7=C7=FF$=04\\F=08=05=E9=E8=FF=F8=C7=FF$=04=1Dp=08=05=\r\n"
			"U=E8=00=BB=B8=00=00=01=00=00=D21=08=A3=05=A7=B8=08=00=01=00=00=0C=A3=05=A7=\r\n"
		};


		for (unsigned int i = 0 ; i < sizeof(testSuites) / sizeof(testSuites[0]) / 2 ; ++i)
		{
			const vmime::string decoded = testSuites[i * 2];
			const vmime::string encoded = testSuites[i * 2 + 1];

			std::ostringstream oss;
			oss << "[QP] Test " << (i + 1) << ": ";

			// Encoding
			VASSERT_EQ(oss.str() + "encoding", encoded, encode("quoted-printable", decoded, 74));

			// Decoding
			VASSERT_EQ(oss.str() + "decoding", decoded, decode("quoted-printable", encoded, 74));

			// Multiple and successive encoding/decoding
			VASSERT_EQ(oss.str() + "multiple1", decoded,
				decode("quoted-printable",
					encode("quoted-printable", decoded)));

			VASSERT_EQ(oss.str() + "multiple2", decoded,
				decode("quoted-printable",
					decode("quoted-printable",
						encode("quoted-printable",
							encode("quoted-printable", decoded)))));

			VASSERT_EQ(oss.str() + "multiple3", decoded,
				decode("quoted-printable",
					decode("quoted-printable",
						decode("quoted-printable",
							encode("quoted-printable",
								encode("quoted-printable",
									encode("quoted-printable", decoded)))))));

			VASSERT_EQ(oss.str() + "multiple4", decoded,
				decode("quoted-printable",
					decode("quoted-printable",
						decode("quoted-printable",
							decode("quoted-printable",
								encode("quoted-printable",
									encode("quoted-printable",
										encode("quoted-printable",
											encode("quoted-printable", decoded)))))))));

			VASSERT(oss.str() + "encoded size",
				getEncoder("quoted-printable")->getEncodedSize(decoded.length())
				>= encode("quoted-printable", decoded).length());

			VASSERT(oss.str() + "decoded size",
				getEncoder("quoted-printable")->getDecodedSize(encoded.length())
				>= decode("quoted-printable", encoded).length());
		}
	}

	/** Tests Soft Line Breaks (RFC-2047/6.7(5). */
	void testQuotedPrintable_SoftLineBreaks()
	{
		VASSERT_EQ("1", "Now's the time=\r\n"
		                " for all folk =\r\n"
		                "to come to the=\r\n"
		                " aid of their =\r\n"
		                "country.",
		                encode("quoted-printable", "Now's the time for all folk "
		                                           "to come to the aid of their country.", 15));
	}

	/** In text mode, ensure line breaks in QP-encoded text are represented
	  * by a CRLF sequence, as per RFC-2047/6.7(4). */
	void testQuotedPrintable_CRLF()
	{
		vmime::propertySet encProps;

		// in "text" mode
		encProps["text"] = true;
		VASSERT_EQ("text", "line1\r\nline2",
		           encode("quoted-printable", "line1\r\nline2", 80, encProps));

		// in "binary" mode
		encProps["text"] = false;
		VASSERT_EQ("binary", "line1=0D=0Aline2",
		           encode("quoted-printable", "line1\r\nline2", 80, encProps));
	}

	void testQuotedPrintable_RFC2047()
	{
		/*
		 * The RFC (http://tools.ietf.org/html/rfc2047#section-5) says:
		 *
		 *    In this case the set of characters that may be used in a "Q"-encoded
		 *    'encoded-word' is restricted to: <upper and lower case ASCII
		 *    letters, decimal digits, "!", "*", "+", "-", "/", "=", and "_"
		 *    (underscore, ASCII 95.)>.  An 'encoded-word' that appears within a
		 *    'phrase' MUST be separated from any adjacent 'word', 'text' or
		 *    'special' by 'linear-white-space'.
		 */

		vmime::propertySet encProps;
		encProps["rfc2047"] = true;

		// Ensure 'especials' are encoded
		VASSERT_EQ("especials.1",  "=2C", encode("quoted-printable", ",", 10, encProps));
		VASSERT_EQ("especials.2",  "=3B", encode("quoted-printable", ";", 10, encProps));
		VASSERT_EQ("especials.3",  "=3A", encode("quoted-printable", ":", 10, encProps));
		VASSERT_EQ("especials.4",  "=5F", encode("quoted-printable", "_", 10, encProps));
		VASSERT_EQ("especials.5",  "=40", encode("quoted-printable", "@", 10, encProps));
		VASSERT_EQ("especials.6",  "=28", encode("quoted-printable", "(", 10, encProps));
		VASSERT_EQ("especials.7",  "=29", encode("quoted-printable", ")", 10, encProps));
		VASSERT_EQ("especials.8",  "=3C", encode("quoted-printable", "<", 10, encProps));
		VASSERT_EQ("especials.9",  "=3E", encode("quoted-printable", ">", 10, encProps));
		VASSERT_EQ("especials.10", "=5B", encode("quoted-printable", "[", 10, encProps));
		VASSERT_EQ("especials.11", "=5D", encode("quoted-printable", "]", 10, encProps));
		VASSERT_EQ("especials.12", "=22", encode("quoted-printable", "\"", 10, encProps));
	}

	// TODO: UUEncode

VMIME_TEST_SUITE_END
