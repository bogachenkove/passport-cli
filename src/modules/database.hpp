#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "types.hpp"

struct PasswordRecord {
	uint64_t date = 0;
	std::string login;
	std::string password;
	std::string url;
	std::string note;
};
struct NoteRecord {
	uint64_t date = 0;
	std::string title;
	std::string note;
};
class PassportDatabase {
public:
	PassportDatabase() = default;
	bool load_from_file(const std::string& file_path, const std::string& master_password);
	bool save_to_file(const std::string& file_path, const std::string& master_password);
	void add_password_record(PasswordRecord record);
	bool remove_password_record(std::size_t index);
	[[nodiscard]] const std::vector<PasswordRecord>& password_records() const noexcept;
	[[nodiscard]] std::size_t password_record_count() const noexcept;
	void add_note_record(NoteRecord record);
	bool remove_note_record(std::size_t index);
	[[nodiscard]] const std::vector<NoteRecord>& note_records() const noexcept;
	[[nodiscard]] std::size_t note_record_count() const noexcept;
	[[nodiscard]] std::size_t record_count() const noexcept;
	[[nodiscard]] uint64_t timestamp_created()  const noexcept;
	[[nodiscard]] uint64_t timestamp_modified() const noexcept;
	[[nodiscard]] const types::TypeSystem& type_system() const noexcept;
private:
	[[nodiscard]]
	std::vector<uint8_t> serialize_records() const;
	void deserialize_records(const std::vector<uint8_t>& plaintext);
	static constexpr uint8_t kFileMagic[4] = { 'P', 'A', 'S', 'S' };
	static constexpr std::size_t kHeaderAdSize = 64;
	std::vector<PasswordRecord> password_records_;
	std::vector<NoteRecord> note_records_;
	types::TypeSystem type_system_;
	mutable uint64_t ts_created_ = 0;
	mutable uint64_t ts_modified_ = 0;
};