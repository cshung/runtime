// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Reflection;
using System.Diagnostics;

namespace CoreLab
{
    internal static class Program
    {
        private static void Main()
        {
            AppContext.SetData("GCHeapHardLimit", (ulong)209715200);

            var gcType = typeof(GC);
            var method = gcType.GetMethod("_RefreshMemoryLimit", BindingFlags.NonPublic | BindingFlags.Static);
            if (method == null)
                throw new Exception("Failed to find _RefreshMemoryLimit method through reflection");

            var result = method.Invoke(null, Array.Empty<object>());
            GC.Collect();
            Console.WriteLine("Result of Refresh: " + result);
            Console.WriteLine("TotalAvailableMemoryBytes = " + GC.GetGCMemoryInfo().TotalAvailableMemoryBytes);
        }
    }
}
