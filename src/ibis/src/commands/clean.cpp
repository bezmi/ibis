#include <iostream>
#include <Python.h>
#include <spdlog/spdlog.h>
#include "clean.h"

int clean(int argc, char* argv[]) {
    std::string clean_path = std::string(std::getenv("IBIS")) + "/lib";

    Py_Initialize();


    PyRun_SimpleString("import sys");
    std::string import_string = "sys.path.append('" + clean_path + "')";
    PyRun_SimpleString(import_string.c_str());

    PyObject* clean_module = PyImport_ImportModule("clean");
    if (clean_module == NULL) {
        PyErr_Print();
        Py_Finalize();
        return 1;
    }
    
    PyObject *py_clean_main = PyObject_GetAttrString(clean_module, "main");
    if (py_clean_main == NULL) {
        PyErr_Print();
        Py_DECREF(clean_module);
        Py_Finalize();
        return 1;
    }

    PyObject* main_result = PyObject_CallObject(py_clean_main, NULL);
    if (main_result == NULL) {
        PyErr_Print();
    }
    
    Py_DECREF(py_clean_main);
    Py_DECREF(clean_module);
    
    Py_Finalize();

    spdlog::info("Cleaned directory");
    return 0;
}
