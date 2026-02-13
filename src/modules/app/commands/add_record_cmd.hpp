#pragma once

#include "../../interface/interface_database.hpp"
#include "../../interface/interface_terminal.hpp"
#include <memory>
#include "../../models/password_record.hpp"
#include "../../models/note_record.hpp"

namespace app::commands {

class AddRecordCommand {
public:
	AddRecordCommand(
		std::shared_ptr<domain::interfaces::ITerminal> terminal,
		std::shared_ptr<domain::interfaces::IDatabase> database);

	void execute();

private:
	std::shared_ptr<domain::interfaces::ITerminal> term_;
	std::shared_ptr<domain::interfaces::IDatabase> db_;

	domain::models::PasswordRecord prompt_password_record();
	domain::models::NoteRecord prompt_note_record();
};

} // namespace app::commands