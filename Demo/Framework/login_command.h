#pragma once
#include "abstract_command.h"
#include <thread>
#include <iostream>

class LoginCommand : public AbstractCommand
{
public:
    explicit LoginCommand(CommandContext ctx)
    {
        m_context = std::move(ctx);
    }

    bool execute() override
    {
        for (auto& r : m_receivers)
        {
            r->performAction(m_context);
        }
        return true;
    }

    std::string getCommandType() const override
    {
        return "LoginCommand";
    };

};