# Nujel

Nothing in here is final, these are just a collection of ideas, although the general direction will
probably stay the same. If you have found an oversight or have a more elegant solution then I'd love to hear about it!

---------------------------------------------------------------------------

# Problems
First I would like to describe the **problems** Nujel is adressing in order of importance, because otherwise the solutions won't make sense.

---------------------------------------------------------------------------

## Sandboxed
This is necessary because it would be great if a client could just run whatevery code we got from the server, this would allow all kinds of changes to the gameplay/controls/ui that might be possible but wouldn't feel great if they were forced
to reuse one common interface.  And who knows what else might be possible with something like this.
### Inspirations
- JavaScript
- eBPF

---------------------------------------------------------------------------

## Portable
Whatevery language/runtime we pick, it needs to be **as Portable as C99**. It doesn't have to run on a MCU but it has to run at least on every platform that can supply an OpenGL ES 3 context, especially *Windows*, *WASM*, *OpenBSD* and *ARM* seem to filter out many runtimes.
### Inspirations
- C
- Lua

---------------------------------------------------------------------------

## Concurrent
Nujel needs to be able to support **at least** 10k threads/fibers/agents, 100k would be better.  They need to be isolated from each other in that even a malicious fiber **must not** interfere with other fibers as much as possible, slowing things
down *slightly* is acceptable, but blocking everything is unacceptable.
### Inspirations
- Erlang / LFE
- Limbo / Inferno
- Kali Scheme

---------------------------------------------------------------------------

## Serializable
Everything needs to be serializable, **even running threads**.  This is because at any point in time we might have a couple of thousands threads active and the server needs to be able to serialize them to disk when shutting down.  We could force coders to write
special functions that serialize the current state but that just makes things more complicated, especially when there is no reason why we can't serialize a fiber/continuation to disk and resume a couple of days later, or send it over the network to finish on some other machine.
### Inspirations
- Stackless Python
- Telescript

---------------------------------------------------------------------------

## Fast
While it would be great to have everything be as fast as native code, that is not really necessary/feasible.  However there are some
scripted parts that need to be **as fast as possible**, for example code that does world generation where even the current C
code is too slow.  This code can be quite complicated to write (although it would be nice if it weren't) but there needs to be some sort of fast path.  Additionally long (> 1-2ms) GC pauses are just plain unacceptable for games, but even then we need to control when a GC pause might occur.
### Inspirations
- Julia
- Common Lisp / SBCL


---------------------------------------------------------------------------

# Solutions

## Static typing / multimethods
This seems like something that should be quite easy to implement and greatly improve performance.  This also means that we can use **multimethods** throughout the codebase with (almost) no performance overhead, which (in my opinion) is **much more convenient** than dynamic typing.
### Adresses
- Fast

---------------------------------------------------------------------------

## Preemption
The runtime needs to preempt fibers if they run for too long, since we can also run malicious code in a sandbox we can't depend on a fiber regularly calling some sort of yield function.  Blocking the event loop/scheduler must be impossible.
### Adresses
- Sandboxed
- Concurrent
- Serializable

---------------------------------------------------------------------------

## Tiny core
The language needs a minimal, safe, baseline language that is available even in sandboxed environments.  Every sort of I/O routine needs to be optional and only allowed for privilaged threads.
### Adresses
- Sandboxed
- Portable

---------------------------------------------------------------------------

## Multiple heaps
Nujel should have separate heaps that the GC can collect separately, this way we can limit the size of each single heap so the GC pauses won't be as long and additionally we should be able to schedule fibers in such a way that it won't be as noticeable (prioritize fibers just before and after a GC)
### Adresses
- Sandboxed
- Concurrent
