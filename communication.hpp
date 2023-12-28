#pragma once

enum class Task : char
{
    ADD         = 'a',
    SUSPEND     = 'S', 
    RESUME      = 'R',
    REMOVE      = 'r',
    INFO        = 'i',
    LIST        = 'l',
    PRINT       = 'p',
    TERMINATE   = 'k',      // TODO: ramane asa?

    DEFAULT     = '?'
};

const Task getTask(const char& ch)
{
    switch (ch)
    {
        case 'a':     return Task::ADD;       break;
        case 'S':     return Task::SUSPEND;   break;
        case 'R':     return Task::RESUME;    break;
        case 'r':     return Task::REMOVE;    break;
        case 'i':     return Task::INFO;      break;
        case 'l':     return Task::LIST;      break;
        case 'p':     return Task::PRINT;     break;
        case 'k':     return Task::TERMINATE; break;

        default:      return Task::DEFAULT;   break;
    }
}

/*
    Task task = Task::ADD;
    static_cast<char>(task) => return char
*/ 

/*
    TODO:
    nume pt memoria partajata
*/

