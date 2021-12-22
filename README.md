[<img src="https://img.youtube.com/vi/I904IqwbCXk/maxresdefault.jpg">](https://youtu.be/I904IqwbCXk)

# Info

This project uses Data oriented/ECS programming to find how much performance you can squeeze out with these paradigms while using Godot.

This particular example is based on [Unity's AngryBots ECS](https://github.com/UnityTechnologies/AngryBots_ECS) and mimics its main functionality.

Currently, using this paradigm nets a lot of performance over using the traditional object oriented aproach with Godot nodes.
Yet there are some roadblocks that may arise when interfacing with Godot, causing bottlenecks and prevent you from reaching maximum potential.
In this case, having a lot of moving entities that meed to be displayed, the most low level/performant way that Godot exposes to do this are MultiMeshes (as of 3.2),
which implement a bunch of stuff that isn't need and has a cost, like copying the Transform data of each mesh instance for user convenience and recalculating its AABB.

Ideally you would be able to interface with the renderer and just submit that data straight to the GPU to draw, which would also allow to do your own extra culling if needed etc...
Sadly Godot doesn't expose anything like that (as of 3.2).

The other annoyance that may impose a bottleneck is that some functions that need a buffer/array expect a GodotArray, which means that you must copy the data if you arent using them.
In this project you have to pay said copy to pass the dozen of thousands of transforms to the MultiMesh, which is not cheap.
Would be cool if there was a way to construct a GodotArray without copying by implementing some kind of move-memory-ownership semantics.

# Building

Needs a C++17 compiler with OpenMP support.

[Uses the 3.2 GDNative CPP Bindings](https://docs.godotengine.org/en/stable/tutorials/plugins/gdnative/gdnative-cpp-example.html#setting-up-the-project)
