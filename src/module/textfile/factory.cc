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
 #include <udjat/agent.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/value.h>
 #include <internals.h>
 #include <pugixml.hpp>
 #include <string>

 using namespace std;

 namespace Udjat {

	/// @brief OnDemand regex parser
	template <typename T>
	class OnDemand : public Abstract::Agent, public TextFile::Regex {
	private:

		Quark filename;

	protected:

		Udjat::Value & get(Udjat::Value &value) const override {

			try {

				auto file = Udjat::File::Text(filename.c_str());
				T response;
				this->parse(file.c_str(),response);
				value = response;

			} catch(const std::exception &e) {

				throw runtime_error(string{"Error '"} + e.what() + "' acessing file");

			}

			return value;

		}

	public:
		OnDemand(const pugi::xml_node &node) : Abstract::Agent(node), TextFile::Regex(node) {
			filename.set(node,"filename");
			Abstract::Agent::load(node);
		}

		virtual ~OnDemand() {
		}

	};

	/// @brief Inotify regex parser.
	template <typename T>
	class Inotify : public Agent<T>, public File::Agent, public TextFile::Regex {

	protected:

		void set(const char *contents) override {

			try {

				// Override 'set' from File::Agent
				T value;
				this->parse(contents,value);
				Udjat::Agent<T>::set(value);

#ifdef DEBUG
				cout << "Updated value=" << value << endl;
#endif // DEBUG

			} catch(const std::exception &e) {

				Udjat::Agent<T>::failed("Can't parse file information",e);

			}

		}

	public:
		Inotify(const pugi::xml_node &node) : Udjat::Agent<T>(node), File::Agent(node,"filename"), TextFile::Regex(node) {
			Object::properties.icon = "text-x-generic";
			Udjat::Agent<T>::load(node);
		}

		virtual ~Inotify() {
		}

		bool refresh() override {
			File::Agent::update();
			return true;
		}

	};

	static const Udjat::ModuleInfo moduleinfo{
		PACKAGE_NAME,									// The module name.
		"Text file parser", 							// The module description.
		PACKAGE_VERSION, 								// The module version.
		PACKAGE_URL, 									// The package URL.
		PACKAGE_BUGREPORT 								// The bugreport address.
	};

	TextFile::Factory::Factory() : Udjat::Factory("textfile",&moduleinfo) {
	}

	TextFile::Factory::~Factory() {
	}

	bool TextFile::Factory::parse(Abstract::Agent &parent, const pugi::xml_node &node) const {

		auto expression = Attribute(node,"expression",false);

		if(!expression) {
			clog << node.attribute("name").as_string() << "\tExpression attribute is required" << endl;
			return false;
		}

		// Get agent type.
		string type = Attribute(node,"type",false).as_string("bool");

		if(Attribute(node,"update-on-demand").as_bool(true)) {

			// On Demand Agent
			if(!strcasecmp(type.c_str(),"bool")) {

				parent.insert(make_shared<OnDemand<bool>>(node));

			} else if(!strcasecmp(type.c_str(),"integer")) {

				parent.insert(make_shared<OnDemand<unsigned int>>(node));

			} else if(!strcasecmp(type.c_str(),"string")) {

				parent.insert(make_shared<OnDemand<string>>(node));

			} else {

				throw system_error(ENOTSUP,system_category(),string{"Can't create agent of type '"} + type + "'");

			}

		} else {

			// Inotify agent
			if(!strcasecmp(type.c_str(),"bool")) {

				parent.insert(make_shared<Inotify<bool>>(node));

			} else if(!strcasecmp(type.c_str(),"integer")) {

				parent.insert(make_shared<Inotify<unsigned int>>(node));

			} else if(!strcasecmp(type.c_str(),"string")) {

				parent.insert(make_shared<Inotify<string>>(node));

			} else {

				throw system_error(ENOTSUP,system_category(),string{"Can't create agent of type '"} + type + "'");

			}

		}

		return true;

	}

 }
