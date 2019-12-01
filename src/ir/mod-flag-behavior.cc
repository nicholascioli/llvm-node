//
// Created by Micha Reiser on 01.03.17.
//

#include "mod-flag-behavior.h"
#include <llvm/IR/Module.h>

NAN_MODULE_INIT(InitModFlagBehaviorTypes) {
    auto object = Nan::New<v8::Object>();

    Nan::Set(object, Nan::New("Error").ToLocalChecked(), Nan::New(llvm::Module::ModFlagBehavior::Error));
    Nan::Set(object, Nan::New("Warning").ToLocalChecked(), Nan::New(llvm::Module::ModFlagBehavior::Warning));
    Nan::Set(object, Nan::New("Require").ToLocalChecked(), Nan::New(llvm::Module::ModFlagBehavior::Require));
    Nan::Set(object, Nan::New("Override").ToLocalChecked(), Nan::New(llvm::Module::ModFlagBehavior::Override));
    Nan::Set(object, Nan::New("Append").ToLocalChecked(), Nan::New(llvm::Module::ModFlagBehavior::Append));
    Nan::Set(object, Nan::New("AppendUnique").ToLocalChecked(), Nan::New(llvm::Module::ModFlagBehavior::AppendUnique));
    Nan::Set(object, Nan::New("Max").ToLocalChecked(), Nan::New(llvm::Module::ModFlagBehavior::Max));


    Nan::Set(target, Nan::New("ModFlagBehavior").ToLocalChecked(), object);
}