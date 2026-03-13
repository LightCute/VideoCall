#pragma once
#include "../../../framework/command/abstract_command.h"


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