# Py3ScriptingEngine
For C++ code

## Setting up your build options

Add the relative path `scripting/Python34/include` to your compiler's paths
Add the relative path `scripting/Python34/libs` to your linker's paths
Add the option `-lPython34` to your linker's options

## How to use

Download the Zip archive and extract it somewhere (recommanded to extract the `scripting` directory in your source directory)
Then `#include "path to scripting/scripting.hpp"`

To start the scripting engine, just do `PyScripting::connect()`
It will automatically load all the scripts in the specified directory (you can change it line 19 in `scripting.cpp`), and create a Python Module from the C++ source in the namespace `PyModule` (in the `scripting.cpp` file).

### Create a C++ function for the Python module
You can check this : https://docs.python.org/3/extending/extending.htm
And also this : https://docs.python.org/3/extending/embedding.html

### Add more functions to the module
You just have to add an entry in the `PyModuleDef` table, like this :
```cpp
// module definition
static PyMethodDef ModuleMethods[] = {
    // ...
    {"the name the function will have in the python code", the_cpp_function, METH_VARARGS, documentation},
    {"test", test, METH_VARARGS, "Return a number time PyScripting::getValue()"},
    // ...
    {NULL, NULL, 0, NULL}  // sentinel
};
```

The documentation is optionnal, you can just put `NULL` if you don't want to write one.

`METH_VARARGS` refers to the kind of argument the C++ function will receive from the Python code
You can find more about it right there : https://docs.python.org/3/extending/extending.html#the-module-s-method-table-and-initialization-function
And also here : https://docs.python.org/3/c-api/structures.html#METH_KEYWORDS

### Define a module
The documentation (at https://docs.python.org/3/extending/extending.html#the-module-s-method-table-and-initialization-function , second code block) wants you to add `struct` after `static` when defining a module, but notice it also works without it, so don't be surprise.

A module is defined like this :

```cpp
static PyModuleDef ModulenameModule = {
   PyModuleDef_HEAD_INIT,  // everymodule must start like this
   "spam",   /* name of module */
   spam_doc, /* module documentation, may be NULL */
   -1,       /* size of per-interpreter state of the module,
                or -1 if the module keeps state in global variables. */
   ModuleMethods  // the methods of the module, which we defined some time ago :)
};
```

## Adding "globals" values to your Python Module
By "adding globals values", I mean variables, eg. an instance of `Character`, a class in your code which controls the player, his inventory and all that stuff.

Of course a scripting module is unuseful if you can only interact from one side to another, but not the other way :/
So what we are going to do in this example is a C++ function to load a texture in your game, and another to draw the texture ... somewhere

First, put yourself in the namespace `PyModule`, we are going to create a new function.

```cpp
static PyObject* load_texture(PyObject* self, PyObject* args)
{
    const char* fname;
    if (!PyArg_ParseTuple(args, "s", &fname))  // we want a string ("s"), and we want to have it in fname
    {
        // an error occured, let's tell Python about it
        PyErr_SetString(ModuleError, "Can not parse arguments");
        return NULL;
    }
    PyScripting::load_texture(fname);
    
    RETURN_NONE;  // a define I already added for you ;)
}

static PyObject* display_texture(PyObject* self, PyObject* args)
{
    const char* fname;
    int x;
    int y;
    if (!PyArg_ParseTuple(args, "sii", &fname, &x, &y)) // we are retrieving a string, and 2 integers
    {
        // an error occured, let's tell Python about it
        PyErr_SetString(ModuleError, "Can not parse arguments");
        return NULL;
    }
    PyScripting::display_texture(fname, x, y);
    
    RETURN_NONE;
}
```

Now Our ModuleMethods[] should look like this :

```cpp
static PyMethodDef ModuleMethods[] = {
    // ...
    {"load_texture", load_texture, METH_VARARGS, NULL},
    {"display_texture", display_texture, METH_VARARGS, NULL},
    // ...
    {NULL, NULL, 0, NULL}  // sentinel
};
```

And now we need to add 2 methods to our PyScripting class : `load_texture`, and `display_texture`

`PyScripting.hpp`:

```cpp
public:
// ...
    static void load_texture(const char*);
    static void display_texture(const char*, int, int);
private:
    SCREEN screen;  // here SCREEN represents the type of your window object (it's sf::RenderWindow in SFML)
    TEXTURE_MANAGER textures;
};
// ...
```

`PyScripting.cpp`:

```cpp
// ...
void PyScripting::load_texture(const char* name)
{
    instance.textures.load_and_add_texture(std::string(name));  // we convert the const char* to string
}

void PyScripting::display_texture(const char* name, int x, int y)
{
    // IT'S ONLY AN EXAMPLE
    // not all the graphics libs work like this
    instance.screen.display(instance.textures.get(std::string(name)), x, y);
}
// ...
```

And in your Python code, you will just have to do :

```python
import Module  # it's the name of the module, defined in the namespace PyModule

Module.load_texture("my texture.png")
Module.display_texture("my texture.png", 150, 200)  # should display "my texture.png" at x=150 and y=200
```

Happy coding !
