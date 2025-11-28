// MonoAppDemo.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/class.h"
#include "mono/metadata/debug-helpers.h"
#include "mono/metadata/mono-config.h"
#include "mono/metadata/appdomain.h"
#include "mono/metadata/object.h"
#include "mono/metadata/image.h"


typedef int (*AddFunction)(int, int);

int Add(int a, int b)
{
	return a + b;
}

int main()
{
    std::cout << "Hello Mono App(native)!\n";

	mono_set_dirs("D:/Code/MonoSDK/runtime", "");

	MonoDomain* domain = mono_jit_init("MyMonoApp");


	const char* assemblyPath = "../ManagedDemo/bin/Debug/net9.0/ManagedDemo.dll";

	MonoAssembly* assembly = mono_domain_assembly_open(domain, assemblyPath);
	MonoImage* image = mono_assembly_get_image(assembly);
	
	MonoClass* ManagedClass = mono_class_from_name(image, "ManagedDemo", "ManagedClass");

	{
		MonoMethod* MsgMethod = mono_class_get_method_from_name(ManagedClass, "PrintMessage", 0);
		mono_runtime_invoke(MsgMethod, nullptr, nullptr, nullptr);

	}

	{
		MonoMethod* addMethod = mono_class_get_method_from_name(ManagedClass, "Add", 2);
		AddFunction addFunc = (AddFunction)mono_method_get_unmanaged_thunk(addMethod);
		int result = addFunc(5, 7); 
		std::cout << "Result of Add(5, 7): " << result << std::endl;	
	}

	{
		MonoMethod* method = mono_class_get_method_from_name(ManagedClass, "RegisterFunctionPtr", 1);
		void* args[1];
		args[0] = (void*)(&Add);
		mono_runtime_invoke(method, nullptr, args, nullptr);


		// MonoMethod* method2 = mono_class_get_method_from_name(ManagedClass, "CallNativeAdd", 2);
		// void* args2[2];
        // int a = 15, b = 25;
        // args2[0] = &a;
        // args2[1] = &b;
        // MonoObject* exception = nullptr;
        // MonoObject* result = mono_runtime_invoke(method, nullptr, args2, &exception); 
        // if (!exception && result) {
        //     int addResult = *(int*)mono_object_unbox(result);
        //     printf("Managed call to native Add: %d + %d = %d\n", a, b, addResult);
        // }
	}

	mono_jit_cleanup(domain);

	domain = nullptr;
	return 0;

}

