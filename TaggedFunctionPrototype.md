# Tagged Function Prototype

The document is written to describe my prototype available [here](https://cloudbuild.microsoft.com/build?id=7a730686-9d69-fe04-56a7-2118a28196ea&bq=devdiv_DevDiv_DotNetFramework_QuickBuildNoDrops_AutoGen) as a PR. I am not planning to merge it.

The goal of this prototype is to investigate whether or not the tagged function concept is practically feasible in the CoreCLR code base.

## How does the CoreCLR interpreter work today?

This section covers a small portion of how the interpreter integrate with the runtime. It does NOT attempt to explain the full interpreter execution process.

The interpreter work by pretending itself as jitted code, as such, it needs to

1. Convert the incoming arguments from the register/stack to something C++ understands
2. The control flows to `InterpretMethodBody`, where it interprets the byte code.
3. Call any other callee as if they are jitted code as well, and
4. Put thing back on the stack as if it were produced by jitted code.

Step 1 is something require special generated code to do, right now, it is done by `GenerateInterpreterStub`. It is meant to be a tiny routine that take arguments from the stack
and rewrite the stack so that the values can be consumed by C++.

## What do we want?

We want to get rid of the concept of interpreter stub, and instead, have the caller calling the actual `InterpretMethodBody` directly.

`InterpretMethodBody` requires an `InterpreterMethodInfo` object, which basically is a representation where we can easily access its signature and its byte code.

So the problem is reduced to:

1. Identify a caller that is currently calling using the standard calling convention.
2. Get that caller to access an `InterpreterMethodInfo` object, and so
3. Make it calls `InterpretMethodBody` instead.

## Wrong attempts

I tried 3 different approaches to that and only the last one succeed. These wrong attempts are documented just so we don't try the same wrong idea again.

### Idea 1

- Make `GenerateInterpreterStub` return a tagged pointer instead

This approach failed because `GenerateInterpreterStub` is called as part of `ThePreStub`. `ThePreStub` works by leaving the call arguments on the stack, so the incoming call arguments are already on the stack, and we at least need some code to get it back.

### Idea 2

Now we know we must perform call `InterpretMethodBody` earlier then `ThePreStub`, which means `ThePreStub` must be replaced by something else. In fact, how does `ThePreStub` knows what `MethodDesc` to interpret? Upon investigation, I learn about this concept of `Precode`.

Basically, every method has a `Precode`, that is a simple `jmp` instruction the goes somewhere else. This is the first instruction that get executed. To begin with, that instruction jumps to `ThePreStub`, and that instruction is code generated. Given the precode, we can get to the MethodDesc.

What that means is that we need to get rid of the code generation during the Precode generation, which means will no longer have the jmp instruction. Instead, we will put a thing there that allow us to get to the `InterpreterMethodInfo`.

A reasonable choice is to put a pointer to the `InterpreterMethodInfo` object right there. We will tag the least significant bit of it so that we know it is not a normal function entry point.

To be more concrete, the precode is generated during `MethodDesc::EnsureTemporaryEntryPointCore`. We will modify that code so that it translate the `MethodDesc` into an `InterpreterMethodInfo` there and tag it so that we put it into the method table there.

The reason why this approach fails is more subtle. It turns out that the `InterpreterMethodInfo` construction process leveraged the code that supports the JIT to extract the IL, and that code assumed the method tables are also properly constructed, but that's not true at the time `MethodDesc::EnsureTemporaryEntryPointCore` is called. So we must delay the process of `InterpreterMethodInfo` object construction.

## Working approach

### Idea 3

To get around the cyclic dependency issue above, I tagged the MethodDesc pointer instead. By the time we are about to call the function, then we construct the `InterpreterMethodInfo`. This worked.

The down side of this approach, obviously, is that the pointer in the method table is no longer a valid entry point, so anything else that try to call it will lead to an access violation. This will work in a pure interpreted scenario, where the interpreter is the only thing that runs in the process. 

Suppose we also want to let (e.g. ReadyToRun) code to run, that won't work unless we also change the ReadyToRun callers.

The code in this branch demonstrated this concept. It will execute some code under the interpreter (and fail pretty quickly because I haven't implemented everything yet).

### Lowlights

This code is still using dynamic code generation for a couple of things. We are still generating code for GC write barrier, and we are still generating some glue code for pinvoke. Lastly, the call made by the interpreter is not converted to use the new call convention yet. These seems to be solvable problems.