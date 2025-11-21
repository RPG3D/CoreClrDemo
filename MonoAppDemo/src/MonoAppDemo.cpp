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


/*

copy mono sdk at E:\Code\MonoSDK

E:\Code\MonoSDK\runtime, place managed dlls here
E:\Code\MonoSDK\include, place mono headers here
E:\Code\MonoSDK\lib, place mono libs here

*/

int main()
{
    std::cout << "Hello Mono App!\n";

	mono_set_dirs("E:/Code/MonoSDK/runtime", "");

	MonoDomain* domain = mono_jit_init("MyMonoApp");


	const char* assemblyPath = "../ManagedDemo/bin/Debug/net9.0/ManagedDemo.dll";

	MonoAssembly* assembly = mono_domain_assembly_open(domain, assemblyPath);
	MonoImage* image = mono_assembly_get_image(assembly);
	
	MonoClass* ManagedClass = mono_class_from_name(image, "ManagedDemo", "ManagedClass");
	MonoMethod* ManagedMethod = mono_class_get_method_from_name(ManagedClass, "PrintMessage", 0);

	mono_runtime_invoke(ManagedMethod, nullptr, nullptr, nullptr);

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
