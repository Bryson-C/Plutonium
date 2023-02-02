//
// Created by Owner on 1/29/2023.
//

#ifndef PLUTONIUM_SCRIPTCORE_HPP
#define PLUTONIUM_SCRIPTCORE_HPP

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


typedef enum {
    SCRIPTCMD_NONE,
    SCRIPTCMD_PRINT,
} ScriptCommandType;
typedef enum {
    SCRIPTTYPE_INT,
    SCRIPTTYPE_STR,
    SCRIPTTYPE_FLT,
} ScriptCommandValueType;
typedef union {
    int32_t integer;
    char string[32];
    float decimal;
} ScriptCommandValue;
typedef struct {
    ScriptCommandType commandType;
    ScriptCommandValue value;
    ScriptCommandValueType valueType;
} ScriptCommand;

inline ScriptCommand NewScriptCommand(ScriptCommandType type, ScriptCommandValue value, ScriptCommandValueType valueType) {
    ScriptCommand command;
    command.commandType = type;
    command.value = value;
    command.valueType = valueType;
    return command;
}

typedef struct {
    const char* path;
} ScriptExecutor;



/*const char** */
inline void parseScript(const char* path) {
    FILE* file;
    int32_t result = (int32_t)fopen_s(&file, path, "rb");
    fseek(file, 0, SEEK_END);
    int32_t fileLength = ftell(file);
    rewind(file);

    char* buffer = malloc(sizeof(char) * fileLength+1);
    fread_s(buffer, fileLength, sizeof(char), fileLength, file);
    buffer[fileLength] = '\0';

    printf("Event Script '%s':\n\n%s\n\n", path, buffer);
}


#endif //PLUTONIUM_SCRIPTCORE_HPP
