#include <iostream>
#include <fstream>
#include <stdio.h>

#include "scripting.hpp"

#ifdef PLATFORM_WIN
#include <windows.h>
#endif // PLATFORM_WIN

#ifdef PLATFORM_POSIX
#include <dirent.h>
#endif // PLATFORM_POSIX

#define RETURN_NONE Py_INCREF(Py_None); return Py_None;

#define DEBUG
#define AUTO_IMPORT_MODULE
#define SCRIPTS_DIRECTORY L"scripts_directory"  // do not include final '/'

inline bool file_exists (const std::string& name)
{
    if (FILE *file = fopen(name.c_str(), "r"))
    {
        fclose(file);
        return true;
    }
    else
        return false;
}

namespace PyModule
{
    extern "C"
    {
        const char* name = "Module";

        // errors
        static PyObject* ModuleError;

        // module functions
        static PyObject* test(PyObject* self, PyObject* args)
        {
            int x = 0;

            if (!PyArg_ParseTuple(args, "i", &x))
            {
                PyErr_SetString(ModuleError, "Can not parse arguments");
                return NULL;
            }

            x *= PyScripting::getValue();
            // and we can also change the value from here !
            PyScripting::setValue(PyScripting::getValue() + 1);

            return Py_BuildValue("i", x);
        }

        // module definition
        static PyMethodDef ModuleMethods[] = {
            // ...
            {"test", test, METH_VARARGS, "Return a number time PyScripting::getValue()"},
            // ...
            {NULL, NULL, 0, NULL}  // sentinel
        };
        static PyModuleDef ModuleModule = {
            PyModuleDef_HEAD_INIT,
            name,
            "C++ functions to modify the way some components of the project works",
            -1,
            ModuleMethods
        };

        // init the module
        PyMODINIT_FUNC PyInit_Module(void)
        {
            PyObject* m;

            m = PyModule_Create2(&ModuleModule, PYTHON_API_VERSION);
            if (m == NULL)
                return NULL;

            ModuleError = PyErr_NewException("Module.error", NULL, NULL);
            Py_INCREF(ModuleError);
            PyModule_AddObject(m, "error", ModuleError);

            return m;
        }
    }
}

PyScripting PyScripting::instance = PyScripting();

PyScripting::PyScripting():
    connected(false)
    , program(L"PyScripter engine")
{
    Py_SetProgramName(this->program);
}

PyScripting& PyScripting::Instance()
{
    return instance;
}

void PyScripting::load_all_modules()
{
    std::string directory = SCRIPTS_DIRECTORY;

    #ifdef PLATFORM_WIN
    WIN32_FIND_DATA File;
    HANDLE hSearch;

    hSearch = FindFirstFile((directory + "/*.py").data(), &File);
    if (hSearch != INVALID_HANDLE_VALUE)
    {
        do {
                if (std::string(File.cFileName) != "." && std::string(File.cFileName) != ".." && file_exists(std::string(File.cFileName)))
                    this->modules_names.push_back(directory + "/" + std::string(File.cFileName));
        } while (FindNextFile(hSearch, &File));

        FindClose(hSearch);
    }
    #endif // PLATFORM_WIN

    #ifdef PLATFORM_POSIX
    DIR* rep = opendir(directory);

    if (rep != NULL)
    {
        struct dirent* ent;

        while ((ent = readdir(rep)) != NULL)
        {
            if (file_exists(std::string(ent->d_name)))
                this->modules_names.push_back(directory + "/" + std::string(ent->d_name));
        }

        closedir(rep);
    }
    #endif // PLATFORM_POSIX

    for (auto& fname: this->modules_names)
    {
        std::ifstream file;
        file.open(fname);

        #ifdef DEBUG
        std::cout << "Loading " << fname << std::endl;
        #endif // DEBUG

        std::string content;
        std::string line;

        while (file)
        {
            getline(file, line);

            content += line;
            content += "\n";
        }

        file.close();

        this->modules_content.push_back(content);
    }
}

// static methods
bool PyScripting::connect()
{
    if (!instance.connected)
    {
        #ifdef DEBUG
        std::cout << "Connecting PyScripting to Python (3) interpreter" << std::endl;
        #endif // DEBUG
        instance.connected = true;

        if (PyImport_AppendInittab(PyModule::name, PyModule::PyInit_Module) != -1)
        {
            #ifdef DEBUG
            std::cout << "Can not add the module to the init tab" << std::endl;
            #endif // DEBUG
            instance.connected = false;
        }
        Py_Initialize();
        Py_SetPythonHome(SCRIPTS_DIRECTORY);
        #ifdef AUTO_IMPORT_MODULE
        PyImport_ImportModule(PyModule::name);
        #endif // AUTO_IMPORT_MODULE

        #ifdef DEBUG
        if (instance.connected)
            std::cout << "PyScripting is connected to Python (3) interpreter" << std::endl;
        else
            std::cout << "An error occured while trying to add the C++ module" << std::endl;
        #endif // DEBUG

        if (instance.connected)
            instance.load_all_modules();
        else
        {
            #ifdef DEBUG
            std::cout << "Can not load modules, PyScripting is not connected to Python (3) interpreter" << std::endl;
            #endif // DEBUG
        }

        return true;
    }
    return false;  // already running
}

bool PyScripting::disconnect()
{
    if (instance.connected)
    {
        instance.connected = false;
        #ifdef DEBUG
        std::cout << "Undoing initializations made by Py_Initialize() -- freeing memory" << std::endl;
        #endif // DEBUG
        Py_Finalize();

        return true;
    }
    return false;  // already disconnected
}

int PyScripting::run_code(const char* code)
{
    if (instance.connected)
        return PyRun_SimpleString(code);
    #ifdef DEBUG
    std::cout << "You need to connect PyScripting to Python (3) before using it !" << std::endl;
    #endif // DEBUG
    return -1;
}

int PyScripting::run_all_modules()
{
    int i = 0;

    for (auto& module_code: instance.modules_content)
    {
        instance.run_code(module_code.data());
        i++;
    }
    #ifdef DEBUG
    std::cout << "Ran " << i << " script(s) one time" << std::endl;
    #endif // DEBUG

    return 1;
}

// for the example ONLY
void PyScripting::setValue(int val)
{
    instance.value = val;
}

int PyScripting::getValue()
{
    return instance.value;
}
