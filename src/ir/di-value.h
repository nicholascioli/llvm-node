#ifndef __LLVM_NODE_DI_VALUE__
#define __LLVM_NODE_DI_VALUE__

#include <nan.h>
#include <llvm/IR/DIBuilder.h>
#include "../util/from-value-mixin.h"

template <class T>
class DIValueWrapper: public Nan::ObjectWrap, public FromValueMixin<DIValueWrapper<T>> {
public:
	static std::string name;

	static NAN_MODULE_INIT(Init);
	static bool isInstance(v8::Local<v8::Value> value);
	T* getDIValue();

	static v8::Local<v8::Object> of(T* diValue);

private:
	static Nan::Persistent<v8::FunctionTemplate> functionTemplate;
	T* diValue;

	explicit DIValueWrapper(T* diValue): diValue { diValue } {
	}

	static NAN_METHOD(New);

	// Rename
	static void setName(const char* name);
};

// Template functions
#include "di-value.ipp"

#endif // __LLVM_NODE_DI_TEMPLATE__