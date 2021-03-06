// This file may be redistributed and modified only under the terms of
// the GNU Lesser General Public License (See COPYING for details).
// Copyright (C) 2001 Alistair Riddoch

#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#include <algorithm>
#include <iostream>

#include <Atlas/Codec.h>
#include <Atlas/Net/Stream.h>
#include <Atlas/Objects/Decoder.h>
#include <Atlas/Objects/Encoder.h>
#include <Atlas/Objects/Entity.h>

#include <sys/time.h>

#include "ClientConnection.h"

#include "process_debug.h"

using Atlas::Objects::Entity::Anonymous;

static const bool debug_flag = false;

typedef std::vector<std::string> StringVec;
StringVec tokenize(const std::string &s, const char t);

int ClientConnection::serialNoBase = 0;

using Atlas::Message::Element;
using Atlas::Message::ListType;
using Atlas::Message::MapType;
using Atlas::Objects::Root;

static inline const std::string typeAsString(const Element & o)
{
    switch (o.getType()) {
        case Element::TYPE_NONE:
            return "none";
        case Element::TYPE_INT:
            return "integer";
        case Element::TYPE_FLOAT:
            return "float";
        case Element::TYPE_STRING:
            return "string";
        case Element::TYPE_MAP:
            return "map";
        case Element::TYPE_LIST:
            return "list";
        default:
            return "unknown";
    }
}

typedef std::list<ClientConnection*>  ConnectionList;
static ConnectionList static_allConnections;

ClientConnection::ClientConnection() :
    encoder(NULL), serialNo(serialNoBase += 512), oogActivationRefno(-1)
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

