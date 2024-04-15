#pragma once
#include <Arduino.h>

#define MAX_LINE 100
#define MAX_COMMANDS 20
#define MAX_ARGS 20
#define MAX_MSG 200
#define CLI_ARG_COUNT   1002

// Create a type to point to a CLI command function.
typedef int (*pCmd)(int argc, char* argv[]);

class CLICommand {

    public:

    CLICommand(const char* name, pCmd fp, const char* description = NULL, const char* helpText = NULL);
    const char* name();
    const char* description();
    const char* helpText();
    int execute(int argc = 0, char* argv[] = NULL);

    private:

    const char* _name;
    pCmd _fp;
    const char* _description;
    const char* _helpText;

};

// Create a type to point to a CLI printf function
typedef void (*CLIprintf) (const char* fmt, va_list va);

class CLI {

    private:
    CLI() = default;
    CLIprintf cliPrintf;

    public:
    static CLI &getInstance();      // Accessor for singleton instance
    CLI(const CLI &) = delete;      // no copying
    CLI &operator = (const CLI &) = delete;
    boolean begin(const char* prompt = "cli> ", boolean promptFirst = false, CLIprintf fp = NULL);
    boolean add(const char* name, pCmd fp, const char* description = NULL, const char* helpText = NULL);
    int read();
    char* cmd(char* cmdLine);
    void printf(const char* fmt, ...);

    private:

    CLICommand* commands[MAX_COMMANDS];
    int commandCount;
    char line[MAX_LINE];            // command line
    char lastLine[MAX_LINE];        // one line of history
    int cp;
    int argc = 0;
    char* argv[MAX_ARGS];
    const char* _prompt;
    char msgBuf[MAX_MSG];

    int findCmd(char* name);
    void cmdHelp(char* name);
    void showHelp();
    boolean prompt();
    int handleLine();
    int parseLine();

};

extern CLI &cli;
