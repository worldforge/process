// This file may be redistributed and modified only under the terms of
// the GNU Lesser General Public License (See COPYING for details).
// Copyright (C) 2001 Alistair Riddoch

#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#include <algorithm>
#include <iostream>

#include <Atlas/Codec.h>
#include <Atlas/Message/Object.h>
#include <Atlas/Net/Stream.h>
#include <Atlas/Objects/Decoder.h>
#include <Atlas/Objects/Encoder.h>

#include <sys/time.h>

#include "ClientConnection.h"

#include "debug.h"
#include "process.h"

static const bool debug_flag = false;

typedef std::vector<std::string> StringVec;
StringVec tokenize(const std::string &s, const char t);

using Atlas::Message::Object;

static inline const std::string typeAsString(const Object & o)
{
    switch (o.GetType()) {
        case Object::TYPE_NONE:
            return "none";
        case Object::TYPE_INT:
            return "integer";
        case Object::TYPE_FLOAT:
            return "float";
        case Object::TYPE_STRING:
            return "string";
        case Object::TYPE_MAP:
            return "map";
        case Object::TYPE_LIST:
            return "list";
        default:
            return "unknown";
    }
}

typedef std::list<ClientConnection*>  ConnectionList;
static ConnectionList static_allConnections;

ClientConnection::ClientConnection() :
    encoder(NULL), serialNo(512)
{
    static_allConnections.push_back(this);
}

ClientConnection::~ClientConnection()
{
    if (encoder != NULL) {
        delete encoder;
    }
    
    static_allConnections.remove(this);
}

void ClientConnection::ObjectArrived(const Error&op)
{
    debug(std::cout << "Error" << std::endl << std::flush;);
    push(op);
    reply_flag = true;
    error_flag = true;
}

void ClientConnection::ObjectArrived(const Info & op)
{
    debug(std::cout << "Info" << std::endl << std::flush;);
    push(op);
    const std::string & from = op.GetFrom();
    if (from.empty()) {
        try {
            Object ac = op.GetArgs().front();
            reply = ac.AsMap();
            // const std::string & acid = reply["id"].AsString();
            // objects[acid] = new ClientAccount(acid, *this);
        }
        catch (...) {
        }
    }
}

void ClientConnection::ObjectArrived(const Action& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Appearance& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Combine& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Atlas::Objects::Operation::Communicate& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Create& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Delete& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Disappearance& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Divide& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Atlas::Objects::Operation::Feel& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Get& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Imaginary& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Atlas::Objects::Operation::Listen& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Login& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Logout& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Look& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Move& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Atlas::Objects::Operation::Perceive& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Atlas::Objects::Operation::Perception& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const RootOperation& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Set& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Sight& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Atlas::Objects::Operation::Smell& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Atlas::Objects::Operation::Sniff& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Sound& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Talk& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Touch& op)
{
    push(op);
}

int ClientConnection::read() {
    if (ios.is_open()) {
        codec->Poll();
        return 0;
    } else {
        return -1;
    }
}

bool ClientConnection::connect(const std::string & server)
{
    if (ios.is_open()) {
	std::cerr << "Conection is already connected!" << std::endl;
	return false;
    }
    
    ios.clear(); // clear any lingering errors (necessary for re-conncections)
    ios.open(server, 6767);
    if (!ios.is_open()) {
	std::cerr << "failed to open socket" << std::endl;
	return false;
    }
    
    Atlas::Net::StreamConnect conn("cyphesis_aiclient", ios, this);

    while (conn.GetState() == Atlas::Net::StreamConnect::IN_PROGRESS) {
      conn.Poll();
    }
  
    if (conn.GetState() == Atlas::Net::StreamConnect::FAILED) {
	std::cerr << "atlas negotiation failed" << std::endl;
        return false;
    }

    codec = conn.GetCodec();

    encoder = new Atlas::Objects::Encoder(codec);

    codec->StreamBegin();

    return true;
}

void ClientConnection::close()
{
    ios.close();
}

