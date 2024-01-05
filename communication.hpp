#pragma once

enum class TaskType : char
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

const TaskType getTask(const char& ch)
{
    switch (ch)
    {
        case 'a':     return TaskType::ADD;       break;
        case 'S':     return TaskType::SUSPEND;   break;
        case 'R':     return TaskType::RESUME;    break;
        case 'r':     return TaskType::REMOVE;    break;
        case 'i':     return TaskType::INFO;      break;
        case 'l':     return TaskType::LIST;      break;
        case 'p':     return TaskType::PRINT;     break;
        case 'k':     return TaskType::TERMINATE; break;

        default:      return TaskType::DEFAULT;   break;
    }
}

