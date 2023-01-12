// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Diagnostics;

namespace GenAwareDemo
{
    internal sealed class LongTermObject
    {
        public ShortTermObject leak;
    }

    internal sealed class ShortTermObject
    {
        public ShortTermObject prev;
        public byte[] weight;
    }

    internal sealed class Program
    {
        private static void Main()
        {
            Console.WriteLine("My process id is " + Environment.ProcessId);
            Console.ReadLine();
            LongTermObject LongTermObject = new LongTermObject();
            GC.Collect();
            GC.Collect();
            for (int phase = 0; phase < 2; phase++)
            {
                int counter = 0;
                for (int iteration = 0; iteration < 10000; iteration++)
                {
                    ShortTermObject head = new ShortTermObject();
                    for (int i = 0; i < 1000; i++)
                    {
                        ShortTermObject next = new ShortTermObject();
                        next.weight = new byte[1000];
                        next.prev = head;
                        head = next;
                    }
                    counter++;
                    // Emulate a leak of a ephermal object to LongTermObject.
                    if (counter % 1000 == 0)
                    {
                        Console.WriteLine("Leaked");
                        LongTermObject.leak = head;
                    }
                }
            }
        }
    }
}
