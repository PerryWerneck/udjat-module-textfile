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

 using namespace std;

 namespace Udjat {

	static const Udjat::ModuleInfo moduleinfo{
		PACKAGE_NAME,									// The module name.
		"Sysconfig reader agent", 						// The module description.
		PACKAGE_VERSION, 								// The module version.
		PACKAGE_URL, 									// The package URL.
		PACKAGE_BUGREPORT 								// The bugreport address.
	};

	SysConfig::Factory::Factory() : Udjat::Factory(Quark::getFromStatic("sysconfig"),&moduleinfo) {
	}

	SysConfig::Factory::~Factory() {
	}

	void SysConfig::Factory::parse(Abstract::Agent &parent, const pugi::xml_node &node) const {

		/// @brief Agent state.
		class State : public Udjat::Abstract::State {
		public:
			State(const SysConfig::File &file) : Abstract::State(unimportant,"") {
			}

			State(const Udjat::File::Agent &agent) : Abstract::State(unimportant,"") {
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
				this->icon = "text-x-generic";
			}

		public:

			OnDemand(const char *filename, const char *key) : Abstract::Agent(key) {

				cout << "Filename: " << filename << " key: " << key << endl;

				setup();

				this->key = key;
				this->filename = filename;
				this->strict = false;
			}

			OnDemand(const pugi::xml_node &node) {

				setup();

				key.set(node,"key");
				filename.set(node,"filename",true);
				strict = Udjat::Attribute(node,"strict").as_bool(strict);
				Udjat::Abstract::Agent::load(node);
			}

			virtual ~OnDemand() {
			}

			void get(const char *name, Json::Value &value) override {

				auto file = SysConfig::File(filename.c_str());

				this->label = file.getPath();
				this->summary = file.getDescription();

				if(!(hasStates() || hasChildren())) {
					Abstract::Agent::activate(make_shared<State>(file));
				}

				if(!key) {

					// No key! Load them all.
					Json::Value report(Json::objectValue);
					for(auto v : file) {
						v.get(v.name.c_str(),report);
					}

					value[name] = report;

				} else {

					try {

						auto v = file[key.c_str()];
						if(!v) {
							throw system_error(ENOENT,system_category(),string{"Can't find key '"} + key.c_str() + "'");
						}

						v.get(name,value);

					} catch(const std::exception &e) {

						error("'{}': {}",filename.c_str(),e.what());
						throw;

					}

				}

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
		class Inotify : public Udjat::Abstract::Agent, Udjat::File::Agent {
		private:
			Quark key;
			SysConfig::Value value;
			bool strict = false;

		protected:
				/*
			void set(const char *contents) override {
				if(!(hasStates() || hasChildren())) {
					#error parei aqui.
					Abstract::Agent::activate(make_shared<State>(file));
				}
			}
				*/

			void refresh() override {
				throw runtime_error("INotify.Sysconfig is not implemented");
			}

		public:

			Inotify(const pugi::xml_node &node) : Udjat::Abstract::Agent(), Udjat::File::Agent(Udjat::Attribute(node,"filename")) {

				this->icon = "text-x-generic";

				key.set(node,"key");
				strict = Udjat::Attribute(node,"strict").as_bool(strict);
				Udjat::Abstract::Agent::load(node);

			}

			virtual ~Inotify() {
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
			parent.insert(make_shared<OnDemand>(node));

		} else {

			// INotify agent.
			parent.insert(make_shared<Inotify>(node));

		}

	}




 }
