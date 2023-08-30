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
            int size = 9000;
            for (int i = 0; i < 10000; i++)
            {
                int[] whatever = new int[size];
                whatever[0] = 1;
                whatever[size-1] = 1;
            }
        }
    }
}
