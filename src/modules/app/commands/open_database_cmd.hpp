#pragma once

#include "../../interface/interface_database.hpp"
#include "../../interface/interface_terminal.hpp"
#include <memory>
#include <string>

namespace app::commands {

class OpenDatabaseCommand {
public:
    OpenDatabaseCommand(
        std::shared_ptr<domain::interfaces::ITerminal> terminal,
        std::shared_ptr<domain::interfaces::IDatabase> database);

    bool execute(std::string& out_db_path, std::string& out_master_pw);

private:
    std::shared_ptr<domain::interfaces::ITerminal> term_;
    std::shared_ptr<domain::interfaces::IDatabase> db_;
};

} // namespace app::commands