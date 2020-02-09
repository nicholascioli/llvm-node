//
// Created by Micha Reiser on 24.03.17.
//

#include "constant-expr.h"
#include "llvm-context.h"
#include "type.h"

#include "../util/string.h"

#include <iostream>


NAN_MODULE_INIT(ConstantExprWrapper::Init) {
        auto constantExpr = Nan::GetFunction(Nan::New(constantExprTemplate())).ToLocalChecked();
        Nan::Set(target, Nan::New("ConstantExpr").ToLocalChecked(), constantExpr);
}

NAN_METHOD(ConstantExprWrapper::New) {
        if (!info.IsConstructCall()) {
            return Nan::ThrowTypeError("Class Constructor ConstantExpr cannot be invoked without new");
        }

        if (info.Length() != 1 || !info[0]->IsExternal()) {
            return Nan::ThrowTypeError("ConstantExpr constructor needs to be called with: constantExpr: external");
        }

        auto* constantExpr = static_cast<llvm::ConstantExpr*>(v8::External::Cast(*info[0])->Value());
        auto* wrapper = new ConstantExprWrapper { constantExpr };
        wrapper->Wrap(info.This());

        info.GetReturnValue().Set(info.This());
}

NAN_METHOD(ConstantExprWrapper::getBitCast) {
    if (info.Length() != 2 || !ConstantWrapper::isInstance(info[0]) || !TypeWrapper::isInstance(info[1])) {
		return Nan::ThrowTypeError("get called with illegal arguments");
    }

	auto cons = ConstantWrapper::FromValue(info[0])->getConstant();
	auto type = TypeWrapper::FromValue(info[1])->getType();

	auto as = llvm::ConstantExpr::getBitCast(cons, type);

	return info.GetReturnValue().Set(ConstantWrapper::of(as));
}

NAN_METHOD(ConstantExprWrapper::getInBoundsGetElementPtr) {
    if (info.Length() != 3 || !TypeWrapper::isInstance(info[0]) || !ConstantWrapper::isInstance(info[1]) || !info[2]->IsArray()) {
		return Nan::ThrowTypeError("getInBoundsGetElementPtr called with illegal arguments");
    }

	auto type = TypeWrapper::FromValue(info[0])->getType();
	auto cons = ConstantWrapper::FromValue(info[1])->getConstant();

	v8::Handle<v8::Array> array = v8::Handle<v8::Array>::Cast(info[2]);

	// Check that all elements in the array are correct
	std::vector<llvm::Constant*> args(array->Length());
	for (auto i = 0u; i != array->Length(); ++i) {
		if (!ConstantWrapper::isInstance(array->Get(i))) {
			return Nan::ThrowTypeError("getInBoundsGetElementPtr requires all of the elements of paramTypes to be constants");
		}

		args[i] = ConstantWrapper::FromValue(array->Get(i))->getConstant();
	}

	auto as = llvm::ConstantExpr::getInBoundsGetElementPtr(type, cons, llvm::ArrayRef<llvm::Constant*>(args));

	return info.GetReturnValue().Set(ConstantWrapper::of(as));
}

llvm::ConstantExpr *ConstantExprWrapper::getConstantExpr() {
    return static_cast<llvm::ConstantExpr*>(getValue());
}

v8::Local<v8::Object> ConstantExprWrapper::of(llvm::ConstantExpr *constantExpr) {
    auto constructorFunction = Nan::GetFunction(Nan::New(constantExprTemplate())).ToLocalChecked();
    v8::Local<v8::Value> args[1] = { Nan::New<v8::External>(constantExpr) };
    auto instance = Nan::NewInstance(constructorFunction, 1, args).ToLocalChecked();

    Nan::EscapableHandleScope escapeScpoe {};
    return escapeScpoe.Escape(instance);
}

Nan::Persistent<v8::FunctionTemplate>& ConstantExprWrapper::constantExprTemplate() {
    static Nan::Persistent<v8::FunctionTemplate> functionTemplate {};

    if (functionTemplate.IsEmpty()) {
        auto localTemplate = Nan::New<v8::FunctionTemplate>(ConstantExprWrapper::New);
        localTemplate->SetClassName(Nan::New("ConstantExpr").ToLocalChecked());
        localTemplate->InstanceTemplate()->SetInternalFieldCount(1);
        localTemplate->Inherit(Nan::New(constantTemplate()));

        Nan::SetMethod(localTemplate, "getBitCast", ConstantExprWrapper::getBitCast);
        Nan::SetMethod(localTemplate, "getInBoundsGetElementPtr", ConstantExprWrapper::getInBoundsGetElementPtr);

        functionTemplate.Reset(localTemplate);
    }

    return functionTemplate;
}
