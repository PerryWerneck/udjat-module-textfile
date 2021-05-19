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
 #include <udjat/module.h>
 #include <internals.h>

 using namespace Udjat;
 using namespace std;

 static const Udjat::ModuleInfo moduleinfo{
	PACKAGE_NAME,									// The module name.
	"Text file parser", 							// The module description.
	PACKAGE_VERSION, 								// The module version.
	PACKAGE_URL, 									// The package URL.
	PACKAGE_BUGREPORT 								// The bugreport address.
 };

 class Module : public Udjat::Module, Udjat::SysConfig::Factory, TextFile::Factory {
 private:

 public:

 	Module() : Udjat::Module("filereader",&moduleinfo) {
 	};

 	virtual ~Module() {
 	}


 };

 /// @brief Register udjat module.
 Udjat::Module * udjat_module_init() {
	return new ::Module();
 }

 bool udjat_module_deinit() {
	return false;
 }
