#pragma once

#include "../../interface/interface_database.hpp"
#include "../../interface/interface_terminal.hpp"
#include "../../interface/interface_crypto_service.hpp"  // NEW
#include <memory>
#include <string>

namespace app::commands {

    class CreateDatabaseCommand {
    public:
        CreateDatabaseCommand(
            std::shared_ptr<domain::interfaces::ITerminal> terminal,
            std::shared_ptr<domain::interfaces::IDatabase> database,
            std::shared_ptr<domain::interfaces::ICryptoService> crypto);   // NEW

        bool execute(std::string& out_db_path, std::string& out_master_pw);

    private:
        std::shared_ptr<domain::interfaces::ITerminal> term_;
        std::shared_ptr<domain::interfaces::IDatabase> db_;
        std::shared_ptr<domain::interfaces::ICryptoService> crypto_;      // NEW

        bool prompt_overwrite(const std::string& path);
    };

} // namespace app::commands