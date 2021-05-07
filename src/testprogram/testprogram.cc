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

 using namespace std;
 using namespace Udjat;

//---[ Implement ]------------------------------------------------------------------------------------------

int main(int argc, char **argv) {

	setlocale( LC_ALL, "" );

	auto module = udjat_module_init();
	Module::load();

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
		const char *text = {"\n\nTEST\n\n"};
		regex_t re;

		if(regcomp(&re,".*TEXT.*",REG_EXTENDED|REG_NEWLINE) != 0) {
			throw runtime_error("Can't compile expression");
		}

		regmatch_t rm[2];
		memset(&rm,0,sizeof(rm));
		int rc = regexec(&re, text, 2, rm, 0);
		if(rc == 0) {
			cout << "MATCH" << endl;
		} else if(rc == REG_NOMATCH) {
			cout << "NO MATCH" << endl;
		} else {
			cout << to_string(rc) << endl;
		}

		regfree(&re);

	}

	/*
	{
		auto root_agent = Abstract::Agent::set_root(make_shared<Abstract::Agent>("root","System","Application"));

		{
			pugi::xml_document doc;
			doc.load_file("test.xml");
			root_agent->load(doc);
		}

		cout << "http://localhost:8989/info/1.0/modules" << endl;
		cout << "http://localhost:8989/info/1.0/factories" << endl;
		cout << "http://localhost:8989/api/1.0/agent/backup" << endl;
		cout << "http://localhost:8989/api/1.0/agent/cpuinfo" << endl;
		cout << "http://localhost:8989/api/1.0/agent/cpuname" << endl;
		cout << "http://localhost:8989/api/1.0/agent/auth_basic" << endl;

		Udjat::run();

	}
	*/

	delete module;
	return 0;
}
