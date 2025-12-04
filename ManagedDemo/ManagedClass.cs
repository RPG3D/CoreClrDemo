
using System.Runtime.InteropServices;
namespace ManagedDemo
{
    public unsafe class ManagedClass
    {

        static ManagedClass()
        {
            System.Console.WriteLine("Managed: static ManagedClass()");       
        }

        [UnmanagedCallersOnly]
        public static void PrintMessage(char* InMsg)
        {
            string message = Marshal.PtrToStringAnsi((IntPtr)InMsg);
            System.Console.WriteLine("Managed: PrintMessage in C#, " + message);
        }

        public static int Add(int a, int b)
        {
            return a + b;
        }


        // 使用 unmanaged 函数指针，因为指向的是 C++ 原生函数
        public static delegate* unmanaged<int, int, int> NativeAddPtr;
        
        // 托管方法，通过 Mono runtime 被 C++ 调用
        public static void RegisterNativeFunction(void* nativeFunctionPtr)
        {
            // 将传入的 IntPtr 转换为 unmanaged 函数指针
            NativeAddPtr = (delegate* unmanaged<int, int, int>)nativeFunctionPtr;
        }

        // 另一个方法，用于后续调用原生函数
        public static int CallNativeAdd(int a, int b)
        {
            if (NativeAddPtr == null)
            {
                System.Console.WriteLine("Managed: Native function not registered");
                throw new InvalidOperationException("Managed: Native function not registered");
            }
            int ret = NativeAddPtr(a, b);
            System.Console.WriteLine("Managed: Native function called " + ret);
            return ret;
        }
    }
}
