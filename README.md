Hlsl-To-Glsl
============

This utility program takes a *.hlsl file as input and convert it into a *.glsl file.

The utility has been made to work with the Sketch-3D framework: [https://github.com/gviau/sketch-3d](https://github.com/gviau/sketch-3d)

As such, it might not directly work with your environment, but should be close enough.

There are a few assumptions that the program makes:
* It is assumed that the Hlsl program is used by the **Direct3D11** API
* It is assumed that the *.hlsl file compiles correctly. Otherwise, it might generate a none compiling *.glsl or even a not correctly formatted *.glsl file
* It is assumed that the Hlsl uses SamplerStates and Textures with specified register slot

This last point is particular and kind of specific to the Sketch-3D engine. In Sketch-3D, SamplerStates and Textures are separate objects. However,
because OpenGL doesn't separate those, the Hlsl-To-Glsl program will generate several Sampler2D objects, one for each combination of sampler states and
textures found in the original Hlsl program.

Known issues
============
* The **mul** HLSL keyword can have several inputs embedded in it, such as this example: mul(matrix1, mul(matrix2, float4(vertex, 1.0))). However,
the parse currently assumes that the **mul** operation will only have 2 non-nested parameters, such as: mul(matrix1, vertex). I will eventually
fix that.
* Some built-in HLSL functions are not supported yet, namely **clip**, **fmod** and **log10**.
* Only vertex and fragment shaders are supported right now.
* Block comments /* */ are not supported yet.
* Struct weren't tested properly yet. They might not work out of the box.
* Multiple render targets weren't tested properly yet. They might not work out of the box.
* The formatting is not that great yet. But it's not too shabby either.

How to build
============
**Cmake 2.8 is required**
* Create a folder named, for example, "build", in your repository root folder.
* Go in it
* Open a command shell with Administrator rights
* Run, from this folder you created in step 1, the following command:
```
cmake ../. -G "Generator Name"
```

where the relative path "../." points to the root folder of the git repository and "Generator Name" is the name of a generator accepted by CMake which you will use to generate the binaries.
You can find all the generator names here : [http://www.cmake.org/cmake/help/v3.0/manual/cmake-generators.7.html](http://www.cmake.org/cmake/help/v3.0/manual/cmake-generators.7.html)

Here are some example depending of your OS:
* For Windows, use "Visual Studio 11" for Visual Studio 2012
* For Linux, use "Unix Makefiles"
* For Mac OS X, use "Unix Makefiles" or Xcode

The project file, or makefiles, will then be generated. You can then compile the source.

To use
======
There are two way to use the utilitary:
* Either copy directly the header and source files in your project to use them
* Or compile the program and use it via a command shell. Here is the usage:
```
hlsl-to-glsl input_file.hlsl output_file.glsl isVertexShader {true|false}
```