# Low Memory Callback Prototype

This branch is a prototype implementation of having the GC to issue a low memory callback so that managed code (e.g. libraries) can opt-in to handle the siutation.

# Disclaimer

This is a *prototype*, I didn't put much effort into designing it or testing it. I hope not, but it might be wrong, or it might crash, or it might leads to slower code, I don't know yet. The goal is to figure these out.

This prototype was developed under Windows, I hope it works on *nix platforms but I haven't tested that.

# How to use

To use this prototype, we can:

1. Build the coreclr runtime
The script `bc.cmd` will build the runtime. Use `so.cmd` or `sd.cmd` to choose debug or release build, the default is debug.

2. Create a dotnet console application

```cmd
mkdir c:\sample-application
cd c:\sample-application
c:\dev\runtime\dotnet.cmd new console
```

3. Change the code to the following:

```c#
using System;
using System.Reflection;

public static class Program
{
    public static int Main(string[] args)
    {
        MethodInfo registerLowMemoryCallbackMethod = typeof(GC).GetMethod("RegisterLowMemoryCallback", BindingFlags.Public|BindingFlags.Static)!;
        registerLowMemoryCallbackMethod.Invoke(null, new object[]{new Action(Callback)});
        return 0;
    }

    public static void Callback()
    {
        Console.WriteLine("The callback is executed!");
    }
}
```

4. Publish the application in the self-contained mode

```
cd c:\sample-application
c:\dev\runtime\dotnet.cmd publish -r win-x64 --self-contained
```

5. Replace the runtime with the built runtime

```
cd c:\sample-application\bin\Debug\net6.0\win-x64\publish
xcopy /s/y c:\dev\runtime\artifacts\bin\coreclr\windows.x64.Debug\*
```

6. Run the application

```
cd c:\sample-application\bin\Debug\net6.0\win-x64\publish
sample-application.exe
```

This should *NOT* run the callback as we don't have a memory load at all. It should when the load it high.

# How does it work

I added the `RegisterLowMemoryCallback` API on `GC.cs`, using the QCALL infrastructure, it will get into the runtime at `GCInterface::_RegisterLowMemoryCallback`, all that does is saving the callback in a global variable named `g_lowMemoryCallback`.

At the finalizer thread, we will call this function if it exists and matches the low memory condition reported from the GC. When a GC happens, we will call `generation_to_condemn` to evaluate the memory status, and when the GC decides that the memory load is high and therefore trigger a gen 2 GC, it will also raise the flag and enable finalization. That will lead to the execution of the callback. The flag will reset itself once the finalizer decides to run the callback so hopefully we will not be calling that repeatedly.

# Known limitation
Currently it only support one callback. Instead of maintaining a list of callback in the native runtime, I am thinking about letting the managed code to do it. Feel free to send me a PR if you think you need more than one registration.

# Wanted - community validation
I think it is bad idea to design and implement it in vacuum. It makes much more sense if we collaborate and test it on real life scenarios so that we can optimize it again real usages. If you can try it out and report back what you find (e.g. it doesn't work, it triggered too early, too late, ...), that would be great. Even if it worked perfectly, sharing some perf data with us would be good as well.

The whole prototype is mostly additive and the code is arranged as a single commit, so hopefully this can cherry-pick to whatever coreclr version we need (hopefully not too old).

