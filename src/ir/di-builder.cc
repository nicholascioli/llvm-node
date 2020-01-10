#include "di-builder.h"
#include "basic-block.h"
#include "module.h"

#include <vector>

template<> std::string DIBasicTypeWrapper::name = "DIBasicType";
template<> std::string DICompileUnitWrapper::name = "DICompileUnit";
template<> std::string DICompositeTypeWrapper::name = "DICompositeType";
template<> std::string DIDerivedTypeWrapper::name = "DIDerivedType";
template<> std::string DIFileWrapper::name = "DIFile";
template<> std::string DILexicalBlockWrapper::name = "DILexicalBlock";
template<> std::string DILocalVariableWrapper::name = "DILocalVariable";
template<> std::string DIScopeWrapper::name = "DIScope";
template<> std::string DISubprogramWrapper::name = "DISubprogram";
template<> std::string DISubroutineTypeWrapper::name = "DISubroutineType";
template<> std::string DITypeWrapper::name = "DIType";

NAN_MODULE_INIT(InitDebug) {
	DIBasicTypeWrapper::Init(target);
	DIBuilderWrapper::Init(target);
	DICompileUnitWrapper::Init(target);
	DICompositeTypeWrapper::Init(target);
	DIDerivedTypeWrapper::Init(target);
	DIFileWrapper::Init(target);
	DILexicalBlockWrapper::Init(target);
	DILocalVariableWrapper::Init(target);
	DIScopeWrapper::Init(target);
	DISubprogramWrapper::Init(target);
	DISubroutineTypeWrapper::Init(target);
	DITypeWrapper::Init(target);
}

NAN_MODULE_INIT(DIBuilderWrapper::Init) {
	v8::Local<v8::FunctionTemplate> functionTemplate = Nan::New<v8::FunctionTemplate>(New);
	functionTemplate->SetClassName(Nan::New("DIBuilder").ToLocalChecked());
	functionTemplate->InstanceTemplate()->SetInternalFieldCount(1);

	Nan::SetPrototypeMethod(functionTemplate, "createAutoVariable", DIBuilderWrapper::CreateAutoVariable);
	Nan::SetPrototypeMethod(functionTemplate, "createBasicType", DIBuilderWrapper::CreateBasicType);
	Nan::SetPrototypeMethod(functionTemplate, "createCompileUnit", DIBuilderWrapper::CreateCompileUnit);
	Nan::SetPrototypeMethod(functionTemplate, "createFile", DIBuilderWrapper::CreateFile);
	Nan::SetPrototypeMethod(functionTemplate, "createFunction", DIBuilderWrapper::CreateFunction);
	Nan::SetPrototypeMethod(functionTemplate, "createLexicalBlock", DIBuilderWrapper::CreateLexicalBlock);
	Nan::SetPrototypeMethod(functionTemplate, "createParameterVariable", DIBuilderWrapper::CreateParameterVariable);
	Nan::SetPrototypeMethod(functionTemplate, "createPointerType", DIBuilderWrapper::CreatePointerType);
	Nan::SetPrototypeMethod(functionTemplate, "createStructType", DIBuilderWrapper::CreateStructType);
	Nan::SetPrototypeMethod(functionTemplate, "createSubroutineType", DIBuilderWrapper::CreateSubroutineType);
	Nan::SetPrototypeMethod(functionTemplate, "finalize", DIBuilderWrapper::Finalize);
	Nan::SetPrototypeMethod(functionTemplate, "insertDeclare", DIBuilderWrapper::InsertDeclare);
	Nan::SetPrototypeMethod(functionTemplate, "insertDbgValueIntrinsic", DIBuilderWrapper::InsertDbgValueIntrinsic);

	auto constructorFunction = Nan::GetFunction(functionTemplate).ToLocalChecked();
	diBuilderConstructor().Reset(constructorFunction);

	Nan::Set(target, Nan::New("DIBuilder").ToLocalChecked(), constructorFunction);
}