bool ClientConnection::login(const std::string & account,
                             const std::string & password)
{
    Atlas::Objects::Operation::Login l = Atlas::Objects::Operation::Login::Instantiate();
    Object::MapType acmap;
    acmap["username"] = account;
    acmap["password"] = password;
    acmap["parents"] = Object::ListType(1,"account");
    acmap["objtype"] = "object";

    setTag("account", account);
    setTag("pass", password);
    
    l.SetArgs(Object::ListType(1,Object(acmap)));

    reply_flag = false;
    error_flag = false;
    send(l);
    return true;
}

bool ClientConnection::create(const std::string & account,
                              const std::string & password)
{
    Atlas::Objects::Operation::Create c = Atlas::Objects::Operation::Create::Instantiate();
    Object::MapType acmap;
    acmap["username"] = account;
    acmap["password"] = password;
    acmap["parents"] = Object::ListType(1,"account");
    acmap["objtype"] = "object";

    setTag("account", account);
    setTag("pass", password);

    c.SetArgs(Object::ListType(1,Object(acmap)));

    reply_flag = false;
    error_flag = false;
    send(c);
    return true;
}

bool ClientConnection::wait(int time, bool error_expected)
// Waits for response from server. Used when we are expecting a login response
// Return whether or not an error occured
{
    error_flag = false;
    reply_flag = false;
    debug( std::cout << "WAITing" << std::endl << std::flush; );
    if (!poll(time)) {
        std::cerr << "ERROR: Timeout waiting for reply"
                  << std::endl << std::flush;
        regress( std::cout << "Timeout while waiting for reply"
                           << std::endl << std::flush;);
        return true;
    }
    if (!reply_flag) {
        std::cerr << "ERROR: Reply was not decoded as an operation"
                  << std::endl << std::flush;
        regress( std::cout << "Invalid reply received"
                           << std::endl << std::flush;);
        return true;
    }
    // codec->Poll();
    debug(std::cout << "WAIT finished" << std::endl << std::flush;);
    return error_expected ? !error_flag : error_flag;
}

bool ClientConnection::waitFor(const std::string & opParent,
                               const Object::MapType & arg,
                               bool error_expected)
{
    if (wait(timeOut, error_expected)) {
        return true;
    }
    RootOperation * op;
    std::string opP;
    do {
        poll(0);
        op = pop();
        if (op == NULL) {
            std::cerr << "ERROR: No response to operation"
                      << std::endl << std::flush;
            return true;
        }
        opP = op->GetParents().front().AsString();
        verbose( std::cout << "Got op of type " << opP << std::endl << std::flush;);
    } while (opP != opParent);
    //const std::string & p = op->GetParents().front().AsString();
    //if (p != opParent) {
        //std::cerr << "ERROR: Response to operation has parent " << p
                  //<< "but it should have parent " << opParent
                  //<< std::endl << std::flush;
        //return true;
    //}
    const Object::ListType & args = op->GetArgs();
    if (arg.empty()) {
        if (!args.empty()) {
            std::cerr << "ERROR: Response to operation has args "
                      << "but no args expected"
                      << std::endl << std::flush;
            return true;
        }
        debug(std::cout << "No arg expected, and none given"
                        << std::endl << std::flush;);
        return false;
    } else {
        if (args.empty()) {
            std::cerr << "ERROR: Response to operation has no args "
                      << "but args are expected"
                      << std::endl << std::flush;
            return true;
        }
        debug(std::cout << "Arg expected, and provided" << std::endl
                        << std::flush;);
    }
    const Object::MapType & a = args.front().AsMap();
    Object::MapType::const_iterator I, J;
    bool error = false;
    for (I = arg.begin(); I != arg.end(); I++) {
        J = a.find(I->first);
        if (J == a.end()) {
            std::cerr << "ERROR: Response to operation args should have "
                      << "attribute '" << I->first << "' of type "
                      << typeAsString(I->second) << " but it is missing"
                      << std::endl << std::flush;
            error = true;
            continue;
        }
        if (I->second.IsNone()) {
            continue;
        }
        if (I->second.GetType() != J->second.GetType()) {
            if (I->second.IsNum() && J->second.IsNum()) {
                std::cerr << "WARNING: Response to operation args should have "
                          << "attribute '" << I->first << "' of type "
                          << typeAsString(I->second) << " but it is of type "
                          << typeAsString(J->second)
                          << std::endl << std::flush;
            } else {
                std::cerr << "ERROR: Response to operation args should have "
                          << "attribute '" << I->first << "' of type "
                          << typeAsString(I->second) << " but it is of type "
                          << typeAsString(J->second)
                          << std::endl << std::flush;
                error = true;
            }
        }
    }
    return error;
}

