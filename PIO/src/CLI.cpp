/*
 * CLI.cpp - command line parser for Arduino
 * add application-specific commands at runtime in setup
 * runs interactively in conjunction with Arduino loop()
 */
#include <Arduino.h>
#include <string.h>
#include "CLI.hpp"

#define CLI_BAD_COMMAND 1001

// CLICommand class
const char* _name;
pCmd _fp;
CLICommand::CLICommand(const char* name, pCmd fp, const char* description, const char* helpText){
    _name = name;
    _fp = fp;
    _description = description;
    _helpText = helpText;

}

const char* CLICommand::name(){
    return _name;
}

const char* CLICommand::description(){
    return _description;
}

const char* CLICommand::helpText(){
    return _helpText;
}


int CLICommand::execute(int argc, char* argv[]) {
    return _fp(argc, argv);
}

// CLI class
CLICommand* commands[MAX_COMMANDS];

int commandCount = 0;
char line[MAX_LINE];
char lastLine[MAX_LINE];
int cp = 0;
int argc = 0;
char* argv[MAX_ARGS];
const char* _prompt = "cli> ";

void _CLIdefaultprintf(const char* fmt, va_list va) {
    char msgBuf[MAX_MSG];
    vsprintf (msgBuf, fmt, va);
    Serial.println(msgBuf);
}
CLIprintf cliPrintf;

void CLI::printf(const char* fmt, ...) {
    if(cliPrintf == NULL)
        cliPrintf = _CLIdefaultprintf;
    va_list va;
    va_start (va, fmt);
    cliPrintf (fmt, va);
    va_end (va);
}

boolean CLI::add(const char* name, pCmd fp, const char* description, const char* helpText) {
    if(commandCount < MAX_COMMANDS) {
        commands[commandCount++] = new CLICommand(name, fp, description, helpText);
        return true;
    }
    return false;
}

boolean CLI::begin(const char* prompt, boolean promptFirst, CLIprintf fp) {
    _prompt = prompt;
    if(fp != NULL) {
        cliPrintf = fp;
    } else {
        cliPrintf = _CLIdefaultprintf;
    }
    if(Serial) {
        if(promptFirst)
            this->prompt();
        return true;
    } else {
        return false;
    }
}

// ansi escape sequences?
enum Arrow {none, up, down, left, right};
Arrow decodeArrow() {
    if(Serial.read() == 91) {
        switch (Serial.read()) {
            case 65: // up
                return up;
            case 66: // down
                return down;
            case 67: // right
                return right;
            case 68: // left
                return left;
            default:
                return none;
        }
    }
    return none;
}

int CLI::read() {
    int n = 0;
    while(Serial.available() > 0) {
        if(cp < MAX_LINE) {
            int c = Serial.read();
            n++;
            switch(c) {
                case 27:                    // esc
                    switch(decodeArrow()) {
                        case up:
                            Serial.write(13);
                            strcpy(line, lastLine);
                            prompt();
                            Serial.print(line);
                            cp = strlen(line);
                            break;
                        case down:
                            line[0] = '\0';
                            cp = 0;
                            Serial.println();
                            prompt();
                            break;
                        case right:
                            break;
                        case left:
                            break;
                        default:
                            break;
                    }
                    break;
                case 13:                    // carriage return
                    Serial.println();
                    line[cp] = '\0';
                    if(strlen(line) > 0) {
                        strcpy(lastLine, line);
                    }
                    handleLine();
                    cp = 0;
                    prompt();
                    break;
                case 127:                   // backspace
                    if(cp > 0) {
                        // clear character
                        Serial.write(8);                // clear
                        Serial.write(' ');              
                        Serial.write(8);                
                        // back up
                        cp = cp - 1;
                    }
                    break;
                default:
                    Serial.write(c);                // echo
                    line[cp++] = (char) c;
                    break;
            }
            Serial.flush();
        } else {
            Serial.println();
            Serial.println("ERROR: CLI buffer overflow");
            cp = 0;
            prompt();
        }
    }
    return n;
}

char* CLI::cmd(char* cmdLine) {
    strncpy(line, cmdLine, MAX_LINE);
    cp = 0;
    handleLine();
    return msgBuf;
}

int CLI::parseLine() {

    int toks = 0;
    char* tok;
    tok = strtok(line, " ");

    while(tok != NULL) {
        if(toks < MAX_ARGS) {
            argv[toks++] = tok;
            tok = strtok(NULL, " ");
        } else {
            Serial.println("ERROR: too many arguments");
            toks = -1;
            break;
        }
    }
    argc = toks;
    return toks;
}

int CLI::findCmd(char* name) {
    for(int i=0; i<commandCount; i++) {
        if(strcmp(name, commands[i]->name()) == 0)
            return i;
    }
    return -1;
}

void CLI::cmdHelp(char* name) {
    int i = findCmd(name);
    if(i == -1) {
        Serial.print("unknown command '");
        Serial.print(name);
        Serial.println("'");
    } else {
        Serial.print("command: ");
        Serial.println(commands[i]->name());
        if(commands[i]->description() != NULL)
            Serial.println(commands[i]->description());
        if(commands[i]->helpText() != NULL)
            Serial.println(commands[i]->helpText());
    }
}

void CLI::showHelp() {
    if(argc == 1) {
        // top-level help
        Serial.println("commands:");
        for(int i=0; i<commandCount; i++) {
            char msgBuf[100];
            if(commands[i]->description() != NULL) {
                sprintf(msgBuf, "%-20s - %-20s", commands[i]->name(), commands[i]->description());
            } else {
                sprintf(msgBuf, "%-20s", commands[i]->name());
            }
            Serial.println(msgBuf);
        }
        Serial.println("type 'help <cmd>' for more specifics");
    } else if(argc == 2) {
        cmdHelp(argv[1]);
    } else {
        Serial.print("ERROR: argc > 2: ");
        Serial.println(argc);
    }
}

// handle a single command
int CLI::handleLine() {
    // no message by default
    msgBuf[0] = '\0';

    // NOOP if empty command line
    if(strlen(line) == 0) {
        return 0;
    }
    parseLine();

    int stat = 0;
    // special handling for the built-in help command
    if(argc >= 1 && strcmp(argv[0], "help") == 0) {
        showHelp();

    // find and execute the command
    } else if(argc != -1) {
        int i = findCmd(argv[0]);
        if(i != -1) {
            stat = commands[i]->execute(argc, argv);
        } else {
            Serial.print("invalid command '");
            Serial.print(line);
            Serial.println("'");
            stat = CLI_BAD_COMMAND;
        }
    } else {
        stat = CLI_ARG_COUNT;
    }

    return stat;
}

boolean CLI::prompt() {
    if(Serial) {
        Serial.print(_prompt);
        Serial.flush();
        return true;
    }
    return false;
}

CLI &CLI::getInstance() {
    static CLI instance;
    return instance;
}

CLI &cli = CLI::getInstance();
