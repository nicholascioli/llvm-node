//
// Created by Micha Reiser on 13.03.17.
//

#include "struct-type.h"
#include "../util/string.h"

NAN_MODULE_INIT(StructTypeWrapper::Init) {
    auto object = Nan::New<v8::Object>();

    Nan::SetMethod(object, "get", StructTypeWrapper::get);

    Nan::Set(target, Nan::New("StructType").ToLocalChecked(), object);
}

v8::Local<v8::Object> StructTypeWrapper::of(llvm::StructType *type) {
    auto constructorFunction = Nan::GetFunction(Nan::New(structTypeTemplate())).ToLocalChecked();
    v8::Local<v8::Value> args[1] = { Nan::New<v8::External>(type) };
    auto instance = Nan::NewInstance(constructorFunction, 1, args).ToLocalChecked();

    Nan::EscapableHandleScope escapeScope {};
    return escapeScope.Escape(instance);
}

bool StructTypeWrapper::isInstance(v8::Local<v8::Value> value) {
    return Nan::New(structTypeTemplate())->HasInstance(value);
}

llvm::StructType* StructTypeWrapper::getStructType() {
    return static_cast<llvm::StructType*>(getType());
}

NAN_METHOD(StructTypeWrapper::New) {
    if (!info.IsConstructCall()) {
        return Nan::ThrowTypeError("The StructType constructor needs to be called with new");
    }

    if (info.Length() != 1 || !info[0]->IsExternal()) {
        return Nan::ThrowTypeError("The StructType constructor needs to be called with: structType: External");
    }

    auto* structType = static_cast<llvm::StructType*>(v8::External::Cast(*info[0])->Value());
    auto* wrapper = new StructTypeWrapper { structType };
    wrapper->Wrap(info.This());

    info.GetReturnValue().Set(info.This());
}

NAN_METHOD(StructTypeWrapper::get) {
    if (info.Length() < 2 || !LLVMContextWrapper::isInstance(info[0]) || !info[1]->IsArray() ||
            (info.Length() == 3 && !info[2]->IsBoolean()) ||
            info.Length() > 3) {
        return Nan::ThrowTypeError("get needs to be called with: context: LLVMContext, elements: Type[], isPacked?: boolean");
    }

    auto& context = LLVMContextWrapper::FromValue(info[0])->getContext();
    auto array = v8::Array::Cast(*info[1]);
    std::vector<llvm::Type*> elements { array->Length() };

    for (size_t i = 0; i < array->Length(); ++i) {
        auto element = array->Get(i);

        if (!TypeWrapper::isInstance(element)) {
            return Nan::ThrowTypeError("expected Type");
        }

        elements[i] = TypeWrapper::FromValue(element)->getType();
    }

    bool isPacked = false;
    if (info.Length() == 3) {
        isPacked = Nan::To<bool>(info[2]).ToChecked();
    }

    auto* result = llvm::StructType::get(context, elements, isPacked);

    info.GetReturnValue().Set(StructTypeWrapper::of(result));
}

NAN_GETTER(StructTypeWrapper::getName) {
    auto* structType = StructTypeWrapper::FromValue(info.Holder())->getStructType();
    if (structType->isLiteral()) {
        info.GetReturnValue().Set(Nan::Undefined()); // Struct Types never have names;
    } else {
        std::string name = structType->getName().str();
        info.GetReturnValue().Set(Nan::New(name).ToLocalChecked());
    }
}

NAN_SETTER(StructTypeWrapper::setName) {
    if (!value->IsString()) {
        return Nan::ThrowTypeError("name needs to be a string");
    }

    auto* structType = StructTypeWrapper::FromValue(info.Holder())->getStructType();
    structType->setName(ToString(value));
}

NAN_GETTER(StructTypeWrapper::getNumElements) {
    auto* structType = StructTypeWrapper::FromValue(info.Holder())->getStructType();
    info.GetReturnValue().Set(Nan::New(structType->getNumElements()));
}

NAN_METHOD(StructTypeWrapper::getElementType) {
    if (info.Length() != 1 || !info[0]->IsUint32()) {
        return Nan::ThrowTypeError("getElementType needs to be called with: index: uint32");
    }

    auto index = Nan::To<uint32_t>(info[0]).FromJust();
    auto* type = StructTypeWrapper::FromValue(info.Holder())->getStructType()->getElementType(index);

    info.GetReturnValue().Set(TypeWrapper::of(type));
}

Nan::Persistent<v8::FunctionTemplate>& StructTypeWrapper::structTypeTemplate() {
    static Nan::Persistent<v8::FunctionTemplate> functionTemplate {};

    if (functionTemplate.IsEmpty()) {
        v8::Local<v8::FunctionTemplate> localTemplate = Nan::New<v8::FunctionTemplate>(StructTypeWrapper::New);
        localTemplate->SetClassName(Nan::New("StructType").ToLocalChecked());
        localTemplate->InstanceTemplate()->SetInternalFieldCount(1);
        localTemplate->Inherit(Nan::New(typeTemplate()));

        Nan::SetAccessor(localTemplate->InstanceTemplate(), Nan::New("name").ToLocalChecked(), StructTypeWrapper::getName, StructTypeWrapper::setName);
        Nan::SetAccessor(localTemplate->InstanceTemplate(), Nan::New("numElements").ToLocalChecked(), StructTypeWrapper::getNumElements);
        Nan::SetPrototypeMethod(localTemplate, "getElementType", StructTypeWrapper::getElementType);

        functionTemplate.Reset(localTemplate);
    }

    return functionTemplate;
}