// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2001-2003 Alistair Riddoch and James Turner

#ifndef PROCESS_TESTS_H
#define PROCESS_TESTS_H

class ClientConnection;

void testTypeQueries(ClientConnection &c);
void testLogout(ClientConnection &c,
                const std::string &acc,
                ClientConnection &watcher);
void testDuplicateLogin(const std::string &account, const std::string &pass);

#endif // PROCESS_TESTS_H
