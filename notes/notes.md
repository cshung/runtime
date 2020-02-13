# Goal
This is a *personal* branch that I use to prototype string deduplication. I screw coding practices, I use funny names, I don't care.

## Strings 
Strings is a simple .NET Core console application that create and deduplicate strings.

`run.cmd` can be used to debug a privately build CoreCLR that has the string deduplication capability.
`andrew_break` will hit twice, once before the deduplication and once after the deduplication.

We will collect two dumps at each break.

Note that the program outputs "I win", meaning it does remove the expected duplicates and not removing the unexpected duplicates.
Not removing the unexpected duplicates is important because we do not update card table.

## DumpStrings

DumpStrings is a tool for analyzing the heap for duplicated strings, it emulates what the prototype does (without its limitation on hash table size) and produce a report as shown below.

`c:\dev\runtime\DumpStrings> dotnet run before.dmp`

```
Gen 2 size         : 63944
Gen 2 strings      : 12848 bytes out of 68 strings
Gen 2 scan string  : 12848 bytes out of 33 strings
Gen 2 dup string   : 30 bytes out of 1 strings
```

`c:\dev\runtime\DumpStrings> dotnet run after.dmp`
```
Gen 2 size         : 63944
Gen 2 strings      : 12848 bytes out of 68 strings
Gen 2 scan string  : 12848 bytes out of 33 strings
Gen 2 dup string   : 0 bytes out of 0 strings
```

The gen 2 size is obvious, this is the size of the gen 2 heap. We only cared about gen 2 heap. This isn't particularly useful, it neither represents the amount of work, nor it represents how much it save, but it could be used to give a sense on order of magnitude.

The gen 2 strings are all the strings in gen 2, the first number is bytes and the second number is the number of strings. This isn't particularly useful either, but we could use it to convince ourselves strings is a significant part of gen 2.

The gen 2 scan strings are all the strings that we check, this is a smaller number than gen 2 string because we only cared the strings that are referenced by a gen 2 objects. Some of these gen 2 strings are statics so that are not counted. This roughly represents the amount of work that we will spend on scanning the strings. My hunch is that computing hash code (and therefore the work for reading the string) is the most significant part of the processing.

The gen2 dup strings are all the strings that are considered duplicate. This represents a potential saving (if we do not remove all references to the string, the deduplication is 'done' but not useful)

As an example interpretation:
The above report shows that we saved 30 bytes after scanning 12848 bytes, this is clearly wasteful. This example is only useful as a functional test.

# Other tools
I checked out the other tools for duplicating strings detection, they are useful, but none of them enumerate gen 2 references to gen 2 strings, so I rolled my own. The tool is not just useful for data collection, it is also useful to debug my prototype, both code perform precisely the same thing (except the hash function and hash table size limitation)

# Data collection process
Capture at least two crash dumps. Ideally with a string deduplication happened in between (i.e. running the private CoreCLR build with String deduplication built in), but I understand it is hard and that's fine without using the private build and the data is still useful.

Run the program to see the scan to dup ratio, if it is high, we win (i.e. found good data to support dedup work)

Run the program on consecutive dumps, if there is a dedup happened between and live size reduces, we win (i.e. string dedup is useful)

Run the program on consecutive dumps, if there is not a dedup happened between and we have more dup strings, we win (i.e. application is continuously generating duplicates)

# Notes
## Coding Notes
- the 1 suffix means nothing.
- Multiple generations could share the same segment
- Small address on the left, gen 2 is on the left of gen 1 on the left of gen 0

## Question
- Why do we have 5 generations?

## Coding progress
The basic scenario seems to work, I win!
The basic scenario seems to work on server GC, I win again!

## Idea
There are quite a few useless strings constructed on startup, these are all error messages that is not happening. (They are loaded as ready to run fix up), can we not create these strings? Not sure.
Even if the read-only heap segment contains string reference, we can't change it anyway, so no big deal there (yet) - what if strings are hold alive there? 

## Bloom filter
Right now, whenever we see a gen 2 reference to a gen 2 string, we compute the hash code and check if it is duplicated. This would be a waste of effort if this string object is already in the hash table. A simple bloom filter that check if the string is already in the hash table can save this effort if we are happy to accept a small probability that some strings are not in the hash table already but we fail (choose not) to dedup.

## TODO
Testing:
- [x] Workstation GC on .NET Core
- [x] Server GC on .NET Core
- [ ] Workstation GC on .NET Framework
- [ ] Server GC on .NET Framework
- [ ] Workstation GC on Linux
- [ ] Server GC on Linux
