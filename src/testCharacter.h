// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2001-2003 Alistair Riddoch and James Turner

#ifndef PROCESS_TEST_CHARACTER_H
#define PROCESS_TEST_CHARACTER_H

class ClientConnection;

void testCharacterCreate(ClientConnection&,
                         ClientConnection&,
                         ClientConnection&);
void testInvalidCharacterCreate(ClientConnection &c);    

#endif // PROCESS_TEST_IG_H
