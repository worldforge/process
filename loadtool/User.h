#ifndef LOADTOOL_USER_H
#define LOADTOOL_USER_H

#include <Eris/Connection.h>
#include <Eris/Account.h>
#include <sigc++/object.h>

class Character;

class User : public SigC::Object
{
public:
    User(const std::string& uid, const std::string& host, short port);
    ~User();
    
    bool failed() const
    { return m_state == FAILED; }
    
private:
    typedef enum
    {
        INVALID,
        CONNECTING,
        LOGGING_IN,
        CONNECTED,
        FAILED
    } State;

    void createCharacter();

    void onConnected();
    void onConnectionFail(const std::string& errMsg);
    
    void onLoggedIn();
    void onLoginFailure(const std::string&);
    void onCharCreated(Eris::Avatar* av);
    
    State m_state;
    std::string m_uid;
    
    Eris::Connection* m_con;
    Eris::Account* m_acc;

    typedef std::map<std::string, Character*> CharacterDict;
    CharacterDict m_chars;
    
    int m_charCount;
};

#endif
