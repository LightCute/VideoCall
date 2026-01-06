#ifndef CLIENTEVENTFACTORY_H
#define CLIENTEVENTFACTORY_H

#include "protocol_types.h"
#include "ClientEvent.h"
class ClientEventFactory
{
public:
    ClientEventFactory();
    static ClientEvent makeEvent(const std::string& msg);

private:


};

#endif // CLIENTEVENTFACTORY_H
