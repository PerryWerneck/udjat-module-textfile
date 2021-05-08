/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2021 Perry Werneck <perry.werneck@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

 #include <config.h>
 #include <internals.h>
 #include <regex.h>

 using namespace std;

 namespace Udjat {

	TextFile::Regex::Regex(const pugi::xml_node &node) : expression(node.attribute("expression")) {

		// Test if the expression can be compiled.
		regex_t re;
		int rc = regcomp(&re,expression.c_str(),REG_EXTENDED|REG_NEWLINE);
		if(rc != 0) {
			char msgbuf[100];
			regerror(rc, &re, msgbuf, sizeof(msgbuf));
			throw runtime_error(string{"Cant compile expression '"} + expression.c_str() + "': " + string(msgbuf));
		}
		regfree(&re);

	}

	TextFile::Regex::~Regex() {
	}

	void TextFile::Regex::parse(const char *contents, bool &response) {

		regex_t re;

		// Just in case
		if(regcomp(&re,expression.c_str(),REG_EXTENDED|REG_NEWLINE) != 0) {
			throw runtime_error(string{"Cant compile expression '"} + expression.c_str() + "'");
		}

		int rc = regexec(&re, contents, 0, NULL, 0);
		if(rc == 0) {
			response = true;
		} else if(rc == REG_NOMATCH) {
			response = false;
		} else {
			regfree(&re);
			throw runtime_error(string{"Unexpected return code '"} + to_string(rc) + "' on regexec()");
		}

		regfree(&re);

	}

	void TextFile::Regex::parse(const char *contents, std::string &response) {

		regex_t re;

		// Just in case
		if(regcomp(&re,expression.c_str(),REG_EXTENDED|REG_NEWLINE) != 0) {
			throw runtime_error(string{"Cant compile expression '"} + expression.c_str() + "'");
		}

		try {

			regmatch_t rm;
			memset(&rm,0,sizeof(rm));
			int rc = regexec(&re, contents, 1, &rm, 0);

			if(rc == 0) {

				if(rm.rm_so < 0) {
					throw runtime_error("Unexpected start point in regex response");
				}

				response = string( (contents + rm.rm_so), (rm.rm_eo-rm.rm_so) );

			} else if(rc == REG_NOMATCH) {

				response.clear();

			} else {

				throw runtime_error(string{"Unexpected return code '"} + to_string(rc) + "' on regexec()");

			}

		} catch(...) {
			regfree(&re);
			throw;
		}

		regfree(&re);

	}

	void TextFile::Regex::parse(const char *contents, unsigned int &response) {

		// Reference: https://www.lemoda.net/c/unix-regex/

		regex_t re;

		// Just in case
		if(regcomp(&re,expression.c_str(),REG_EXTENDED|REG_NEWLINE) != 0) {
			throw runtime_error(string{"Cant compile expression '"} + expression.c_str() + "'");
		}

		try {

			response = 0;

			regmatch_t rm;
			const char *ptr = contents;

			memset(&rm,0,sizeof(rm));
			int rc = regexec(&re, ptr, 1, &rm, 0);

			while( rc == 0 ) {

				response++;

				ptr += rm.rm_eo;
				memset(&rm,0,sizeof(rm));

				rc = regexec(&re, ptr, 1, &rm, 0);
			}

			if(rc != REG_NOMATCH) {
				throw runtime_error(string{"Unexpected return code "} + to_string(rc));
			}

		} catch(...) {
			regfree(&re);
			throw;
		}

		regfree(&re);
	}

 }