#if 0
void ClientConnection::ObjectArrived(const Error&op)
{
    debug(std::cout << "Error" << std::endl << std::flush;);
    push(op);
    reply_flag = true;
    error_flag = true;
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
#else
void ClientConnection::objectInfoArrived(const Info & op)
{
    debug(std::cout << "Info" << std::endl << std::flush;);
    // push(op);
        
    if (accountId.empty() && (op->getRefno() == oogActivationRefno)) {
        try {
            const Root & ac = op->getArgs().front();
            const std::string & id = ac->getId();
            if (!id.empty()) {
                accountId = id;
                verbose_only( std::cout << "Got account id of " << accountId
                                        << std::endl << std::flush; );
                Element spawns;
                if (ac->copyAttr("spawns", spawns) == 0 && spawns.isList()) {
                    debug( std::cout << "Got list of " << spawns.List().size()
                                     << " spawns" << std::endl << std::flush; );
                    m_spawns = spawns.List();
                }
            }
        }
        catch (...) {
        }
    }
}

void ClientConnection::objectArrived(const Root & op)
{
    debug( std::cout << "Got something" << std::endl << std::flush; );
    if (op->getClassNo() == Atlas::Objects::Operation::INFO_NO) {
        objectInfoArrived(Atlas::Objects::smart_dynamic_cast<Info>(op));
    }
    push(op);
}
#endif

int ClientConnection::read() {
    if (ios.is_open()) {
        codec->poll();
        return 0;
    } else {
        return -1;
    }
}

bool ClientConnection::connect(const std::string & server)
{
    if (ios.is_open()) {
        std::cerr << "Attempt to connect when already connected." << std::endl;
        return false;
    }
    
    ios.clear(); // clear any lingering errors (necessary for re-conncections)
    ios.open(server, 6767);
    if (!ios.is_open()) {
        std::cerr << "Failed to create TCP socket for connection." << std::endl;
        return false;
    }
    
    Atlas::Net::StreamConnect conn("process", ios);

    while (conn.getState() == Atlas::Net::StreamConnect::IN_PROGRESS) {
        conn.poll();
    }
  
    if (conn.getState() == Atlas::Net::StreamConnect::FAILED) {
        std::cerr << "Atlas protocol negotiation with server failed."
                  << std::endl;
        return false;
    }

    codec = conn.getCodec(*this);

    encoder = new Atlas::Objects::ObjectsEncoder(*codec);

    codec->streamBegin();

    return true;
}

void ClientConnection::close()
{
    ios.close();
}

int ClientConnection::login(const std::string & account,
                            const std::string & password)
{
    Atlas::Objects::Operation::Login l;
    Anonymous account_arg;
    account_arg->setAttr("username", account);
    account_arg->setAttr("password", password);
    account_arg->setParents(std::list<std::string>(1,"account"));
    account_arg->setObjtype("object");

    setTag("account", account);
    setTag("pass", password);
    
    l->setArgs1(account_arg);

    reply_flag = false;
    error_flag = false;
    oogActivationRefno = send(l);
    return oogActivationRefno;
}

int ClientConnection::create(const std::string & account,
                             const std::string & password)
{
    Atlas::Objects::Operation::Create c;
    Anonymous account_arg;
    account_arg->setAttr("username", account);
    account_arg->setAttr("password", password);
    account_arg->setParents(std::list<std::string>(1,"account"));
    account_arg->setObjtype("object");

    setTag("account", account);
    setTag("pass", password);

    c->setArgs1(account_arg);

    reply_flag = false;
    error_flag = false;
    oogActivationRefno = send(c);
        return oogActivationRefno;
}

bool ClientConnection::createChar(const Root & charData)
{
    Atlas::Objects::Operation::Create create;
    create->setFrom(accountId);
    create->setArgs1(charData);

    int serialno = send(create);
    verbose( std::cout << "Waiting for info response to character creation"
                       << std::endl << std::flush; );

// wait for the INFO response
    OperationDeque::iterator I = checkQueue("info", serialno);
    int remainingTime = timeOut;
    struct timeval tm, initialTm;
    ::gettimeofday(&initialTm, NULL);
    
    while ((I == operationQueue.end()) && (remainingTime > 0)) {
        poll(remainingTime);
        I = checkQueue("info", serialno);
        
        ::gettimeofday(&tm, NULL);
        int elapsed = tm.tv_sec - initialTm.tv_sec;
        remainingTime = timeOut - elapsed;
    }
        
    if (I == operationQueue.end()) {
        std::cerr << "no INFO response to character creation" << std::endl;
        return false;
    }

// see what we got
    Atlas::Objects::Operation::RootOperation op = *I;
    operationQueue.erase(I);
    
    Atlas::Objects::Entity::RootEntity ent = 
        Atlas::Objects::smart_dynamic_cast<Atlas::Objects::Entity::RootEntity>(op->getArgs()[0]);

    if (!ent.isValid()) {
        std::cerr << "INFO response to character creation does not have an entity argument" << std::endl;
        return false;
    }
    
    setTag("character",  ent->getId());
    return true;
}

bool ClientConnection::wait(int time, bool error_expected, int refNo)
// Waits for response from server. Used when we are expecting a login response
// Return whether or not an error occured
{
    error_flag = false;
    reply_flag = false;
    debug( std::cout << "WAITing" << std::endl << std::flush; );
    if (!poll(time)) {
        std::cerr << "ERROR: Timeout waiting for reply from server."
                  << std::endl << std::flush;
        regress( std::cout << "Timeout while waiting for reply from server."
                           << std::endl << std::flush;);
        return true;
    }
    if (!reply_flag) {
        std::cerr << "ERROR: Reply from server was not decoded as an operation."
                  << std::endl << std::flush;
        regress( std::cout << "Invalid reply received from server."
                           << std::endl << std::flush;);
        return true;
    }
    // codec->poll();
    debug(std::cout << "WAIT finished" << std::endl << std::flush;);
    return error_expected ? !error_flag : error_flag;
}

bool ClientConnection::waitFor(const std::string & opParent,
                               const MapType & arg,
                               int refNo)
{
    if (refNo == -1) {
        verbose(std::cout << "Waiting for " << opParent << " op" << std::endl;);
    } else {
        verbose(std::cout << "Waiting for " << opParent << " op with refno "
                          << refNo << std::endl;);
    }
    OperationDeque::iterator I = checkQueue(opParent, refNo);
    int remainingTime = timeOut;
    struct timeval tm, initialTm;
    
    ::gettimeofday(&initialTm, NULL);
    
    while ((I == operationQueue.end()) && (remainingTime > 0)) {
        poll(remainingTime);
        I = checkQueue(opParent, refNo);
        
        ::gettimeofday(&tm, NULL);
        int elapsed = tm.tv_sec - initialTm.tv_sec;
        remainingTime = timeOut - elapsed;
    }
        
    if (I == operationQueue.end()) {
        return true;    // we failed miserably
    }
    
    RootOperation op = *I;
    operationQueue.erase(I);

    return compareArgToTemplate(op, arg);
}

bool ClientConnection::compareArgToTemplate(const RootOperation & op, const MapType & templ_arg)
{
    const std::vector<Root> args = op->getArgs();
    if (templ_arg.empty()) {
        if (!args.empty()) {
            std::cerr << "ERROR: Response from server has args "
                      << "but no args expected."
                      << std::endl << std::flush;
            return true;
        }
        debug(std::cout << "Response from server has no args as expected."
                        << std::endl << std::flush;);
        return false;
    } else {
        if (args.empty()) {
            std::cerr << "ERROR: Response from server has no args "
                      << "but args are expected."
                      << std::endl << std::flush;
            return true;
        }
        debug(std::cout << "Response from server has args as expected."
                        << std::endl << std::flush;);
    }
    
    const MapType & a = args.front()->asMessage();
    MapType::const_iterator J, K;
    bool error = false;
    for (J = templ_arg.begin(); J != templ_arg.end(); J++) {
        K = a.find(J->first);
        if (K == a.end()) {
            std::cerr << "ERROR: Response from server should have an argument "
                      << " with attribute '" << J->first << "' of type "
                      << typeAsString(J->second) << " but it is missing"
                      << std::endl << std::flush;
            error = true;
            continue;
        }
        if (J->second.isNone()) {
            continue;
        }
        if (J->second.getType() != K->second.getType()) {
            if (J->second.isNum() && K->second.isNum()) {
                std::cerr << "WARNING: Response from server should have an "
                          << " argument with attribute '" << J->first
                          << "' of type " << typeAsString(J->second)
                          << " but it is of type " << typeAsString(K->second)
                          << std::endl << std::flush;
            } else {
                std::cerr << "ERROR: Response from server should have an "
                          << " argument with attribute '" << J->first
                          << "' of type " << typeAsString(J->second)
                          << " but it is of type " << typeAsString(K->second)
                          << std::endl << std::flush;
                error = true;
            }
        }
    }
    return error;

}

const RootOperation ClientConnection::recv(const std::string & opParent,
                                           int refno)
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
        return NULL;    // we failed miserably
    
    RootOperation ret = *I;
    operationQueue.erase(I);
    return ret;
}