RootOperation* ClientConnection::recv(const std::string & opParent, int refno)
{
    OperationDeque::iterator I = checkQueue(opParent, refno);
    int remainingTime = timeOut;
    struct timeval tm, initialTm;
    
    ::gettimeofday(&initialTm, NULL);
    
    while (I == operationQueue.end()) {
	poll(remainingTime);
	I = checkQueue(opParent, refno);
	
	// update the remaining time (roughly, since we only use seconds .. millsecs would be easy)
	::gettimeofday(&tm, NULL);
	int elapsed = tm.tv_sec - initialTm.tv_sec;
	remainingTime = timeOut - elapsed;
    }
    
    if (I == operationQueue.end())
	return NULL;	// we failed miserably
    
    RootOperation *ret = *I;
    operationQueue.erase(I);
    return ret;
}

/* wait for an error operation; the provided arg will be the op that caused the error,
 i.e args[1] of the ERROR op */
bool ClientConnection::waitForError(const Object::MapType & arg)
{
    if (wait(timeOut, true)) {
        return true;
    }
    RootOperation * op;
    std::string opP;
    do {
        poll(0);
        op = pop();
        if (op == NULL) {
            std::cerr << "ERROR: No response to operation"
                      << std::endl << std::flush;
            return true;
        }
        opP = op->GetParents().front().AsString();
        verbose( std::cout << "Got op of type " << opP << std::endl << std::flush;);
    } while (opP != "error");

    const Object::ListType & args = op->GetArgs();
    if (arg.empty()) {
        debug(std::cout << "No arg expected"
                        << std::endl << std::flush;);
        return false;
    } else {
        if (args.empty()) {
            std::cerr << "ERROR: Response to operation has no args "
                      << "but args are expected"
                      << std::endl << std::flush;
            return true;
        }
        debug(std::cout << "Arg expected, and provided" << std::endl
                        << std::flush;);
    }
    const Object::MapType & a = args[1].AsMap();
    Object::MapType::const_iterator I, J;
    bool error = false;
    for (I = arg.begin(); I != arg.end(); I++) {
        J = a.find(I->first);
        if (J == a.end()) {
            std::cerr << "ERROR: Response to operation args should have "
                      << "attribute '" << I->first << "' of type "
                      << typeAsString(I->second) << " but it is missing"
                      << std::endl << std::flush;
            error = true;
            continue;
        }
        if (I->second.IsNone()) {
            continue;
        }
        if (I->second.GetType() != J->second.GetType()) {
            if (I->second.IsNum() && J->second.IsNum()) {
                std::cerr << "WARNING: Response to operation args should have "
                          << "attribute '" << I->first << "' of type "
                          << typeAsString(I->second) << " but it is of type "
                          << typeAsString(J->second)
                          << std::endl << std::flush;
            } else {
                std::cerr << "ERROR: Response to operation args should have "
                          << "attribute '" << I->first << "' of type "
                          << typeAsString(I->second) << " but it is of type "
                          << typeAsString(J->second)
                          << std::endl << std::flush;
                error = true;
            }
        }
    }
    return error;
}

