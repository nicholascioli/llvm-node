#ifndef LLVM_NODE_DI_BUILDER_H
#define LLVM_NODE_DI_BUILDER_H

#include <nan.h>
#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/Module.h>
#include <nan_callbacks_12_inl.h>
#include "../util/from-value-mixin.h"
#include "value.h"
#include "../util/string.h"

#include "di-value.h"

// Helper macro for seeing if it is a scope
#define IS_SCOPE(X) (DILexicalBlockWrapper::isInstance(X) || DIScopeWrapper::isInstance(X) || DIFileWrapper::isInstance(X) || DISubprogramWrapper::isInstance(X) || DICompileUnitWrapper::isInstance(X))

// Method to initialize all of the DI components
NAN_MODULE_INIT(InitDebug);

class DIBuilderWrapper: public Nan::ObjectWrap, public FromValueMixin<DIBuilderWrapper> {
public:
	static NAN_MODULE_INIT(Init);
	llvm::DIBuilder& getDIBuilder();

private:
	llvm::DIBuilder diBuilder;

	explicit DIBuilderWrapper(llvm::Module* mod, bool allowUnresolved = true, llvm::DICompileUnit* cu = nullptr) : diBuilder(*mod, allowUnresolved, cu) {
	}

	// static
	static NAN_METHOD(New);

	// instance
	static NAN_METHOD(CreateAutoVariable);
	static NAN_METHOD(CreateBasicType);
	static NAN_METHOD(CreateCompileUnit);
	static NAN_METHOD(CreateFile);
	static NAN_METHOD(CreateFunction);
	static NAN_METHOD(CreateLexicalBlock);
	static NAN_METHOD(CreatePointerType);
	static NAN_METHOD(CreateSubroutineType);
	static NAN_METHOD(Finalize);
	static NAN_METHOD(InsertDeclare);
	static NAN_METHOD(InsertDbgValueIntrinsic);
	// static NAN_METHOD(SetCurrentDebugLocation); // FIXME: Is this in IR?

	static inline Nan::Persistent<v8::Function>& diBuilderConstructor() {
		static Nan::Persistent<v8::Function> constructor {};
		return constructor;
	}
};

// Various subclasses
using DIBasicTypeWrapper      = DIValueWrapper<llvm::DIBasicType>;
using DICompileUnitWrapper    = DIValueWrapper<llvm::DICompileUnit>;
using DIFileWrapper           = DIValueWrapper<llvm::DIFile>;
using DILexicalBlockWrapper   = DIValueWrapper<llvm::DILexicalBlock>;
using DILocalVariableWrapper  = DIValueWrapper<llvm::DILocalVariable>;
using DIScopeWrapper          = DIValueWrapper<llvm::DIScope>;
using DISubprogramWrapper     = DIValueWrapper<llvm::DISubprogram>;
using DISubroutineTypeWrapper = DIValueWrapper<llvm::DISubroutineType>;
using DITypeWrapper           = DIValueWrapper<llvm::DIType>;

#endif // LLVM_NODE_DI_BUILDER_H