/* wait for an error operation; the provided arg will be the op that caused the error,
 i.e args[1] of the ERROR op */
bool ClientConnection::waitForError(int refNo)
{
    if (refNo == -1) {
        verbose(std::cout << "Waiting for error op" << std::endl;);
    } else {
        verbose(std::cout << "Waiting for error op with refno "
                          << refNo << std::endl;);
    }
    OperationDeque::iterator I = checkQueue("error", refNo);
    int remainingTime = timeOut;
    struct timeval tm, initialTm;
    
    ::gettimeofday(&initialTm, NULL);
    
    while ((I == operationQueue.end()) && (remainingTime > 0)) {
        poll(remainingTime);
        I = checkQueue("error", refNo);
        
        ::gettimeofday(&tm, NULL);
        int elapsed = tm.tv_sec - initialTm.tv_sec;
        remainingTime = timeOut - elapsed;
    }
    
    if (I == operationQueue.end()) {
        return true;    // we failed miserably
    }
    
    bool ret = false;
    RootOperation erOp = *I;
    const std::vector<Root> & erArgs = erOp->getArgs();
    if (erArgs.size() != 2) {
        std::cerr << "Error operation does not have 2 args"
                  << std::endl << std::flush;
        ret = true;
    } else {
        // std::cout << "ERROR arg class is " << erOp->getArgs().front()->getClassNo() << std::endl << std::flush;
        const Root & arg1 = erArgs[0];
        Element message;
        if (arg1->copyAttr("message", message) != 0) {
            std::cerr << "Error operation's first arg does not contain a message string"
                      << std::endl << std::flush;
            ret = true;
        }
        const Root & arg2 = erArgs[1];
        if (!arg2->hasAttrFlag(Atlas::Objects::OBJTYPE_FLAG)) {
            std::cerr << "Error operation's second arg does not have an objtype"
                      << std::endl << std::flush;
            ret = true;
        } else if (arg2->getObjtype() != "op") {
            std::cerr << "Error operation's second arg does not have objtype=\"op\""
                      << std::endl << std::flush;
            ret = true;
        }
        Element serialno;
        if (arg2->copyAttr("serialno", serialno) != 0 || !serialno.isInt()) {
            std::cerr << "Error operation's second arg does not have an objtype"
                      << std::endl << std::flush;
            ret = true;
        } else if (serialno.asInt() != refNo) {
            std::cerr << "Error operation's second arg does not have the serialno we sent "
                      << serialno.asInt() << " " << refNo
                      << std::endl << std::flush;
            ret = true;
        }
    }
    operationQueue.erase(I);

    return ret;
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
        if ((refno > 0) && ((*I)->getRefno() != refno))
            continue;
        
        const std::list<std::string> & parents = (*I)->getParents();
        if (parents.empty()) {
            continue;   // isn't this very bad? Yes
        }
        
        if (opType == parents.front()) {
            return I;
        }
    }
    
    return I;
}

