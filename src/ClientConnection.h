// This file may be redistributed and modified only under the terms of
// the GNU Lesser General Public License (See COPYING for details).
// Copyright (C) 2001 Alistair Riddoch

#ifndef CLIENT_CONNECTION_H
#define CLIENT_CONNECTION_H

#include <Atlas/Objects/Decoder.h>
#include <Atlas/Objects/Encoder.h>
#include <Atlas/Codec.h>

#include <skstream/skstream.h>

#include <deque>
#include <map>

#include "opfwd.h"

typedef std::map<std::string, std::string> StringMap;

typedef std::deque<Atlas::Objects::Operation::RootOperation *> OperationDeque;

class ClientConnection : public Atlas::Objects::Decoder {
  private:
    bool reply_flag;
    bool error_flag;
    tcp_socket_stream ios;
    Atlas::Codec<std::iostream> * codec;
    Atlas::Objects::Encoder * encoder;
    std::string accountId;
    
    Atlas::Message::Object::MapType reply;
    static int serialNoBase;
    int serialNo;

	/** the serialno of the OOG activation op, either account
	creation or longin. We watch for an INFO whose REFNO
	matches this to determine our account ID. */
	int oogActivationRefno;
  
    OperationDeque operationQueue;


    OperationDeque::iterator checkQueue(const std::string &opType, int refno);
  
    template<class O>
    void push(const O &);

    virtual void ObjectArrived(const Atlas::Objects::Operation::Error&);
    virtual void ObjectArrived(const Atlas::Objects::Operation::Info&);

    virtual void ObjectArrived(const Atlas::Objects::Operation::Action&);
    virtual void ObjectArrived(const Atlas::Objects::Operation::Appearance&);
    virtual void ObjectArrived(const Atlas::Objects::Operation::Combine&);
    virtual void ObjectArrived(const Atlas::Objects::Operation::Communicate&);
    virtual void ObjectArrived(const Atlas::Objects::Operation::Create&);
    virtual void ObjectArrived(const Atlas::Objects::Operation::Delete&);
    virtual void ObjectArrived(const Atlas::Objects::Operation::Disappearance&);
    virtual void ObjectArrived(const Atlas::Objects::Operation::Divide&);
    virtual void ObjectArrived(const Atlas::Objects::Operation::Feel&);
    virtual void ObjectArrived(const Atlas::Objects::Operation::Get&);
    virtual void ObjectArrived(const Atlas::Objects::Operation::Imaginary&);
    virtual void ObjectArrived(const Atlas::Objects::Operation::Listen&);
    virtual void ObjectArrived(const Atlas::Objects::Operation::Login&);
    virtual void ObjectArrived(const Atlas::Objects::Operation::Logout&);
    virtual void ObjectArrived(const Atlas::Objects::Operation::Look&);
    virtual void ObjectArrived(const Atlas::Objects::Operation::Move&);
    virtual void ObjectArrived(const Atlas::Objects::Operation::Perceive&);
    virtual void ObjectArrived(const Atlas::Objects::Operation::Perception&);
    virtual void ObjectArrived(const Atlas::Objects::Operation::RootOperation&);
    virtual void ObjectArrived(const Atlas::Objects::Operation::Set&);
    virtual void ObjectArrived(const Atlas::Objects::Operation::Sight&);
    virtual void ObjectArrived(const Atlas::Objects::Operation::Smell&);
    virtual void ObjectArrived(const Atlas::Objects::Operation::Sniff&);
    virtual void ObjectArrived(const Atlas::Objects::Operation::Sound&);
    virtual void ObjectArrived(const Atlas::Objects::Operation::Talk&);
    virtual void ObjectArrived(const Atlas::Objects::Operation::Touch&);

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
                 const Atlas::Message::Object::MapType &,
                 int refNo = -1);
  
    bool waitForError(int refNo = -1);

    bool compareArgToTemplate(Atlas::Objects::Operation::RootOperation* op, 
        const Atlas::Message::Object::MapType & arg);

    void setTag(const std::string &tag, const std::string &value);
    bool hasTag(const std::string &t)
    { return m_tags.find(t) != m_tags.end(); }
    
    std::string getTag(const std::string &t);
    
    // check the queue / wait for the specific op, and return it
    Atlas::Objects::Operation::RootOperation*
    recv(const std::string & opParent, int refno);
  
    int send(Atlas::Objects::Operation::RootOperation & op);
    void error(const std::string & message);
    bool poll(int time);
    RootOperation * pop();
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
    const Atlas::Message::Object::MapType & getReply() {
        return reply;
    }
    const bool isOpen() const;

    static const int timeOut = 5;
};

ClientConnection* getConnectionBySpec(const std::string &spec);

#endif // CLIENT_CONNECTION_H

