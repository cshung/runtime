// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Runtime.CompilerServices;

public class InterpreterTest
{
    static int Main(string[] args)
    {
        //
        // This is the test case I used to work on the debugger
        //
        // To reproduce the issue, run the test with the debugger attached
        // break on coreclr!InterpreterStub
        // and then use !DumpMD to inspect the method
        //
        // We will see that the debugger output is a lie, the method
        // is described as MinOptJit where we are actually running
        // the interpreter
        //
        RunInterpreterTests();
        return 100;
    }

    [MethodImpl(MethodImplOptions.NoInlining)]
    public static void RunInterpreterTests()
    {
//        Console.WriteLine("Run interp tests");
    }

}
