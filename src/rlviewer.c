#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdint.h>

#include "numpy/arrayobject.h" // Include any other Numpy headers, UFuncs for example.
#include "viewer.h"

static PyObject *rlviewer_load(PyObject *self, PyObject *args);
static PyObject *rlviewer_grab(PyObject *self, PyObject *args);
static PyObject *rlviewer_set_light(PyObject *self, PyObject *args);

static uint8_t *pixels = NULL;
static float *depth = NULL;
static const int dimensions_rgb = 3;
static npy_intp shape_rgb[3] = { 768, 1024, 3 };
static const int dimensions_mask = 2;
static npy_intp shape_mask[3] = { 768, 1024 };

static PyMethodDef RLViewerMethods[] = {
        {"load",  rlviewer_load, METH_VARARGS, "XYZ"},
        {"grab",  rlviewer_grab, METH_VARARGS, "XYZ"},
        {"set_light",  rlviewer_set_light, METH_VARARGS, "XYZ"},
        {NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct PyModuleDef RLViewModule = {
        PyModuleDef_HEAD_INIT,
        "rlviewer",   /* name of module */
        NULL, /* module documentation, may be NULL */
        -1,   /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
        RLViewerMethods
};

PyMODINIT_FUNC
PyInit_rlviewer(void)
{
        assert(!PyErr_Occurred());
        import_array(); // Initialise Numpy
        if (PyErr_Occurred()) {
                return NULL;
        }

        int length = shape_rgb[0] * shape_rgb[1] * shape_rgb[2];
        pixels = (uint8_t *) malloc(length * sizeof(uint8_t));
        if (pixels == NULL) {
                return NULL;
        }
        
        length = shape_mask[0] * shape_mask[1];
        depth = (float *) malloc(length * sizeof(float));
        if (depth == NULL) {
                return NULL;
        }
        
        viewer_init(); 
        return PyModule_Create(&RLViewModule);
}

static PyObject *rlviewer_load(PyObject *self, PyObject *args)
{
        const char *path;
        int ret = 0;
        
        if (!PyArg_ParseTuple(args, "s", &path))
                return NULL;

        if (viewer_load(path) != 0)
                return NULL;

        return Py_BuildValue("i", ret);
}

static PyObject *rlviewer_grab(PyObject *self, PyObject *args)
{
        float r, lat, lon;
        
        if (!PyArg_ParseTuple(args, "fff", &r, &lat, &lon))
                return NULL;

        viewer_grab(pixels, r, lat, lon);

        // Convert it to a NumPy array.
        PyObject *pArray = PyArray_SimpleNewFromData(dimensions_rgb, shape_rgb, NPY_UBYTE,
                                                     (void *) pixels);
        
        return pArray;
}

static PyObject *rlviewer_set_light(PyObject *self, PyObject *args)
{
        int index;
        float x, y, z, power;
        
        if (!PyArg_ParseTuple(args, "iffff", &index, &x, &y, &z, &power))
                return NULL;

        viewer_set_light(index, x, y, z, power);

        Py_RETURN_NONE;
}

