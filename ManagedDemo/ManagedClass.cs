namespace ManagedDemo
{
    public unsafe class ManagedClass
    {

        public static void PrintMessage()
        {
            System.Console.WriteLine("Hello from ManagedClass in C#!");
        }

        public static int Add(int a, int b)
        {
            return a + b;
        }


        // 使用 unmanaged 函数指针，因为指向的是 C++ 原生函数
        public static delegate* unmanaged[Cdecl]<int, int, int> NativeAddPtr;
        
        // 托管方法，通过 Mono runtime 被 C++ 调用
        public static IntPtr RegisterNativeFunction(IntPtr nativeFunctionPtr)
        {
            // 将传入的 IntPtr 转换为 unmanaged 函数指针
            NativeAddPtr = (delegate* unmanaged[Cdecl]<int, int, int>)nativeFunctionPtr;
            
            Console.WriteLine("Native function registered successfully!");
            
            // 测试调用原生函数
            if (NativeAddPtr != null)
            {
                int result = NativeAddPtr(10, 20);
                Console.WriteLine($"Test call: 10 + 20 = {result}");
            }
            
            return nativeFunctionPtr;
        }

        // 另一个方法，用于后续调用原生函数
        public static int CallNativeAdd(int a, int b)
        {
            if (NativeAddPtr == null)
                throw new InvalidOperationException("Native function not registered");
                
            return NativeAddPtr(a, b);
        }
    }
}