NAN_METHOD(DIBuilderWrapper::New) {
	if (!info.IsConstructCall()) {
		return Nan::ThrowTypeError("DIBuilder constructor needs to be called with new");
	}

	if (info.Length() < 1 || !ModuleWrapper::isInstance(info[0]) ||
			(info.Length() > 1 && !(info[1]->IsBoolean() || info[1]->IsUndefined())) ||
			(info.Length() > 2 && !(DICompileUnitWrapper::isInstance(info[2]) || info[2]->IsUndefined())) ||
			info.Length() > 3) {
		return Nan::ThrowTypeError("DIBuilder constructor needs to be called with module: LLVMModule, allowUnresolved?: boolean, and compileUnit?: DICompileUnit");
	}

	DIBuilderWrapper* wrapper = nullptr;
	auto* llvmModule = ModuleWrapper::FromValue(info[0])->getModule();
	bool allowUnresolved = true;
	llvm::DICompileUnit* cu = nullptr;

	if (info.Length() > 1 && !(info[1]->IsUndefined())) {
		allowUnresolved = info[1]->IsTrue();
	}
	if (info.Length() == 3 && !(info[2]->IsUndefined())) {
		cu = DICompileUnitWrapper::FromValue(info[2])->getDIValue();
	}

	wrapper = new DIBuilderWrapper(llvmModule, allowUnresolved, cu);

	wrapper->Wrap(info.This());
	info.GetReturnValue().Set(info.This());
}


NAN_METHOD(DIBuilderWrapper::CreateAutoVariable) {
	auto& builder = DIBuilderWrapper::FromValue(info.Holder())->diBuilder;

	if (info.Length() != 5 || !(
		IS_SCOPE(info[0]) &&
		info[1]->IsString() &&
		DIFileWrapper::isInstance(info[2]) &&
		info[3]->IsUint32() &&
		IS_TYPE(info[4])
	)) {
		return Nan::ThrowTypeError("createAutoVariable needs to be called with scope: DIScope, name: string, file: DIFile, lineNo: number, ty: DIType");
	}

	llvm::DIScope* scope = DIScopeWrapper::FromValue(info[0])->getDIValue();
	auto  name  = ToString(info[1]);
	auto* file  = DIFileWrapper::FromValue(info[2])->getDIValue();
	auto  line  = Nan::To<uint32_t>(info[3]).FromJust();
	auto* ty    = DITypeWrapper::FromValue(info[4])->getDIValue();

	auto var = builder.createAutoVariable(scope, name, file, line, ty, true);
	info.GetReturnValue().Set(DILocalVariableWrapper::of(var));
}

NAN_METHOD(DIBuilderWrapper::CreateBasicType) {
	auto& builder = DIBuilderWrapper::FromValue(info.Holder())->diBuilder;

	// here
	if (info.Length() < 1 ||
			(info.Length() == 3 && !(
				info[0]->IsString() &&
				info[1]->IsUint32() &&
				info[2]->IsUint32()
			)) ||
			info.Length() > 3) {
		return Nan::ThrowTypeError("createBasicType needs to be called with name: string, sizeInBits: number, and encoding: number");
	}

	std::string name = ToString(info[0]);
	auto sizeInBits  = Nan::To<uint32_t>(info[1]).FromJust();
	auto encoding    = Nan::To<uint32_t>(info[2]).FromJust();

	auto* type = builder.createBasicType(name, sizeInBits, encoding);
	info.GetReturnValue().Set(DIBasicTypeWrapper::of(type));
}

NAN_METHOD(DIBuilderWrapper::CreateCompileUnit) {
	auto& builder = DIBuilderWrapper::FromValue(info.Holder())->diBuilder;

	// here
	if (info.Length() < 1 ||
			(info.Length() == 6 && !(
				info[0]->IsUint32() &&
				DIFileWrapper::isInstance(info[1]) &&
				info[2]->IsString() &&
				info[3]->IsBoolean() &&
				info[4]->IsString() &&
				info[5]->IsUint32()
			)) ||
			info.Length() > 6) {
		return Nan::ThrowTypeError("createCompileUnit needs to be called with lang: LangTypes, file: DIFile, producer: string, isOptimized: false, flags: string, and RV: Number");
	}

	auto lang = Nan::To<uint32_t>(info[0]).FromJust();
	auto* file = DIFileWrapper::FromValue(info[1])->getDIValue();
	std::string producer = ToString(info[2]);
	auto isOptimized = Nan::To<bool>(info[3]).FromJust();
	std::string flags = ToString(info[4]);
	auto rv = Nan::To<uint32_t>(info[5]).FromJust();

	// Create the actual compilation unit
	auto* cu = builder.createCompileUnit(
		lang,
		file,
		producer,
		isOptimized,
		flags,
		rv
	);

	// TODO: Implement the below line
	info.GetReturnValue().Set(DICompileUnitWrapper::of(cu));
}

