#include "modules/app/commands/application.hpp"
#include "modules/ui/console_terminal.hpp"
#include "modules/crypto/crypto_service.hpp"
#include "modules/storage/file_database.hpp"
#include "modules/core/metadata.hpp"
#include <memory>

int main(int argc, char* argv[]) {
	if (handle_metadata_flags(argc, argv)) {
		return 0;
	}
	auto terminal = std::make_shared<ui::ConsoleTerminal>();
	auto crypto = std::make_shared<security::crypto::SodiumCryptoService>();
	auto database = std::make_shared<security::storage::FileDatabase>(crypto);
	app::Application app(terminal, crypto, database);
	return app.run();
}