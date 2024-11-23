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
            Console.WriteLine("Hello hack");
            while (true)
            {
                int[] array = new int[20];
                array[0] = 12;
            }
        }
    }
}