NAN_METHOD(DIBuilderWrapper::CreateFile) {
	auto& builder = DIBuilderWrapper::FromValue(info.Holder())->diBuilder;

	// FIXME: Add support for checksum and source
	if (info.Length() < 1 ||
			(info.Length() == 2 && !(info[0]->IsString() && info[1]->IsString())) ||
			info.Length() > 2) {
		return Nan::ThrowTypeError("CreateFile needs to be called with filename: string and directory: string");
	}

	std::string file = ToString(info[0]);
	std::string dir = ToString(info[1]);

	auto* diFile = builder.createFile(file, dir);
	info.GetReturnValue().Set(DIFileWrapper::of(diFile));
}

NAN_METHOD(DIBuilderWrapper::CreateFunction) {
	auto& builder = DIBuilderWrapper::FromValue(info.Holder())->diBuilder;

	// FIXME: Add support for checksum and source
	if (info.Length() < 1 ||
			(info.Length() == 7 && !(
				IS_SCOPE(info[0]) &&
				info[1]->IsString() &&
				info[2]->IsString() &&
				DIFileWrapper::isInstance(info[3]) &&
				info[4]->IsUint32() &&
				DISubroutineTypeWrapper::isInstance(info[5]) &&
				info[6]->IsUint32()
			)) ||
			info.Length() > 7) {
		return Nan::ThrowTypeError("CreateFunction needs to be called with scope: DIScope, name: string, linkageName: string, file: DIFile, lineNo: number, ty: DISubroutineType, and scopeLine: number");
	}

	llvm::DIScope* scope;
	if (DILexicalBlockWrapper::isInstance(info[0]))
		scope = DILexicalBlockWrapper::FromValue(info[0])->getDIValue();
	else if (DIScopeWrapper::isInstance(info[0]))
		scope = DIScopeWrapper::FromValue(info[0])->getDIValue();
	else if (DIFileWrapper::isInstance(info[0]))
		scope = (llvm::DIScope*) DIFileWrapper::FromValue(info[0])->getDIValue();
	else if (DISubprogramWrapper::isInstance(info[0]))
		scope = (llvm::DIScope*) DISubprogramWrapper::FromValue(info[0])->getDIValue();
	else
		scope = (llvm::DIScope*) DICompileUnitWrapper::FromValue(info[0])->getDIValue();

	std::string name = ToString(info[1]);
	std::string linkage = ToString(info[2]);
	auto* file = DIFileWrapper::FromValue(info[3])->getDIValue();
	auto line = Nan::To<uint32_t>(info[4]).FromJust();
	auto* type = DISubroutineTypeWrapper::FromValue(info[5])->getDIValue();
	auto scopeLine = Nan::To<uint32_t>(info[6]).FromJust();

	// FIXME: No idea what those last two flags are...
	auto* diSub = builder.createFunction(scope, name, linkage, file, line, type, scopeLine, llvm::DINode::FlagPrototyped, llvm::DISubprogram::SPFlagDefinition);

	// TODO: Implement line below
	info.GetReturnValue().Set(DISubprogramWrapper::of(diSub));
}

NAN_METHOD(DIBuilderWrapper::CreateLexicalBlock) {
	auto& builder = DIBuilderWrapper::FromValue(info.Holder())->diBuilder;

	// FIXME: Add support for checksum and source
	if (info.Length() < 1 ||
			(info.Length() == 4 && !(
				IS_SCOPE(info[0]) &&
				DIFileWrapper::isInstance(info[1]) &&
				info[2]->IsUint32() &&
				info[3]->IsUint32()
			)) ||
			info.Length() > 4) {
		return Nan::ThrowTypeError("CreateLexicalBlock needs to be called with scope: Scope, file: DIFile, line: number, and column: number");
	}

	auto* scope = DIScopeWrapper::FromValue(info[0])->getDIValue();
	auto* file = DIFileWrapper::FromValue(info[1])->getDIValue();
	auto line = Nan::To<uint32_t>(info[2]).FromJust();
	auto col = Nan::To<uint32_t>(info[3]).FromJust();

	info.GetReturnValue().Set(DILexicalBlockWrapper::of(builder.createLexicalBlock(scope, file, line, col)));
}

