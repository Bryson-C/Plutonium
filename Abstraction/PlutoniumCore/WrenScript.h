//
// Created by Owner on 8/12/2022.
//

#ifndef PLUTONIUM_WRENSCRIPT_H
#define PLUTONIUM_WRENSCRIPT_H

#include <stdio.h>
#include <stdlib.h>

#include "wren.h"



inline void writeFn(WrenVM* vm, const char* text) {
    printf("%s\n", text);
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



int wrenMain(const char* path) {

    // Load The File As A String
    FILE* file;
    fopen_s(&file, path, "rb");
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);
    char* buffer = malloc(sizeof(char) * fileSize + 1);
    fread_s(buffer, fileSize, sizeof(char), fileSize, file);
    buffer[fileSize] = '\0';


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
        case WREN_RESULT_COMPILE_ERROR:
        { printf("Compile Error!\n"); } break;
        case WREN_RESULT_RUNTIME_ERROR:
        { printf("Runtime Error!\n"); } break;
        case WREN_RESULT_SUCCESS:
        { printf("Success!\n"); } break;
    }

    // Free The Wren Instance
    wrenFreeVM(vm);

    return 0;
}


#endif //PLUTONIUM_WRENSCRIPT_H
