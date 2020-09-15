#include "ipc_client.h"

#define DOCTEST_CONFIG_IMPLEMENT
#define DOCTEST_CONFIG_NO_UNPREFIXED_OPTIONS
#include "../doctest/doctest.h"

#include "utils/logger.h"
#include "utils/stopwatch.h"

#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char** argv) {
    doctest::Context context;
    context.applyCommandLine(argc, argv);

    Stopwatch totalTime;

    /******************************************************************
                          Parse command line
    ******************************************************************/

    // read non-optionals
    string problemFileName = string(argv[1]);

    // init optionals to default values
    string hostName = "localhost";
    unsigned short port = 2323;
    string parserOptions = "";

    bool allParamsRead = false;
    string plannerDesc;
    for (unsigned int i = 2; i < argc; ++i) {
        if (!allParamsRead && string(argv[i])[0] == '[') {
            allParamsRead = true;
        }
        if (allParamsRead) {
            plannerDesc += string(argv[i]) + " ";
        } else {
            string nextOption = string(argv[i]);
            if (nextOption == "-h" || nextOption == "--hostname") {
                hostName = string(argv[++i]);
            } else if (nextOption == "-p" || nextOption == "--port") {
                port = (unsigned short)(atoi(string(argv[++i]).c_str()));
            } else if (nextOption == "--parser-options") {
                parserOptions = string(argv[++i]);
            } else {
                cerr << "Unknown option: " << nextOption << endl;
                return 1;
            }
        }
    }

    // Create connector to rddlsim and run
    IPCClient* client = new IPCClient(hostName, port, parserOptions);
    client->run(problemFileName, plannerDesc);

    Logger::logLine(
        "PROST complete running time: " + to_string(totalTime()),
        Verbosity::SILENT);
    return 0;
}