NAN_METHOD(DIBuilderWrapper::CreateParameterVariable) {
	auto& builder = DIBuilderWrapper::FromValue(info.Holder())->diBuilder;

	if (info.Length() != 6 || !(
		IS_SCOPE(info[0]) &&
		info[1]->IsString() &&
		info[2]->IsUint32() &&
		DIFileWrapper::isInstance(info[3]) &&
		info[4]->IsUint32() &&
		IS_TYPE(info[5])
	)) {
		return Nan::ThrowTypeError("createParameterVariable needs to be called with scope: DIScope, name: string, argPos: number, file: DIFile, lineNo: number, ty: DIType");
	}

	llvm::DIScope* scope = DIScopeWrapper::FromValue(info[0])->getDIValue();
	auto  name   = ToString(info[1]);
	auto  argPos = Nan::To<uint32_t>(info[2]).FromJust();
	auto* file   = DIFileWrapper::FromValue(info[3])->getDIValue();
	auto  line   = Nan::To<uint32_t>(info[4]).FromJust();
	auto* ty     = DITypeWrapper::FromValue(info[5])->getDIValue();

	auto var = builder.createParameterVariable(scope, name, argPos, file, line, ty, false);
	info.GetReturnValue().Set(DILocalVariableWrapper::of(var));
}

NAN_METHOD(DIBuilderWrapper::CreatePointerType) {
	auto& builder = DIBuilderWrapper::FromValue(info.Holder())->diBuilder;

	if (info.Length() != 3 || !(
		IS_TYPE(info[0]) &&
		info[1]->IsUint32() &&
		info[2]->IsString()
	)) {
		return Nan::ThrowTypeError("createPointerType needs to be called with pointeeType: DIType, sizeInBits: number, and name: string");
	}

	auto* ty = DIBasicTypeWrapper::FromValue(info[0])->getDIValue();
	auto size = Nan::To<uint32_t>(info[1]).FromJust();
	std::string name = ToString(info[2]);

	auto* ptr = builder.createPointerType(ty, size, 0, llvm::None, name);
	info.GetReturnValue().Set(DIDerivedTypeWrapper::of(ptr));
}

NAN_METHOD(DIBuilderWrapper::CreateStructType) {
	auto& builder = DIBuilderWrapper::FromValue(info.Holder())->diBuilder;

	// Check if args are correct
	if (info.Length() != 9 || !(
		IS_SCOPE(info[0]) &&
		info[1]->IsString() &&
		DIFileWrapper::isInstance(info[2]) &&
		info[3]->IsUint32() &&
		info[4]->IsUint32() &&
		info[5]->IsUint32() &&
		IS_TYPE(info[6]) &&
		info[7]->IsArray()
	)) {
		return Nan::ThrowTypeError("createStructType needs to be called with scope: DIScope, name: string, file: DIFile, line: number, size: number, align: number, derived: DIType, elements: Array<DIType>");
	}

	v8::Handle<v8::Array> array = v8::Handle<v8::Array>::Cast(info[7]);

	// Check that all elements in the array are correct
	std::vector<llvm::Metadata*> args(array->Length());
	for (auto i = 0u; i != array->Length(); ++i) {
		if (!(IS_TYPE(array->Get(i)))) {
			return Nan::ThrowTypeError("CreateStructType requires all of the elements of paramTypes to be of type DIType");
		}

		args[i] = DITypeWrapper::FromValue(array->Get(i))->getDIValue();
	}

	auto* scope = DIScopeWrapper::FromValue(info[0])->getDIValue();
	std::string name = ToString(info[1]);
	auto* file = DIFileWrapper::FromValue(info[2])->getDIValue();
	auto line = Nan::To<uint32_t>(info[3]).FromJust();
	auto size = Nan::To<uint32_t>(info[4]).FromJust();
	auto align = Nan::To<uint32_t>(info[5]).FromJust();
	auto* derived = DITypeWrapper::FromValue(info[6])->getDIValue();

	auto structy = builder.createStructType(scope, name, file, line, size, align, llvm::DINode::DIFlags::FlagPublic, derived, builder.getOrCreateArray(args));
	info.GetReturnValue().Set(DICompositeTypeWrapper::of(structy));
}

