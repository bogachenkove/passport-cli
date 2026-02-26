#pragma once
#include "../interface/interface_database.hpp"
#include "../interface/interface_terminal.hpp"
#include <memory>

namespace ui {
	void display_password_records(const domain::interfaces::IDatabase& db, std::shared_ptr<domain::interfaces::ITerminal> term);
	void display_note_records(const domain::interfaces::IDatabase& db, std::shared_ptr<domain::interfaces::ITerminal> term);
	void display_bankcard_records(const domain::interfaces::IDatabase& db, std::shared_ptr<domain::interfaces::ITerminal> term);
	void display_discountcard_records(const domain::interfaces::IDatabase& db, std::shared_ptr<domain::interfaces::ITerminal> term);
	void display_transportcard_records(const domain::interfaces::IDatabase& db, std::shared_ptr<domain::interfaces::ITerminal> term);
	void display_mnemonic_records(const domain::interfaces::IDatabase& db, std::shared_ptr<domain::interfaces::ITerminal> term);
	void display_wifi_records(const domain::interfaces::IDatabase& db, std::shared_ptr<domain::interfaces::ITerminal> term);
}