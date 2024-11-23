# Hack for profiler debuging

This hack will force the condition where we perform a foreground GC while a background GC is in progress.

## Instructions

Build the runtime and run `CoreLab\Program.cs` using it. Do not use a profiler, because then the behavior would be suppressed by it.

Ignore any scripts in the branch, they are just for my convenience.

## Observations

You should see the `DiagGCStart` callback that got called once (by the background GC), and then repeatedly a pair of `DiagGCStart`/`DiagGCEnd` callback for all the foreground ephemeral GCs.

The comments in `gc.cpp` should explain clearly how the hack works.

