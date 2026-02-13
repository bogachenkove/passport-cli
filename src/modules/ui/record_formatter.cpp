#include "record_formatter.hpp"
#include <iomanip>
#include <iostream>

namespace ui {

namespace {
	std::string format_field(const std::string& field) {
		return field.empty() ? "---" : field;
	}
} // namespace

void display_password_records(
	const domain::interfaces::IDatabase& db,
	std::shared_ptr<domain::interfaces::ITerminal> term)
{
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
		if (p.size() > w_pass)   w_pass = p.size();
		if (u.size() > w_url)    w_url = u.size();
		if (n.size() > w_note)   w_note = n.size();
	}

	std::cout << "\n  --- Password Records (* = required) ---\n\n";
	std::cout << "  "
			  << std::left
			  << std::setw(5) << "#"
			  << std::setw(kDateColWidth + 3) << "Date"
			  << std::setw(w_login + 3) << "Login*"
			  << std::setw(w_pass + 3)  << "Password*"
			  << std::setw(w_url + 3)   << "URL"
			  << std::setw(w_note + 3)  << "Note"
			  << '\n';
	std::size_t total_w = 5 + (kDateColWidth + 3) + (w_login + 3) +
						  (w_pass + 3) + (w_url + 3) + (w_note + 3);
	std::cout << "  " << std::string(total_w, '-') << '\n';

	for (std::size_t i = 0; i < records.size(); ++i) {
		const auto& r = records[i];
		std::cout << "  "
				  << std::left
				  << std::setw(5) << (i + 1)
				  << std::setw(kDateColWidth + 3) << term->format_datetime(r.date)
				  << std::setw(w_login + 3) << format_field(r.login)
				  << std::setw(w_pass + 3)  << format_field(r.password)
				  << std::setw(w_url + 3)   << format_field(r.url)
				  << std::setw(w_note + 3)  << format_field(r.note)
				  << '\n';
	}
	std::cout << "\n  Total password records: " << records.size() << '\n';
}

void display_note_records(
	const domain::interfaces::IDatabase& db,
	std::shared_ptr<domain::interfaces::ITerminal> term)
{
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
		if (n.size() > w_note)  w_note  = n.size();
	}

	std::cout << "\n  --- Note Records (* = required) ---\n\n";
	std::cout << "  "
			  << std::left
			  << std::setw(5) << "#"
			  << std::setw(kDateColWidth + 3) << "Date"
			  << std::setw(w_title + 3) << "Title*"
			  << std::setw(w_note + 3)  << "Note"
			  << '\n';
	std::size_t total_w = 5 + (kDateColWidth + 3) + (w_title + 3) + (w_note + 3);
	std::cout << "  " << std::string(total_w, '-') << '\n';

	for (std::size_t i = 0; i < records.size(); ++i) {
		const auto& r = records[i];
		std::cout << "  "
				  << std::left
				  << std::setw(5) << (i + 1)
				  << std::setw(kDateColWidth + 3) << term->format_datetime(r.date)
				  << std::setw(w_title + 3) << format_field(r.title)
				  << std::setw(w_note + 3)  << format_field(r.note)
				  << '\n';
	}
	std::cout << "\n  Total note records: " << records.size() << '\n';
}

} // namespace ui