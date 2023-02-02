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
            MethodInfo andrewCuteMethod = typeof(GC).GetMethod("AndrewCute", BindingFlags.Public | BindingFlags.Static);
            if (andrewCuteMethod == null)
            {
                Console.WriteLine("AndrewCute method not found");
            }
            else
            {
                andrewCuteMethod.Invoke(null, Array.Empty<object>());
            }
        }
    }
}
