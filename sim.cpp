#define PY_SSIZE_T_CLEAN
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSIO
#include <Python.h>
#include <numpy/arrayobject.h>
#include "main.cpp" // dirty direct include
#include "mutils.h" // random helper functions

static char module_docstring[] =
    "Module simulating various strategies for matchmaking";

// Helper function that creates a dictionary of all player data
PyObject *get_player_data(const Player &p)
{
    // All arrays of the same size
    size_t size = p.opponent_history.size();

    // Allocate data for c-arrays
    double *c_opponent_history = new double[size];
    double *c_mmr_history = new double[size];
    double *c_predicted_chances = new double[size];

    // Copy from vectors into c-arrays
    std::copy(p.opponent_history.begin(), p.opponent_history.end(), c_opponent_history);
    std::copy(p.mmr_history.begin(), p.mmr_history.end(), c_mmr_history);
    std::copy(p.predicted_chances.begin(), p.predicted_chances.end(), c_predicted_chances);

    // Create numpy objects
    npy_intp m = size;
    PyObject *opponent_history = PyArray_SimpleNewFromData(1, &m, NPY_DOUBLE, c_opponent_history);
    PyObject *mmr_history = PyArray_SimpleNewFromData(1, &m, NPY_DOUBLE, c_mmr_history);
    PyObject *predicted_chances = PyArray_SimpleNewFromData(1, &m, NPY_DOUBLE, c_predicted_chances);

    // Build a final player object
    return Py_BuildValue("{sdsdsOsOsO}", "skill", p.skill, "mmr", p.mmr, "opponent_history", opponent_history, "mmr_history", mmr_history, "predicted_chances", predicted_chances);
}

// Runs simulation and returns its data
static PyObject *run_simulation(PyObject *self, PyObject *args)
{
    int iterations, players;
    if (!PyArg_ParseTuple(args, "ii", &players, &iterations))
        return NULL;

    // Run simulation
    std::vector<Player> &player_data = run_sim(players, iterations);

    // Prepare data to be send away
    Timeit t;
    PyObject *Data = PyList_New(0);
    for (const Player &p : player_data)
    {
        PyList_Append(Data, Py_BuildValue("O", get_player_data(p)));
    }
    print("Creating Python objects for players finished in", t.s(), "seconds");
    return Data;
}

// Exports prediction diff as a numpy array
static PyObject *export_prediction_diff(PyObject *self, PyObject *args)
{
    std::vector<double> &prediction_diff = get_prediction_diff();
    // Allocate memory for a new array
    double *carray = new double[prediction_diff.size()];
    // Copy values to it
    std::copy(prediction_diff.begin(), prediction_diff.end(), carray);
    // Specify its dimension/size in them
    npy_intp m = prediction_diff.size();
    // Create numpy array and return
    return PyArray_SimpleNewFromData(1, &m, NPY_DOUBLE, carray);
}

/* Module methods (how it's called for python | how it's called here | arg-type | docstring) METH_VARARGS/METH_KEYWORDS/METH_NOARGS */
static PyMethodDef module_methods[] = {
    {"run_simulation", run_simulation, METH_VARARGS, "Runs a simulation with `players` and `iterations`"},
    {"export_prediction_diff", export_prediction_diff, METH_NOARGS, "get prediction differences (numpy array)"},
    {NULL, NULL, 0, NULL} // Last needs to be this
};

/* Initialize the module. Use to correct module name: psimulation here*/
PyMODINIT_FUNC PyInit_psimulation(void)
{
    static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "psimulation",
        module_docstring,
        -1,
        module_methods,
        NULL,
        NULL,
        NULL,
        NULL};

    import_array(); // necessary for numpy initialization
    return PyModule_Create(&moduledef);
}