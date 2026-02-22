#pragma once
#include "../../interface/interface_database.hpp"
#include "../../interface/interface_terminal.hpp"
#include "../../models/password_record.hpp"
#include "../../models/note_record.hpp"
#include "../../models/bank_card_record.hpp"
#include "../../models/discount_card_record.hpp"
#include "../../models/transport_card_record.hpp"
#include "../../models/mnemonic_phrase_record.hpp"
#include <memory>

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
		domain::models::BankCardRecord prompt_bankcard_record();
		domain::models::DiscountCardRecord prompt_discountcard_record();
		domain::models::TransportCardRecord prompt_transportcard_record();
		domain::models::MnemonicPhraseRecord prompt_mnemonicphrase_record();
	};
}