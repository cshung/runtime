# Investigation on making interpreter work with ReadyToRun

## Status

This document is preliminary - it only covers the most basic case - it doesn't even cover very often used case (i.e. virtual method calls).

Imagine I am doing a Hackathon overnight trying to get something working, not designing something for the long term, yet.

## Goals

- Figure out how relevant parts of ready to run works.
- Figure out how to hack it so that we can get into the CoreCLR interpreter.

## Non Goals

- Deliver a working prototype (I just don't have the time - and the CoreCLR interpreter is not the right target)
- Come up with an optimal design (Same, I just don't have the time)

## High-level observations

We already have a mechanism to call an arbitrary managed method from the native runtime - this mechanism can be used to call ReadyToRun compiled method. So in general, interpreter -> ReadyToRun is not an issue.

The key challenge is to get ReadyToRun code to call into the interpreter.

## Understanding what happened when we are about to make an outgoing call from ReadyToRun

When ReadyToRun code makes a call to a static function, it

- push the arguments on the register/stack as per the calling convention
- call into a redirection cell
- get into the runtime.

Inside the runtime, I will eventually get to `ExternalMethodFixupWorker` defined in `prestub.cpp`.

At this point, I have
- transitionBlock - no idea what it is
- pIndirection - the address for storing the callee address
- sectionIndex - a number, pushed by the thunk, and
- pModule - a pointer to the module containing the call instruction

Since the call comes from a ReadyToRun image, `pModule` must have a ready to run image

We can easily calculate the RVA of the `pIndirection`

If the call provided the `sectionIndex`, we will just use it, otherwise we can still calculate the section index based on the RVA.

The calculation is simply by sequentially scanning the import sections, each section is self describing its address range so we can check

The import section has an array signature - using the rva - beginning rva of the section. we can index into the signature array to find the signature.

The signature is then parsed to become a `MethodDesc` - where the method preparation continues as usual

Last but not least, eventually, the `pIndirection` will be patched with that entry point, and the call proceed by using the arguments already on the stack/restored registers.

## How the potential hack looks like

We keep everything the same up to the method preparation part.

We knew it is possible to produce an `InterpreterMethodInfo` given a `MethodDesc` when the system is ready to JIT, so we should be able to produce the `InterpreterMethodInfo` there.

The arguments are already on the registers, but we can't dynamically generate the `InterpreterStub`, the only reasonable thing is to pre-generate the stubs in the ReadyToRun image itself.

> A stub per signature is necessary because each signature need a different way to populate the arguments (and the interpreter method info). On the other hand, a stub per signature is sufficient because if we knew how to prepare the register to begin with, we must know exactly what steps are needed to put them into a format the `InterpretMethodBody` likes. As people points out, this is going to be a large volume, this is by no means optimal.

The stub generation code can 'mostly' be exactly the same as `GenerateInterpreterStub` with two twists:

- We need to use indirection to get to the `InterpreterMethodInfo` object. That involves having a slot that the `InterpreterMethodInfo` construction process need to patch.
- What if the call signature involves unknown struct size (e.g. a method in A.dll take a struct in B.dll where B.dll is considered not in the same version bubble)

Next, we need the data structure that get us to the address of the stub as well as the address of the cell storing the `InterpreterMethodInfo`. What we have is `pIndirection` and therefore `MethodDesc`.

To do that, we might want to mimic how the runtime locate ReadyToRun code.

Here is a stack of how the ready to run code discovery look like:

```
coreclr!ReadyToRunInfo::GetEntryPoint+0x238 [C:\dev\runtime\src\coreclr\vm\readytoruninfo.cpp @ 1148]
coreclr!MethodDesc::GetPrecompiledR2RCode+0x24e [C:\dev\runtime\src\coreclr\vm\prestub.cpp @ 507]
coreclr!MethodDesc::GetPrecompiledCode+0x30 [C:\dev\runtime\src\coreclr\vm\prestub.cpp @ 443]
coreclr!MethodDesc::PrepareILBasedCode+0x5e6 [C:\dev\runtime\src\coreclr\vm\prestub.cpp @ 412]
coreclr!MethodDesc::PrepareCode+0x20f [C:\dev\runtime\src\coreclr\vm\prestub.cpp @ 319]
coreclr!CodeVersionManager::PublishVersionableCodeIfNecessary+0x5a1 [C:\dev\runtime\src\coreclr\vm\codeversion.cpp @ 1739]
coreclr!MethodDesc::DoPrestub+0x72d [C:\dev\runtime\src\coreclr\vm\prestub.cpp @ 2869]
coreclr!PreStubWorker+0x46d [C:\dev\runtime\src\coreclr\vm\prestub.cpp @ 2698]
coreclr!ThePreStub+0x55 [C:\dev\runtime\src\coreclr\vm\amd64\ThePreStubAMD64.asm @ 21]
coreclr!CallDescrWorkerInternal+0x83 [C:\dev\runtime\src\coreclr\vm\amd64\CallDescrWorkerAMD64.asm @ 74]
coreclr!CallDescrWorkerWithHandler+0x12b [C:\dev\runtime\src\coreclr\vm\callhelpers.cpp @ 66]
coreclr!MethodDescCallSite::CallTargetWorker+0xb79 [C:\dev\runtime\src\coreclr\vm\callhelpers.cpp @ 595]
coreclr!MethodDescCallSite::Call+0x24 [C:\dev\runtime\src\coreclr\vm\callhelpers.h @ 465]
```

The interesting part, of course, is how `GetEntryPoint` works. Turn out it is just a `NativeHashtable` lookup given a `VersionResilientMethodHashCode`, so we should be able to encode the same hash table for the stubs as well.

Note that `GetEntryPoint` has the fixup concept, maybe we can use the same concept to patch the slot for `InterpreterMethodInfo`.

## How to implement the potential hack

From the compiler side:

### When do we need to generate the stubs?
When the ReadyToRun compiler generate a call, the JIT will call back into crossgen2 to create a slot for it. At that point, we should know what we need to make sure a stub is available for it by working with the dependency tracking engine.

### Actually generate the stubs

To stub generation should mostly work the same as in `GenerateInterpreterStub` today with a couple twists
- We don't need to generate the `InterpreterMethodInfo`, that work is left until runtime.
- If the stub involve types with unknown size, we need to generate the right stub code for it (e.g. A.dll call a function that involves a struct defined in `B.dll` where they are not in the same version bubble)
- The stub needs an instance of `InterpreterMethodInfo`, it cannot be hardcoded, the pointer of it must be read from somewhere else.
- Whenever we generate the stub, we need to store it somewhere so that we can follow the logic as in `MethodEntryPointTableNode`

From the runtime side:

### Locating the stub
- When we reach `ExternalMethodFixupWorker`, we need to use the table to get back to the generated stubs

### Preparing the data
- We need to create the `InterpreterMethodInfo` and make sure the stub code will be able to read it.

## Alternative designs
Following the thought on the earlier prototype for tagged pointers, we could envision a solution that ditch all those stubs, e.g.

1. Changing the call convention for every method so that it is the same as what the interpreter method likes.

    Pros:
    - Consistency, easily to understand
    - No need for stubs, efficient for interpreter calls

    Cons:
    - Lots of work to have a different calling convention
    - Inefficient for non interpreter calls

2. Changing the call site so that it detects tagged pointers and call differently

    Pros:
    - Similar with what we have in the tagged pointer prototype
    - No need for stubs, efficient for interpreter calls

    Cons:
    - Every call involves dual call code

3. The approach described in this document (i.e. using stubs)

    Pros:
    - Probably cheapest to implement

    Cons:
    - Lots of stubs
    - Inefficient for interpreter call (involve stack rewriting)
    - Unclear how it could work with virtual or interface calls

I haven't put more thoughts into these alternative solutions, but I am aware they exists.