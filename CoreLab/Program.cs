// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

namespace CoreLab
{
    using System;
    using System.Reflection;
    using System.Diagnostics;

    internal static class Program
    {
        private static void Main(string[] args)
        {
            MethodInfo refreshMemoryLimitMethod = typeof(GC).GetMethod("RefreshMemoryLimit", BindingFlags.Public | BindingFlags.Static);
            if (refreshMemoryLimitMethod == null)
            {
                Console.WriteLine("Ooops, wrong version");
            }
            else
            {
                Console.WriteLine("Let's change the job object?");
                Console.ReadLine();
                refreshMemoryLimitMethod.Invoke(null, Array.Empty<object>());
                Console.WriteLine("Let's change the job object again?");
                Console.ReadLine();
                refreshMemoryLimitMethod.Invoke(null, Array.Empty<object>());
                Console.WriteLine("Yay!");
            }

        }
    }
}
