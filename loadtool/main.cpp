#include <cstdio>
#include "User.h"
#include "Scheduler.h"
#include <Eris/PollDefault.h>
#include <unistd.h>
#include <vector>
#include <signal.h>
#include <sstream>
#include <iostream>
#include <Eris/Log.h>

using std::cout;
using std::endl;

bool global_quit = false;

/** signal handling code borrowed from cyphesis */
static void signal_received(int signo)
{
    global_quit = true;
    
    struct sigaction action;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    action.sa_handler = SIG_IGN;
    sigaction(signo, &action, NULL);
}

static void initSignals()
{
    struct sigaction action;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    action.sa_handler = signal_received;
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
	sigaction(SIGQUIT, &action, NULL);

    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    action.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &action, NULL);
}

int main(int argc, char* argv[])
{
    int numUsers = 4;
    short svPort = 6767;
    std::string svHost = "localhost";

    std::vector<User*> users;
    
    initSignals();
    
    while (numUsers-- > 0) {
        std::ostringstream os;
        os << "fuckwad_user_" << numUsers;
        User* u = new User(os.str(), svHost, svPort);
        users.push_back(u);
    }

    while (!global_quit && !users.empty()) {
        usleep(50 * 1000);
        Eris::PollDefault::poll();
        Scheduler::instance()->update();
        
        for (unsigned int i=0; i < users.size(); ) {
            if (users[i]->failed()) {
                delete users[i];
                users.erase(users.begin() + i);
                std::cerr << "deleting failed user" << endl;
            } else
                ++i;
        }
    }

    cout << "shutting down" << endl;
    for (unsigned int it=0; it < users.size(); ++it)
        delete users[it];

    return EXIT_SUCCESS;
}