NAN_METHOD(DIBuilderWrapper::CreateSubroutineType) {
	auto& builder = DIBuilderWrapper::FromValue(info.Holder())->diBuilder;

	// Check if args are correct
	if (info.Length() < 1 ||
		(info.Length() == 1 && !(info[0]->IsArray())) ||
			info.Length() > 1) {
		return Nan::ThrowTypeError("CreateSubroutineType needs to be called with paramTypes: Array<DIBasicType>");
	}

	v8::Handle<v8::Array> array = v8::Handle<v8::Array>::Cast(info[0]);

	// Check that all elements in the array are correct
	std::vector<llvm::Metadata*> args(array->Length());
	for (auto i = 0u; i != array->Length(); ++i) {
		if (!(IS_TYPE(array->Get(i)))) {
			return Nan::ThrowTypeError("CreateSubroutineType requires all of the elements of paramTypes to be of type DIType");
		}

		args[i] = DITypeWrapper::FromValue(array->Get(i))->getDIValue();
	}

	auto subty = builder.createSubroutineType(builder.getOrCreateTypeArray(args));
	info.GetReturnValue().Set(DISubroutineTypeWrapper::of(subty));
}

NAN_METHOD(DIBuilderWrapper::Finalize) {
	auto& builder = DIBuilderWrapper::FromValue(info.Holder())->diBuilder;

	builder.finalize();
	info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(DIBuilderWrapper::InsertDeclare) {
	auto& builder = DIBuilderWrapper::FromValue(info.Holder())->diBuilder;

	if (info.Length() != 5 || !(
		ValueWrapper::isInstance(info[0]) &&
		DILocalVariableWrapper::isInstance(info[1]) &&
		BasicBlockWrapper::isInstance(info[2]) &&
		info[3]->IsUint32() &&
		info[4]->IsUint32()
	)) {
		return Nan::ThrowTypeError("insertDeclare needs to be called with value: Value, local: DILocalVariable, and block: BasicBlock");
	}

	auto* val = ValueWrapper::FromValue(info[0])->getValue();
	auto* var = DILocalVariableWrapper::FromValue(info[1])->getDIValue();
	auto* blo = BasicBlockWrapper::FromValue(info[2])->getBasicBlock();
	auto line = Nan::To<uint32_t>(info[3]).FromJust();
	auto column = Nan::To<uint32_t>(info[4]).FromJust();

	auto* loc = llvm::DebugLoc::get(line, column, var->getScope()).get();
	auto* expr = builder.createExpression();
	builder.insertDeclare(val, var, expr, loc, blo);
}

NAN_METHOD(DIBuilderWrapper::InsertDbgValueIntrinsic) {
	auto& builder = DIBuilderWrapper::FromValue(info.Holder())->diBuilder;

	if (info.Length() != 5 || !(
		ValueWrapper::isInstance(info[0]) &&
		DILocalVariableWrapper::isInstance(info[1]) &&
		BasicBlockWrapper::isInstance(info[2]) &&
		info[3]->IsUint32() &&
		info[4]->IsUint32()
	)) {
		return Nan::ThrowTypeError("insertDbgValueIntrinsic needs to be called with value: Value, local: DILocalVariable, and block: BasicBlock");
	}

	auto* val = ValueWrapper::FromValue(info[0])->getValue();
	auto* var = DILocalVariableWrapper::FromValue(info[1])->getDIValue();
	auto* blo = BasicBlockWrapper::FromValue(info[2])->getBasicBlock();
	auto line = Nan::To<uint32_t>(info[3]).FromJust();
	auto column = Nan::To<uint32_t>(info[4]).FromJust();

	auto* loc = llvm::DebugLoc::get(line, column, var->getScope()).get();
	auto* expr = builder.createExpression();
	builder.insertDbgValueIntrinsic(val, var, expr, loc, blo);
}