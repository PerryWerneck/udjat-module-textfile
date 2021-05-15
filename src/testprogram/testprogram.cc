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

 #include <udjat.h>
 #include <udjat/module.h>
 #include <udjat/tools/logger.h>
 #include <udjat/files/sysconfig.h>
 #include <regex.h>
 #include <unistd.h>

 using namespace std;
 using namespace Udjat;

//---[ Implement ]------------------------------------------------------------------------------------------

int main(int argc, char **argv) {

	setlocale( LC_ALL, "" );

	auto module = udjat_module_init();
	//Module::load();

	/*
	{
		SysConfig::File file("/etc/sysconfig/apache2");
		cout << "----[" << file["APACHE_ACCESS_LOG"].value << "]----" << endl;
	}
	*/

	/*
	{
		SysConfig::File file("/proc/cpuinfo",":");

		file.forEach([](const SysConfig::Value &value){
			cout << "[" << value.name << "] = '" << value.value << "'" << endl;
		});

	}
	*/

	{
		{
			pugi::xml_document doc;
			doc.load_file("test.xml");

			auto root = Abstract::Agent::set_root(make_shared<Abstract::Agent>("root","System","Application"));
			root->load(doc);

			cout << "http://localhost:8989/info/1.0/modules" << endl;
			cout << "http://localhost:8989/info/1.0/factories" << endl;

			for(auto agent : *root) {
				cout << "http://localhost:8989/api/1.0/agent/" << agent->getName() << endl;
			}

		}


		Udjat::run();

		// Force cleanup
		// Abstract::Agent::set_root(std::shared_ptr<Abstract::Agent>());


	}

	cout << "Removing module" << endl;
	delete module;
	return 0;
}
