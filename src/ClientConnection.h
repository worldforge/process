// This file may be redistributed and modified only under the terms of
// the GNU Lesser General Public License (See COPYING for details).
// Copyright (C) 2001-2003 Alistair Riddoch

#ifndef PROCESS_CLIENT_CONNECTION_H
#define PROCESS_CLIENT_CONNECTION_H

#include <Atlas/Objects/Decoder.h>
#include <Atlas/Objects/ObjectsFwd.h>

#include <skstream/skstream.h>

#include <deque>

#include "opfwd.h"

namespace Atlas {
  class Codec;
  namespace Objects {
    class ObjectsEncoder;
  }
}

typedef std::map<std::string, std::string> StringMap;

typedef std::deque<Atlas::Objects::Operation::RootOperation> OperationDeque;

class ClientConnection : public Atlas::Objects::ObjectsDecoder {
  private:
    bool reply_flag;
    bool error_flag;
    tcp_socket_stream ios;
    Atlas::Codec * codec;
    Atlas::Objects::ObjectsEncoder * encoder;
    std::string accountId;
    
    Atlas::Message::MapType reply;
    Atlas::Message::ListType m_spawns;
    static int serialNoBase;
    int serialNo;

    /** the serialno of the OOG activation op, either account
    creation or longin. We watch for an INFO whose REFNO
    matches this to determine our account ID. */
    int oogActivationRefno;
  
    OperationDeque operationQueue;


    OperationDeque::iterator checkQueue(const std::string &opType, int refno);
  
    void push(const Atlas::Objects::Root &);

    virtual void objectInfoArrived(const Atlas::Objects::Operation::Info&);
    virtual void objectArrived(const Atlas::Objects::Root &);

    StringMap m_tags;

  public:
    ClientConnection();
    ~ClientConnection();

    int read();
    bool connect(const std::string &);
    void close();
    int login(const std::string &, const std::string &);
    int create(const std::string &, const std::string &);
    bool wait(int time = 0, bool error_expected = false, int refNo = -1);
    bool waitFor(const std::string &,
                 const Atlas::Message::MapType &,
                 int refNo = -1);
  
    bool waitForError(int refNo = -1);

    bool createChar(const Atlas::Objects::Root & charData);
    
    bool compareArgToTemplate(const Atlas::Objects::Operation::RootOperation & op, 
        const Atlas::Message::MapType & arg);

    void setTag(const std::string &tag, const std::string &value);
    bool hasTag(const std::string &t)
    { return m_tags.find(t) != m_tags.end(); }
    
    std::string getTag(const std::string &t);
    
    // check the queue / wait for the specific op, and return it
    const Atlas::Objects::Operation::RootOperation recv(
                   const std::string & opParent, int refno);
  
    int send(Atlas::Objects::Operation::RootOperation msg);
    void sendOther(Atlas::Objects::Root msg);
    void error(const std::string & message);
    bool poll(int time);
    RootOperation pop();
    bool pending();

    int addSpawnData(const Atlas::Objects::Entity::RootEntity &);

    const std::string getAccount()
    { return getTag("account"); }
    
    const std::string getPassword()
    { return getTag("pass"); }

    const std::string getCharacterId()
    {
        return getTag("character");
    }

    const std::string & getAccountId() const {
        return accountId;
    }

    int getLastSerialno()
    { return serialNo; }
    
    int peek() {
        return ios.peek();
    }
    int eof() {
        return ios.eof();
    }
    int get_fd() {
        return ios.getSocket();
    }
    const Atlas::Message::MapType & getReply() {
        return reply;
    }
    const bool isOpen() const;

    static const int timeOut = 5;
};

ClientConnection* getConnectionBySpec(const std::string &spec);

#endif // PROCESS_CLIENT_CONNECTION_H
