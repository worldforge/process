// This file may be redistributed and modified only under the terms of
// the GNU Lesser General Public License (See COPYING for details).
// Copyright (C) 2001-2003 Alistair Riddoch

#ifndef CLIENT_CONNECTION_H
#define CLIENT_CONNECTION_H

#include <Atlas/Objects/Decoder.h>

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
    
    Atlas::Message::Element::MapType reply;
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
                 const Atlas::Message::Element::MapType &,
                 int refNo = -1);
  
    bool waitForError(int refNo = -1);

    bool compareArgToTemplate(const Atlas::Objects::Operation::RootOperation & op, 
        const Atlas::Message::Element::MapType & arg);

    void setTag(const std::string &tag, const std::string &value);
    bool hasTag(const std::string &t)
    { return m_tags.find(t) != m_tags.end(); }
    
    std::string getTag(const std::string &t);
    
    // check the queue / wait for the specific op, and return it
    const Atlas::Objects::Operation::RootOperation recv(
                   const std::string & opParent, int refno);
  
    int send(Atlas::Objects::Operation::RootOperation msg);
    void error(const std::string & message);
    bool poll(int time);
    RootOperation pop();
    bool pending();

    const std::string getAccount()
    { return getTag("account"); }
    
    const std::string getPassword()
    { return getTag("pass"); }

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
    const Atlas::Message::Element::MapType & getReply() {
        return reply;
    }
    const bool isOpen() const;

    static const int timeOut = 5;
};

ClientConnection* getConnectionBySpec(const std::string &spec);

#endif // CLIENT_CONNECTION_H
