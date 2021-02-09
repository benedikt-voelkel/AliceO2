#include <iostream>
#include "PyTest/PyWrap.h"

using namespace o2::python;

void PyWrap::run()
{
    std::cout << "Hello" << std::endl;
    Py_Initialize();
    PyObject *pName = PyUnicode_DecodeFSDefault("math");
    PyObject *pModule = PyImport_Import(pName);
    Py_DECREF(pName);
    PyObject *pFunc = PyObject_GetAttrString(pModule, "sqrt");

    PyObject *pArgs = PyTuple_New(1);
    PyObject *pValue = PyLong_FromLong(4);
    PyTuple_SetItem(pArgs, 0, pValue);

    pValue = PyObject_CallObject(pFunc, pArgs);
    Py_DECREF(pArgs);
    std::cout << PyLong_AsLong(pValue) << std::endl;
    Py_DECREF(pValue);
    Py_XDECREF(pFunc);
    Py_DECREF(pModule);
    std::cout << "Bye" << std::endl;

}

