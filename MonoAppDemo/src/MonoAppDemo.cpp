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

int main()
{
    std::cout << "Hello Mono App(native)!\n";

	mono_set_dirs("D:/Code/MonoSDK/runtime", "");

	MonoDomain* domain = mono_jit_init("MyMonoApp");


	const char* assemblyPath = "../ManagedDemo/bin/Debug/net9.0/ManagedDemo.dll";

	MonoAssembly* assembly = mono_domain_assembly_open(domain, assemblyPath);
	MonoImage* image = mono_assembly_get_image(assembly);
	
	MonoClass* ManagedClass = mono_class_from_name(image, "ManagedDemo", "ManagedClass");
	MonoMethod* MsgMethod = mono_class_get_method_from_name(ManagedClass, "PrintMessage", 0);
	mono_runtime_invoke(MsgMethod, nullptr, nullptr, nullptr);


	MonoMethod* addMethod = mono_class_get_method_from_name(ManagedClass, "Add", 2);

	AddFunction addFunc = (AddFunction)mono_method_get_unmanaged_thunk(addMethod);

	// Now you can call it like a regular function pointer
	int result = addFunc(5, 7); 
	std::cout << "Result of Add(5, 7): " << result << std::endl;

	mono_jit_cleanup(domain);

	domain = nullptr;
	return 0;

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
