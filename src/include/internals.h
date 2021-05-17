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

#pragma once

#include <udjat/factory.h>
#include <udjat/tools/sysconfig.h>

namespace Udjat {

	namespace SysConfig {

		class Factory : public Udjat::Factory {
		public:
			Factory();
			virtual ~Factory();

			void parse(Abstract::Agent &parent, const pugi::xml_node &node) const override;

		};

	}

	namespace TextFile {

		/// @brief Regex value parser
		class Regex {
		private:
			Quark expression;

		protected:

			/// @brief Parse expression, set response to 'true' if found.
			void parse(const char *contents, bool &response);

			/// @brief Parse expression, set response to 'count' of results.
			void parse(const char *contents, unsigned int &response);

			/// @brief Parse expression, extract string.
			void parse(const char *contents, std::string &response);

		public:
			Regex(const pugi::xml_node &node);
			~Regex();
		};

		class Factory : public Udjat::Factory {
		private:

		public:
			Factory();
			virtual ~Factory();

			void parse(Abstract::Agent &parent, const pugi::xml_node &node) const override;

		};

	}

}

