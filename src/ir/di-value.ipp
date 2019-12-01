template <class T>
Nan::Persistent<v8::FunctionTemplate> DIValueWrapper<T>::functionTemplate {};

template <class T>
NAN_MODULE_INIT(DIValueWrapper<T>::Init) {
	v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
	tpl->SetClassName(Nan::New(name).ToLocalChecked());
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	functionTemplate.Reset(tpl);
	Nan::Set(target, Nan::New(name).ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
}

template <class T>
v8::Local<v8::Object> DIValueWrapper<T>::of(T* diValue) {
	auto constructorFunction = Nan::GetFunction(Nan::New(functionTemplate)).ToLocalChecked();
	v8::Local<v8::Value> argv[1] = { Nan::New<v8::External>(diValue) };
	auto instance = Nan::NewInstance(constructorFunction, 1, argv).ToLocalChecked();

	Nan::EscapableHandleScope escapeScope {};
	return escapeScope.Escape(instance);
}

template <class T>
NAN_METHOD(DIValueWrapper<T>::New) {
	// return Nan::ThrowTypeError("DIFile should not be manually created. Use a DIBuilder's createFile method.");
	if (!info.IsConstructCall()) {
		return Nan::ThrowTypeError("DIValue functionTemplate needs to be called with new");
	}

	if (info.Length() != 1 || !info[0]->IsExternal()) {
		return Nan::ThrowTypeError("DIValue constructor needs to be called with: inst: External");
	}

	auto* inst = static_cast<T*>(v8::External::Cast(*info[0])->Value());
	auto* wrapper = new DIValueWrapper { inst };
	wrapper->Wrap(info.This());

	info.GetReturnValue().Set(info.This());
}

template <class T>
T* DIValueWrapper<T>::getDIValue() {
	return diValue;
}

template <class T>
bool DIValueWrapper<T>::isInstance(v8::Local<v8::Value> value) {
	return Nan::New(functionTemplate)->HasInstance(value);
}