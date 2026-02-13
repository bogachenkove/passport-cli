#pragma once

#include "../../interface/interface_database.hpp"
#include "../../interface/interface_crypto_service.hpp"
#include "../../interface/interface_terminal.hpp"
#include <memory>
#include <string>

namespace app {
	class Application {
	public:
		Application(
			std::shared_ptr<domain::interfaces::ITerminal> terminal,
			std::shared_ptr<domain::interfaces::ICryptoService> crypto,
			std::shared_ptr<domain::interfaces::IDatabase> database);
		int run();
	private:
		std::shared_ptr<domain::interfaces::ITerminal>      term_;
		std::shared_ptr<domain::interfaces::ICryptoService> crypto_;
		std::shared_ptr<domain::interfaces::IDatabase>      db_;
		std::string db_path_;
		std::string master_pw_;
		bool        database_ready_ = false;
		void run_startup_menu();
		void run_main_menu();
	};
}