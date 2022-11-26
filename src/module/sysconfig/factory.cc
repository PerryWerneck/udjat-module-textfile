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
 #include <udjat/tools/sysconfig.h>
 #include <udjat/tools/file.h>
 #include <internals.h>
 #include <pugixml.hpp>
 #include <udjat/moduleinfo.h>

 using namespace std;

 namespace Udjat {

	static const Udjat::ModuleInfo moduleinfo{"Sysconfig reader agent"};

	SysConfig::Factory::Factory() : Udjat::Factory("sysconfig",moduleinfo) {
	}

	SysConfig::Factory::~Factory() {
	}

	std::shared_ptr<Abstract::Agent> SysConfig::Factory::AgentFactory(const Abstract::Object &parent, const pugi::xml_node &node) const {

		/// @brief Agent state.
		class State : public Udjat::Abstract::State {
		public:
			State(const SysConfig::File &file) : Abstract::State("file",Level::unimportant,"") {
				//set(file.getPath());
			}

			State(const Udjat::File::Agent &agent) : Abstract::State("file",unimportant,"") {
				//set(agent.getPath());
			}

			virtual ~State() {
			}

		};

		/// @brief On-demand agent, load file when needed.
		class OnDemand : public Udjat::Abstract::Agent {
		private:
			Quark key;
			Quark filename;
			bool strict = false;

			void setup() {
				Object::properties.icon = "text-x-generic";
			}

		public:

			OnDemand(const char *filename, const char *key) : Abstract::Agent(key) {

				cout << "Filename: " << filename << " key: " << key << endl;

				setup();

				this->key = key;
				this->filename = filename;
				this->strict = false;
			}

			OnDemand(const pugi::xml_node &node) : Abstract::Agent(node) {

				setup();

				key.set(node,"key");
				filename.set(node,"filename",true);
				strict = Udjat::Attribute(node,"strict").as_bool(strict);
			}

			virtual ~OnDemand() {
			}

			Udjat::Value & get(Udjat::Value &value) const override {

				try {

					auto file = SysConfig::File(filename.c_str());

					/*
					Object::properties.label = file.getPath();
					Object::properties.summary = file.getDescription();

					if(!(hasStates() || hasChildren())) {

						// No state or child, set the default one.
						Abstract::Agent::activate(make_shared<State>(file));

					}
					*/

					if(!key) {

						// No key! Load them all.
						for(auto v : file) {
							v.get(value[v.name.c_str()]);
						}

					} else {

						auto v = file[key.c_str()];
						if(!v) {
							throw system_error(ENOENT,system_category(),string{"Can't find key '"} + key.c_str() + "'");
						}

						v.get(value);

					}

				} catch(const std::exception &e) {

					throw runtime_error(string{"Error '"} + e.what() + "' getting value");

				}
				return value;
			}

			std::shared_ptr<Abstract::Agent> find(const char *path, bool required, bool autoins) override {

				std::shared_ptr<Abstract::Agent> rc = Abstract::Agent::find(path,false,false);
				if(rc) {
					return rc;
				}

				if(strchr(path,'/')) {
					throw system_error(ENOENT,system_category(),"Can't search with '/' on sysconfig files");
				}

				// Create a value reader.
				if(!strict) {
					return make_shared<OnDemand>(this->filename.c_str(),path);
				}

				throw system_error(ENOENT,system_category(),string{"Can't find '"} + path + "' in '" + this->filename.c_str() + "'");
			}

		};

		/// @brief INotify agent, read from file when it changes.
		class Inotify : public Udjat::Abstract::Agent, public Udjat::File::Agent {
		private:
			Quark key;
			SysConfig::Value value;
			bool strict = false;

		protected:
			void set(const char *contents) override {

				try {

					auto file = SysConfig::File();

					file.set(contents);

					Object::properties.label = file.getPath();
					Object::properties.summary = file.getDescription();

					/*
					if(!(hasStates() || hasChildren())) {

						// No state or child, set the default one.
						Abstract::Agent::activate(make_shared<State>(*this));

					}
					*/

					if(key) {

						// Get key value.
						this->value = file[key.c_str()];
						if(!this->value) {
							throw system_error(ENOENT,system_category(),string{"Can't find key '"} + key.c_str() + "'");
						}

					}

				} catch(const std::exception &e) {

					failed("Error parsing ile file contents",e);

				}

			}

			bool refresh() override {
				throw system_error(ENOTSUP,system_category(),"Timed update is not available for this agent");
			}

		public:

			Inotify(const pugi::xml_node &node) : Udjat::Abstract::Agent(node), Udjat::File::Agent(Udjat::Attribute(node,"filename")) {

				Object::properties.icon = "text-x-generic";

				key.set(node,"key");
				strict = Udjat::Attribute(node,"strict").as_bool(strict);

			}

			virtual ~Inotify() {
			}

			Udjat::Value & get(Udjat::Value &value) const override {
				return this->value.get(value);
			}

			std::shared_ptr<Abstract::Agent> find(const char *path, bool required, bool autoins) override {

				std::shared_ptr<Abstract::Agent> rc = Abstract::Agent::find(path,false,false);
				if(rc) {
					return rc;
				}

				if(strchr(path,'/')) {
					throw system_error(ENOENT,system_category(),"Can't search with '/' on sysconfig files");
				}

				// Create a value reader.
				if(!strict) {
					return make_shared<OnDemand>(Udjat::File::Agent::c_str(),path);
				}

				throw system_error(ENOENT,system_category(),string{"Can't find '"} + path + "' in '" + Udjat::File::Agent::c_str() + "'");
			}

		};

		if(Udjat::Attribute(node,"update-on-demand").as_bool(true)) {

			// On-demand agent.
			return make_shared<OnDemand>(node);

		}

		// INotify agent.
		return make_shared<Inotify>(node);

	}

 }
