#ifndef LLVM_NODE_CONSTANT_EXPR_WRAPPER_H
#define LLVM_NODE_CONSTANT_EXPR_WRAPPER_H

#include <nan.h>
#include <llvm/IR/Constants.h>
#include "constant.h"
#include "../util/from-value-mixin.h"

class ConstantExprWrapper: public ConstantWrapper, public FromValueMixin<ConstantExprWrapper> {
public:
    static NAN_MODULE_INIT(Init);
    static v8::Local<v8::Object> of(llvm::ConstantExpr* constantExpr);
    using FromValueMixin<ConstantExprWrapper>::FromValue;
    llvm::ConstantExpr* getConstantExpr();

private:
    explicit ConstantExprWrapper(llvm::ConstantExpr* constant)
            : ConstantWrapper { constant }
    {}

    static Nan::Persistent<v8::FunctionTemplate>& constantExprTemplate();

    // static
    static NAN_METHOD(New);
    static NAN_METHOD(getBitCast);
};

#endif //LLVM_NODE_CONSTANT_FP_WRAPPER_H