int ClientConnection::send(Atlas::Objects::Operation::RootOperation op)
{
    op->setSerialno(++serialNo);
    encoder->streamObjectsMessage(op);
    ios << std::flush;
    return serialNo;
}

void ClientConnection::sendOther(Atlas::Objects::Root msg)
{
    encoder->streamObjectsMessage(msg);
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
        codec->poll();
        return true;
    }
    return false;

}

RootOperation ClientConnection::pop()
{
    poll(0);
    if (operationQueue.empty()) {
        return NULL;
    }
    RootOperation op = operationQueue.front();
    operationQueue.pop_front();
    return op;
}

bool ClientConnection::pending()
{
    return !operationQueue.empty();
}

int ClientConnection::addSpawnData(const Atlas::Objects::Entity::RootEntity & ent)
{
    if (m_spawns.empty()) {
        return -1;
    }
    const Element & spawn = m_spawns.front();
    if (!spawn.isMap()) {
        std::cerr << "ERROR: Account has a non-map spawn"
                  << std::endl << std::flush;
        return -1;
    }
    const MapType & spawndata = spawn.Map();
    MapType::const_iterator Iend = spawndata.end();
    MapType::const_iterator I = spawndata.find("name");
    if (I == Iend || !I->second.isString()) {
        std::cerr << "ERROR: Account has spawn with malformed name"
                  << std::endl << std::flush;
        return -1;
    }
    ent->setAttr("spawn_name", I->second);
    I = spawndata.find("character_types");
    if (I == Iend || !I->second.isList()) {
        std::cerr << "ERROR: Account has spawn with malformed character_types"
                  << std::endl << std::flush;
        return -1;
    }
    const ListType & character_types = I->second.List();
    if (character_types.empty()) {
        std::cerr << "WARNING: Account has spawn with empty character_types"
                  << std::endl << std::flush;
        return -1;
    }
    if (!character_types.front().isString()) {
        std::cerr << "ERROR: Account has spawn with bad first character_type"
                  << std::endl << std::flush;
        return -1;
    }
    ent->setParents(std::list<std::string>(1, character_types.front().String()));
    std::cout << "Got spawned!" << std::endl << std::flush;
    return 0;
}

void ClientConnection::push(const Atlas::Objects::Root & op)
{
    reply_flag = true;
    RootOperation new_op = Atlas::Objects::smart_dynamic_cast<Atlas::Objects::Operation::RootOperation>(op); 
    if (!new_op.isValid()) {
        verbose( std::cout << "Recived message which is not an op"
                           << std::endl << std::flush; );
        return;
    }
    operationQueue.push_back(new_op);
    const std::string & opP = op->getParents().front();
    int refNo = new_op->getRefno();
    verbose( std::cout << "Received op of type " << opP << " with refno "
                       << refNo << std::endl << std::flush;);
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
            return NULL;        // no possible matches, we==boned

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
