#pragma once
#include "../../interface/interface_terminal.hpp"
#include "../../interface/interface_database.hpp"
#include <memory>

namespace app::commands {
	class RemoveRecordCommand {
	public:
		RemoveRecordCommand(
			std::shared_ptr<domain::interfaces::ITerminal> terminal,
			std::shared_ptr<domain::interfaces::IDatabase> database);
		void execute();
	private:
		std::shared_ptr<domain::interfaces::ITerminal> term_;
		std::shared_ptr<domain::interfaces::IDatabase> db_;
		void remove_password_records();
		void remove_note_records();
		void remove_bankcard_records();
		void remove_discountcard_records();
	};
}