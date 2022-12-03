// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Linq;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Collections.Concurrent;
using System.Threading;
using System.Runtime.CompilerServices;

namespace CoreLab
{
    internal sealed class Program
    {
        private static class Assert
        {
            public static void True(bool unused, string unused2 = "")
            {
                Debug.Assert(unused, unused2);
            }

            public static void Equal(object a, object b)
            {
                Debug.Assert(a.Equals(b));
            }
        }

        public static void Main()
        {
            new Program().Repro();
        }

        // drop this into a test project

        [StructLayout(LayoutKind.Explicit, Size = Size)]
        private struct Punned
        {
            internal const int Size = 8;

            [FieldOffset(0)]
            public ulong A;
        }

        /// <summary>
        /// This spawns a bunch of threads, half of which do integrity checks on a punned byte[]
        /// and half of which randomly replace a referenced byte[].
        ///
        /// Sometimes things just break: either field corruption, null ref, or access violation.
        ///
        /// Reproduces in DEBUG builds and RELEASE builds.
        ///
        /// Tends to take < 10 iterations, but not more than 100.  You know, on my machine.
        ///
        /// Only reproduces if you use the POH, SOH and LOH are fine.
        /// </summary>
        public void Repro()
        {
            // DOES repro with ALLOC_SIZE >= Punned.Size
            //        and with USE_POH == true
            //
            // does not repro if USE_POH == false

            // tweak these to mess with alignment and heap
            const int ALLOC_SIZE = Punned.Size;
            const bool USE_POH = true;

            Assert.True(Punned.Size == Unsafe.SizeOf<Punned>(), "Hey, this isn't right");
            Assert.True(ALLOC_SIZE >= Punned.Size, "Hey, this isn't right");

            const int MAX_KEY = 1_000_000;

            var iter = 0;
            while (true)
            {
                Debug.WriteLine($"Iteration: {iter}");
                iter++;

                var dict = new ConcurrentDictionary<int, byte[]>();

                // allocate
                for (var i = 0; i < MAX_KEY; i++)
                {
                    dict[i] = GC.AllocateArray<byte>(Punned.Size, pinned: USE_POH);
                }

                // start all the threads
                using var startThreads = new SemaphoreSlim(0, Environment.ProcessorCount);

                var modifyThreads = new Thread[Environment.ProcessorCount / 2];
                for (var i = 0; i < modifyThreads.Length; i++)
                {
                    modifyThreads[i] = ModifyingThread(i, startThreads, dict);
                }

                var checkThreads = new Thread[Environment.ProcessorCount - modifyThreads.Length];
                using var stopCheckThreads = new SemaphoreSlim(0, checkThreads.Length);
                for (var i = 0; i < checkThreads.Length; i++)
                {
                    checkThreads[i] = IntegrityThread(i, MAX_KEY / checkThreads.Length, startThreads, stopCheckThreads, dict);
                }

                // let 'em go
                startThreads.Release(modifyThreads.Length + checkThreads.Length);

                // wait for modifying threads to finish...
                for (var i = 0; i < modifyThreads.Length; i++)
                {
                    modifyThreads[i].Join();
                }

                // stop check threads..
                stopCheckThreads.Release(checkThreads.Length);
                for (var i = 0; i < checkThreads.Length; i++)
                {
                    checkThreads[i].Join();
                }
            }

            static Thread IntegrityThread(
                int threadIx,
                int step,
                SemaphoreSlim startThreads,
                SemaphoreSlim stopThreads,
                ConcurrentDictionary<int, byte[]> dict
            )
            {
                using var threadStarted = new SemaphoreSlim(0, 1);

                var t =
                    new Thread(
                        () =>
                        {
                            threadStarted.Release();

                            startThreads.Wait();

                            while (!stopThreads.Wait(0))
                            {
                                for (var i = 0; i < MAX_KEY; i++)
                                {
                                    var keyIx = (threadIx * step + i) % MAX_KEY;

                                    ref Punned punned = ref Pun(dict[keyIx]);

                                    Check(ref punned);
                                }
                            }
                        }
                     );
                t.Name = $"{nameof(Repro)} Integrity #{threadIx}";
                t.Start();

                threadStarted.Wait();

                return t;
            }

            static Thread ModifyingThread(int threadIx, SemaphoreSlim startThreads, ConcurrentDictionary<int, byte[]> dict)
            {
                using var threadStarted = new SemaphoreSlim(0, 1);

                var t = new
                    Thread(
                        () =>
                        {
                            threadStarted.Release();

                            var rand = new Random(threadIx);

                            startThreads.Wait();

                            for (var i = 0; i < 1_000_000; i++)
                            {
                                var keyIx = rand.Next(MAX_KEY);

                                var newArr = GC.AllocateArray<byte>(Punned.Size, pinned: USE_POH);
                                Assert.True(newArr.All(x => x == 0));

                                // make sure it comes up reasonable
                                ref Punned punned = ref Pun(newArr);
                                Assert.Equal(0UL, punned.A);

                                // this swaps out the only reference to a byte[]
                                // EXCEPT for any of the checking threads, which only
                                // grab it through a ref
                                dict.AddOrUpdate(keyIx, static (_, passed) => passed, static (_, _, passed) => passed, newArr);
                            }
                        }
                    );
                t.Name = $"{nameof(Repro)} Modify #{threadIx}";
                t.Start();

                threadStarted.Wait();

                return t;
            }

            static ref Punned Pun(byte[] data)
            {
                var span = data.AsSpan();

                var punned = MemoryMarshal.Cast<byte, Punned>(span);

                return ref punned[0];
            }

            static void Check(ref Punned val)
            {
                // all possible bit patterns are well known
                var a = val.A;
                // Assert.True(a == 0);
            }
        }
    }
}
