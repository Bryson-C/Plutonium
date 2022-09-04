//
// Created by Owner on 8/12/2022.
//

#ifndef PLUTONIUM_WRENSCRIPT_H
#define PLUTONIUM_WRENSCRIPT_H

#include <stdio.h>
#include <stdlib.h>

#include <windows.h>


#include "wren.h"



inline void writeFn(WrenVM* vm, const char* text) {
    printf("\t%s", text);
}

// Stolen Code:
inline void errorFn(WrenVM* vm, WrenErrorType errorType, const char* module, const int line, const char* msg) {
    switch (errorType) {
        case WREN_ERROR_COMPILE: { printf("[%s line %d] [Error] %s\n", module, line, msg); } break;
        case WREN_ERROR_STACK_TRACE: { printf("[%s line %d] in %s\n", module, line, msg); } break;
        case WREN_ERROR_RUNTIME: { printf("[Runtime Error] %s\n", msg); } break;
    }
}

const char* GetVariableFromWren(WrenVM* vm, const char* module, const char* varName) {
    // Make Sure Wren Can Interact With The C Code
    // This Gives 1 Slot
    wrenEnsureSlots(vm, 1);
    // Check If Enough Slots Are Available
    if (wrenGetSlotCount(vm) <= 0)
        fprintf(stderr, "Not Enough Slots (Slots: %i)\n", wrenGetSlotCount(vm));

    wrenGetVariable(vm, module, varName, 0);

    return wrenGetSlotString(vm, 0);
}

void MathSin(WrenVM* vm) {
    double x = wrenGetSlotDouble(vm, 1);
    wrenSetSlotDouble(vm, 0, sin(x));
}
void MathAdd(WrenVM* vm) {
    double a = wrenGetSlotDouble(vm, 1);
    double b = wrenGetSlotDouble(vm, 2);
    wrenSetSlotDouble(vm, 0, a + b);
}

void TimeNow(WrenVM* vm) {
    wrenSetSlotDouble(vm, 0, clock());
}

WrenForeignMethodFn bindForeignMethodFn(WrenVM* vm, const char* module, const char* className, bool isStatic, const char* signature) {
    printf("Binding: %s from %s in module: %s\n", signature, className, module);
    if (strcmp(module, "main") == 0) {
        if (strcmp(className, "Math") == 0) {
            if (isStatic) {
                if (strcmp(signature, "sin(_)") == 0) {
                    return MathSin;
                } else if (strcmp(signature, "add(_,_)") == 0) {
                    return MathAdd;
                }
            }
        } else {
            if (strcmp(className, "Time") == 0) {
                if (isStatic) {
                    if (strcmp(signature, "now")) {
                        return TimeNow;
                    }
                }
            }
        }
    }
}

const char* getFileContent(const char* path, size_t* size) {
    FILE* file;
    fopen_s(&file, path, "rb");
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);
    char* buffer = malloc(sizeof(char) * fileSize + 1);
    fread_s(buffer, fileSize, sizeof(char), fileSize, file);
    buffer[fileSize] = '\0';
    fclose(file);
    if (size != NULL)
        *size = fileSize;
    return buffer;
}

static int RedoWrenLoop;
void CloseWrenInstance() {
    RedoWrenLoop = 0;
}

int OpenWrenInstance(const char* path) {
    RedoWrenLoop = 1;

    // Load The File As A String
    const char* buffer = getFileContent(path, NULL);


    // Setup Wren
    const char* module = "main";
    WrenConfiguration config;
    wrenInitConfiguration(&config);

    config.writeFn = &writeFn;
    config.errorFn = &errorFn;
    config.bindForeignMethodFn = bindForeignMethodFn;

    // Create The Wren Instance
    WrenVM* vm = wrenNewVM(&config);


    // Use The Code Given In The File
    WrenInterpretResult result = wrenInterpret(vm, module, buffer);

    //const char* variable = GetVariableFromWren(vm, module, "hw");
    //printf("Variable hw: %s\n", variable);




    // Check For Errors
    switch (result) {
        case WREN_RESULT_COMPILE_ERROR: { printf("Compile Error!\n"); } break;
        case WREN_RESULT_RUNTIME_ERROR: { printf("Runtime Error!\n"); } break;
        case WREN_RESULT_SUCCESS:       {} break;
    }

    // Free The Wren Instance
    wrenFreeVM(vm);

    clock_t startTime = clock();
    while (RedoWrenLoop) {
        if (clock() - startTime > 200) {
            startTime = clock();
            const char* newBuffer = getFileContent(path, NULL);
            if (strcmp(buffer, newBuffer) != 0) {
                free(buffer);
                OpenWrenInstance(path);
            }
        }
    }

    return 0;
}


HANDLE NewWrenScriptThread(const char* path) {
#ifdef WIN32
    HANDLE ScriptThread = CreateThread(NULL, 0, OpenWrenInstance, path, 0 , NULL);
    return ScriptThread;
#endif
}
BOOL CloseWrenScriptThread(HANDLE thread) {
#ifdef WIN32
    CloseWrenInstance();
    BOOL exitThreadResult = TerminateThread(thread, 0);
    const char* error = GetLastError();
    printf("%s\n", (error != NULL) ? error : "Exited Safely!");
    return exitThreadResult;
#endif
    return TRUE;
}


#endif //PLUTONIUM_WRENSCRIPT_H
