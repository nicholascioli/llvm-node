//
// Created by Micha Reiser on 11.04.17.
//

#include <llvm/BinaryFormat/Dwarf.h>
#include "dwarf.h"

NAN_MODULE_INIT(InitDwarf) {
	auto attribute = Nan::New<v8::Object>();

	auto attrKind = Nan::New<v8::Object>();
	Nan::Set(attrKind, Nan::New("C").ToLocalChecked(), Nan::New(static_cast<uint32_t>(llvm::dwarf::DW_LANG_C)));

	auto ateKind = Nan::New<v8::Object>();
	Nan::Set(ateKind, Nan::New("Boolean").ToLocalChecked(), Nan::New(static_cast<uint32_t>(llvm::dwarf::DW_ATE_boolean)));
	Nan::Set(ateKind, Nan::New("Float").ToLocalChecked(), Nan::New(static_cast<uint32_t>(llvm::dwarf::DW_ATE_float)));
	Nan::Set(ateKind, Nan::New("Signed").ToLocalChecked(), Nan::New(static_cast<uint32_t>(llvm::dwarf::DW_ATE_signed)));
	Nan::Set(ateKind, Nan::New("Unsigned").ToLocalChecked(), Nan::New(static_cast<uint32_t>(llvm::dwarf::DW_ATE_unsigned)));

	Nan::Set(attribute, Nan::New("Lang").ToLocalChecked(), attrKind);
	Nan::Set(attribute, Nan::New("ATE").ToLocalChecked(), ateKind);

	Nan::Set(target, Nan::New("Dwarf").ToLocalChecked(), attribute);
}