#include "record_formatter.hpp"
#include "../interface/interface_database.hpp"
#include "../interface/interface_terminal.hpp"
#include <cstddef>
#include <iomanip>
#include <sstream>
#include <string>

namespace {
	std::string format_field(const std::string& field) {
		return field.empty() ? "---" : field;
	}
}
namespace ui {
	void display_password_records(
		const domain::interfaces::IDatabase& db, std::shared_ptr<domain::interfaces::ITerminal> term) {
		const auto& records = db.password_records();
		if (records.empty()) {
			term->show_message("\n  No password records to display.");
			return;
		}
		constexpr std::size_t kDateColWidth = 16;
		std::size_t w_login = 5, w_pass = 8, w_url = 3, w_note = 4;
		for (const auto& r : records) {
			auto l = format_field(r.login);
			auto p = format_field(r.password);
			auto u = format_field(r.url);
			auto n = format_field(r.note);
			if (l.size() > w_login) w_login = l.size();
			if (p.size() > w_pass) w_pass = p.size();
			if (u.size() > w_url) w_url = u.size();
			if (n.size() > w_note) w_note = n.size();
		}
		std::ostringstream header;
		header << "\n  --- Password Records (* = required) ---\n\n";
		header << "  "
			<< std::left
			<< std::setw(5) << "#"
			<< std::setw(kDateColWidth + 3) << "Date"
			<< std::setw(w_login + 3) << "Login*"
			<< std::setw(w_pass + 3) << "Password*"
			<< std::setw(w_url + 3) << "URL"
			<< std::setw(w_note + 3) << "Note"
			<< '\n';
		std::size_t total_w = 5 + (kDateColWidth + 3) + (w_login + 3) + (w_pass + 3) + (w_url + 3) + (w_note + 3);
		header << "  " << std::string(total_w, '-');
		term->show_message(header.str());
		for (std::size_t i = 0; i < records.size(); ++i) {
			const auto& r = records[i];
			std::ostringstream row;
			row << "  "
				<< std::left
				<< std::setw(5) << (i + 1)
				<< std::setw(kDateColWidth + 3) << term->format_datetime(r.date)
				<< std::setw(w_login + 3) << format_field(r.login)
				<< std::setw(w_pass + 3) << format_field(r.password)
				<< std::setw(w_url + 3) << format_field(r.url)
				<< std::setw(w_note + 3) << format_field(r.note);
			term->show_message(row.str());
		}
		term->show_message("\n  Total password records: " + std::to_string(records.size()));
	}
	void display_note_records(
		const domain::interfaces::IDatabase& db, std::shared_ptr<domain::interfaces::ITerminal> term) {
		const auto& records = db.note_records();
		if (records.empty()) {
			term->show_message("\n  No note records to display.");
			return;
		}
		constexpr std::size_t kDateColWidth = 16;
		std::size_t w_title = 5, w_note = 4;
		for (const auto& r : records) {
			auto t = format_field(r.title);
			auto n = format_field(r.note);
			if (t.size() > w_title) w_title = t.size();
			if (n.size() > w_note) w_note = n.size();
		}
		std::ostringstream header;
		header << "\n  --- Note Records (* = required) ---\n\n";
		header << "  "
			<< std::left
			<< std::setw(5) << "#"
			<< std::setw(kDateColWidth + 3) << "Date"
			<< std::setw(w_title + 3) << "Title*"
			<< std::setw(w_note + 3) << "Note"
			<< '\n';
		std::size_t total_w = 5 + (kDateColWidth + 3) + (w_title + 3) + (w_note + 3);
		header << "  " << std::string(total_w, '-');
		term->show_message(header.str());
		for (std::size_t i = 0; i < records.size(); ++i) {
			const auto& r = records[i];
			std::ostringstream row;
			row << "  "
				<< std::left
				<< std::setw(5) << (i + 1)
				<< std::setw(kDateColWidth + 3) << term->format_datetime(r.date)
				<< std::setw(w_title + 3) << format_field(r.title)
				<< std::setw(w_note + 3) << format_field(r.note);
			term->show_message(row.str());
		}
		term->show_message("\n  Total note records: " + std::to_string(records.size()));
	}
	void display_bankcard_records(
		const domain::interfaces::IDatabase& db, std::shared_ptr<domain::interfaces::ITerminal> term) {
		const auto& records = db.bankcard_records();
		if (records.empty()) {
			term->show_message("\n  No bank card records to display.");
			return;
		}
		constexpr std::size_t kDateColWidth = 16;
		std::size_t w_num = 5, w_exp = 5, w_cvv = 3, w_holder = 10, w_note = 5;
		for (const auto& r : records) {
			auto num = format_field(r.card_number);
			auto exp = format_field(r.expiry_date);
			auto cvv = format_field(r.cvv);
			auto holder = format_field(r.cardholder_name);
			auto note = format_field(r.note);
			if (num.size() > w_num) w_num = num.size();
			if (exp.size() > w_exp) w_exp = exp.size();
			if (cvv.size() > w_cvv) w_cvv = cvv.size();
			if (holder.size() > w_holder) w_holder = holder.size();
			if (note.size() > w_note) w_note = note.size();
		}
		std::ostringstream header;
		header << "\n  --- Bank Card Records (* = required) ---\n\n";
		header << "  "
			<< std::left
			<< std::setw(5) << "#"
			<< std::setw(kDateColWidth + 3) << "Date"
			<< std::setw(w_num + 3) << "Card Number*"
			<< std::setw(w_exp + 3) << "Expiry*"
			<< std::setw(w_cvv + 3) << "CVV*"
			<< std::setw(w_holder + 3) << "Holder*"
			<< std::setw(w_note + 3) << "Note"
			<< '\n';
		std::size_t total_w = 5 + (kDateColWidth + 3) + (w_num + 3) + (w_exp + 3) + (w_cvv + 3) + (w_holder + 3) + (w_note + 3);
		header << "  " << std::string(total_w, '-');
		term->show_message(header.str());
		for (std::size_t i = 0;
			i < records.size();
			++i) {
			const auto& r = records[i];
			std::ostringstream row;
			row << "  "
				<< std::left
				<< std::setw(5) << (i + 1)
				<< std::setw(kDateColWidth + 3) << term->format_datetime(r.date)
				<< std::setw(w_num + 3) << format_field(r.card_number)
				<< std::setw(w_exp + 3) << format_field(r.expiry_date)
				<< std::setw(w_cvv + 3) << format_field(r.cvv)
				<< std::setw(w_holder + 3) << format_field(r.cardholder_name)
				<< std::setw(w_note + 3) << format_field(r.note);
			term->show_message(row.str());
		}
		term->show_message("\n  Total bank card records: " + std::to_string(records.size()));
	}
	void display_discountcard_records(
		const domain::interfaces::IDatabase& db, std::shared_ptr<domain::interfaces::ITerminal> term) {
		const auto& records = db.discountcard_records();
		if (records.empty()) {
			term->show_message("\n  No discount card records to display.");
			return;
		}
		constexpr std::size_t kDateColWidth = 16;
		std::size_t w_num = 5;
		std::size_t w_barcode = 6;
		std::size_t w_cvv = 3;
		std::size_t w_store = 10;
		std::size_t w_note = 5;
		for (const auto& r : records) {
			auto num = format_field(r.card_number);
			auto barcode = format_field(r.barcode);
			auto cvv = format_field(r.cvv);
			auto store = format_field(r.store_name);
			auto note = format_field(r.note);
			if (num.size() > w_num) w_num = num.size();
			if (barcode.size() > w_barcode) w_barcode = barcode.size();
			if (cvv.size() > w_cvv) w_cvv = cvv.size();
			if (store.size() > w_store) w_store = store.size();
			if (note.size() > w_note) w_note = note.size();
		}
		std::ostringstream header;
		header << "\n  --- Discount Card Records (* = required) ---\n\n";
		header << "  "
			<< std::left
			<< std::setw(5) << "#"
			<< std::setw(kDateColWidth + 3) << "Date"
			<< std::setw(w_num + 3) << "Card Number*"
			<< std::setw(w_barcode + 3) << "Barcode*"
			<< std::setw(w_cvv + 3) << "CVV"
			<< std::setw(w_store + 3) << "Store Name*"
			<< std::setw(w_note + 3) << "Note"
			<< '\n';
		std::size_t total_w = 5 + (kDateColWidth + 3) + (w_num + 3) + (w_barcode + 3) + (w_cvv + 3) + (w_store + 3) + (w_note + 3);
		header << "  " << std::string(total_w, '-');
		term->show_message(header.str());
		for (std::size_t i = 0; i < records.size(); ++i) {
			const auto& r = records[i];
			std::ostringstream row;
			row << "  "
				<< std::left
				<< std::setw(5) << (i + 1)
				<< std::setw(kDateColWidth + 3) << term->format_datetime(r.date)
				<< std::setw(w_num + 3) << format_field(r.card_number)
				<< std::setw(w_barcode + 3) << format_field(r.barcode)
				<< std::setw(w_cvv + 3) << format_field(r.cvv)
				<< std::setw(w_store + 3) << format_field(r.store_name)
				<< std::setw(w_note + 3) << format_field(r.note);
			term->show_message(row.str());
		}
		term->show_message("\n  Total discount card records: " + std::to_string(records.size()));
	}
	void display_transportcard_records(
		const domain::interfaces::IDatabase& db, std::shared_ptr<domain::interfaces::ITerminal> term) {
		const auto& records = db.transportcard_records();
		if (records.empty()) {
			term->show_message("\n  No transport card records to display.");
			return;
		}
		constexpr std::size_t kDateColWidth = 16;
		std::size_t w_num = 5;
		std::size_t w_barcode = 6;
		std::size_t w_expiry = 5;
		std::size_t w_cvv = 3;
		std::size_t w_holder = 10;
		std::size_t w_note = 5;
		for (const auto& r : records) {
			auto num = format_field(r.card_number);
			auto barcode = format_field(r.barcode);
			auto expiry = format_field(r.expiry);
			auto cvv = format_field(r.cvv);
			auto holder = format_field(r.holder);
			auto note = format_field(r.note);
			if (num.size() > w_num)     w_num = num.size();
			if (barcode.size() > w_barcode) w_barcode = barcode.size();
			if (expiry.size() > w_expiry)  w_expiry = expiry.size();
			if (cvv.size() > w_cvv)     w_cvv = cvv.size();
			if (holder.size() > w_holder)  w_holder = holder.size();
			if (note.size() > w_note)    w_note = note.size();
		}
		std::ostringstream header;
		header << "\n  --- Transport Card Records (* = required) ---\n\n";
		header << "  "
			<< std::left
			<< std::setw(5) << "#"
			<< std::setw(kDateColWidth + 3) << "Date"
			<< std::setw(w_num + 3) << "Card Number*"
			<< std::setw(w_barcode + 3) << "Barcode*"
			<< std::setw(w_expiry + 3) << "Expiry"
			<< std::setw(w_cvv + 3) << "CVV"
			<< std::setw(w_holder + 3) << "Holder"
			<< std::setw(w_note + 3) << "Note"
			<< '\n';
		std::size_t total_w = 5 + (kDateColWidth + 3) + (w_num + 3) + (w_barcode + 3) +
			(w_expiry + 3) + (w_cvv + 3) + (w_holder + 3) + (w_note + 3);
		header << "  " << std::string(total_w, '-');
		term->show_message(header.str());
		for (std::size_t i = 0;
			i < records.size();
			++i) {
			const auto& r = records[i];
			std::ostringstream row;
			row << "  "
				<< std::left
				<< std::setw(5) << (i + 1)
				<< std::setw(kDateColWidth + 3) << term->format_datetime(r.date)
				<< std::setw(w_num + 3) << format_field(r.card_number)
				<< std::setw(w_barcode + 3) << format_field(r.barcode)
				<< std::setw(w_expiry + 3) << format_field(r.expiry)
				<< std::setw(w_cvv + 3) << format_field(r.cvv)
				<< std::setw(w_holder + 3) << format_field(r.holder)
				<< std::setw(w_note + 3) << format_field(r.note);
			term->show_message(row.str());
		}
		term->show_message("\n  Total transport card records: " + std::to_string(records.size()));
	}
}