#pragma once

#include <memory>
#include "abstract_command.h"

class AbstractCommandDispatcher
{
public:
    virtual ~AbstractCommandDispatcher() = default;

    virtual void postCommand(
        std::unique_ptr<AbstractCommand> cmd
    ) = 0;
};