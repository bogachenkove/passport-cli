#pragma once

#include "../../interface/interface_database.hpp"
#include "../../interface/interface_terminal.hpp"
#include <memory>

namespace app::commands {

class ListRecordsCommand {
public:
    ListRecordsCommand(
        std::shared_ptr<domain::interfaces::ITerminal> terminal,
        std::shared_ptr<domain::interfaces::IDatabase> database);

    void execute();

private:
    std::shared_ptr<domain::interfaces::ITerminal> term_;
    std::shared_ptr<domain::interfaces::IDatabase> db_;
};

} // namespace app::commands