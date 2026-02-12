#pragma once

#include "database.hpp"
#include "validation/validation.hpp"

#include <cstddef>
#include <string>
#include <variant>

namespace terminal {
	enum class RecordKind { Password, Note, Cancel };
	using AnyRecord = std::variant<PasswordRecord, NoteRecord>;

	void clear_screen();
	void show_welcome_banner();
	void show_message(const std::string& msg);
	void show_error(const std::string& msg);
	void show_success(const std::string& msg);
	void display_password_records(const PassportDatabase& db);
	void display_note_records(const PassportDatabase& db);

	[[nodiscard]] std::string prompt_for_input(const std::string& prompt);
	[[nodiscard]] std::string prompt_for_password(const std::string& prompt);
	void wait_for_enter();

	[[nodiscard]] int show_startup_menu();
	[[nodiscard]] int show_main_menu();

	[[nodiscard]] RecordKind prompt_record_kind_for_add();
	[[nodiscard]] RecordKind prompt_record_kind_for_list();
	[[nodiscard]] RecordKind prompt_record_kind_for_remove();

	[[nodiscard]] PasswordRecord prompt_for_new_password_record();
	[[nodiscard]] NoteRecord prompt_for_new_note_record();
}