/** determine whether a requested operation is in the queue already; if so, return an
    iterator to it. Note we have to be very careful about refno handling, since some ops
   generate multiple response with identical refnos (IG transitions being an
  immediate example. */
OperationDeque::iterator
ClientConnection::checkQueue(const std::string &opType, int refno)
{
    OperationDeque::iterator I;
    for (I=operationQueue.begin(); I != operationQueue.end(); ++I) {
	// if a defined refno is being used, it has to match
	if ((refno > 0) && ((*I)->GetRefno() != refno))
	    continue;
	
	Object::ListType parents = (*I)->GetParents();
	if (parents.empty())
	    continue;	// isn't this very bad?
	
	if (opType == parents.front().AsString())
	    return I;
    }
    
    return I;
}

void ClientConnection::send(Atlas::Objects::Operation::RootOperation & op)
{
    op.SetSerialno(++serialNo);
    encoder->StreamMessage(&op);
    ios << std::flush;
}

void ClientConnection::error(const std::string & message)
{
    // FIXME Need operation based error function
}

bool ClientConnection::poll(int time)
{
    fd_set infds;
    struct timeval tv;

    FD_ZERO(&infds);

    int client_fd = ios.getSocket();
    FD_SET(client_fd, &infds);

    tv.tv_sec = time;
    tv.tv_usec = 0;

    int retval = select(client_fd+1, &infds, NULL, NULL, &tv);

    if ((retval > 0) && (FD_ISSET(client_fd, &infds))) {
        if (ios.peek() == -1) {
            return false;
        }
        codec->Poll();
        return true;
    }
    return false;

}

RootOperation * ClientConnection::pop()
{
    poll(0);
    if (operationQueue.empty()) {
        return NULL;
    }
    RootOperation * op = operationQueue.front();
    operationQueue.pop_front();
    return op;
}

bool ClientConnection::pending()
{
    return !operationQueue.empty();
}

template<class O>
void ClientConnection::push(const O & op)
{
    reply_flag = true;
    RootOperation * new_op = new O(op); 
    operationQueue.push_back(new_op);
}


const bool ClientConnection::isOpen() const
{
    return ios.is_open();
}


void ClientConnection::setTag(const std::string &tag, const std::string &value)
{
    m_tags[tag] = value;
}

std::string ClientConnection::getTag(const std::string &t)
{
    StringMap::iterator T=m_tags.find(t);
    if (T != m_tags.end())
	return T->second;
    
    return "";
}

class TokenRemover
{
public:
    TokenRemover(const std::string& pair) :
	m_valueValid(false)
    {
	size_t eqPos = pair.find('='), len = pair.length();
	if (eqPos < len) {
	    m_valueValid = true;
	    m_value = pair.substr(eqPos + 1, len - (eqPos + 1));
	}
	
	m_tag = pair.substr(0, eqPos);
    }
    
    bool operator()(ClientConnection *con)
    {
	if (m_valueValid) {
	    return !(con->hasTag(m_tag) && (con->getTag(m_tag) == m_value));
	} else
	    return !con->hasTag(m_tag);
    }
private:
    std::string m_tag, m_value;
    bool m_valueValid;
};

ClientConnection* getConnectionBySpec(const std::string &spec)
{
    StringVec toks = tokenize(spec, ',');
    if (toks.empty())
	return NULL;
    
    ConnectionList valid(static_allConnections);
    
    for (unsigned int T=0;T<toks.size();++T) {
	if (valid.empty())
	    return NULL;	// no possible matches, we==boned

	TokenRemover trm(toks[T]);
	std::remove_if(valid.begin(), valid.end(), trm);
    }
    
    if (valid.empty())
	return NULL;
    else
	return valid.front();
}

StringVec tokenize(const std::string &s, const char t)
{
    StringVec ret;
    
    unsigned int pos = 0, back = pos;
    while (pos < s.size()) {
	pos = s.find(t, back);
	// addthe next part
	ret.push_back(s.substr(back, pos - back));
	back = pos + 1;
    }
	    
    return ret;
}
