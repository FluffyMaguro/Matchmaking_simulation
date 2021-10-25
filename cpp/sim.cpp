#define PY_SSIZE_T_CLEAN
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSIO
#include <future>
#include <Python.h>
#include <numpy/arrayobject.h>
#include "main.h"
#include "mutils.h"

static char module_docstring[] =
    "Module simulating various strategies for matchmaking";

// Helper function that creates a dictionary of all player data
PyObject *get_player_data(Player &p)
{
    // Create numpy objects
    npy_intp m = p.opponent_history->size();
    PyObject *opponent_history = PyArray_SimpleNewFromData(1, &m, NPY_DOUBLE, &((*p.opponent_history)[0]));
    PyObject *mmr_history = PyArray_SimpleNewFromData(1, &m, NPY_DOUBLE, &((*p.mmr_history)[0]));
    PyObject *predicted_chances = PyArray_SimpleNewFromData(1, &m, NPY_DOUBLE, &((*p.predicted_chances)[0]));
    // release unique_ptrs so they won't delete data
    p.opponent_history.release();
    p.mmr_history.release();
    p.predicted_chances.release();

    // Build a final player object
    return Py_BuildValue("{sdsdsOsOsO}", "skill", p.skill, "mmr", p.mmr, "opponent_history", opponent_history, "mmr_history", mmr_history, "predicted_chances", predicted_chances);
}

// Initialize and run simulation based on given arguments
Simulation initialize_simulation(PyObject *args)
{
    // simulation parameters and three strategy parameters
    int iterations, players;
    int sp1 = -1;
    int sp2 = -1;
    int sp3 = -1;
    double sp4 = -1;
    const char *strategy_type = "default";
    if (!PyArg_ParseTuple(args, "ii|siiid", &players, &iterations, &strategy_type, &sp1, &sp2, &sp3, &sp4))
        return NULL;

    // Run simulation
    return run_sim(players, iterations, sp1, sp2, sp3, sp4, strategy_type);
}

// Creates a numpy array from a vector of doubles
PyObject *get_np_array(std::vector<double> vect)
{
    double *carray = new double[vect.size()];
    std::copy(vect.begin(), vect.end(), carray);
    npy_intp m = vect.size();
    return PyArray_SimpleNewFromData(1, &m, NPY_DOUBLE, carray);
}

// Creates a numpy array from a vector reference of doubles
// Uses less memory as there is no data copy, but we need to make sure the owner of the vector loses its pointer
PyObject *get_np_array_from_pointer(std::vector<double> *vect)
{
    double *carray = &((*vect)[0]);
    npy_intp m = vect->size();
    return PyArray_SimpleNewFromData(1, &m, NPY_DOUBLE, carray);
}

// Runs simulation and returns its data
static PyObject *run_simulation(PyObject *self, PyObject *args)
{
    Simulation sim = initialize_simulation(args);
    Timeit t;
    // Get data for players
    PyObject *Result_Players = PyList_New(0);
    for (Player &p : sim.players)
        PyList_Append(Result_Players, Py_BuildValue("O", get_player_data(p)));

    // Here we first create numpy arrays based on the pointer of arrays
    // And then release the pointer so the data isn't destroyed when this function ends
    // Data is instead managed by the numpy array
    PyObject *Result_Predictions = get_np_array_from_pointer(sim.prediction_difference.get());
    PyObject *Result_MatchAccuracy = get_np_array_from_pointer(sim.match_accuracy.get());
    sim.prediction_difference.release();
    sim.match_accuracy.release();

    PyObject *Result = PyList_New(0);
    PyList_Append(Result, Result_Players);
    PyList_Append(Result, Result_Predictions);
    PyList_Append(Result, Result_MatchAccuracy);

    print("Creating Python objects for players finished in", t.s(), "seconds");
    // delete sim; // This is not necessary because we are using unique_ptr class and
    // that automatically deletes value at pointer location when not used by anything
    return Result;
}

// Runs parameter optimization and returns its data
static PyObject *run_parameter_optimization(PyObject *self, PyObject *args)
{
    Simulation sim = initialize_simulation(args);
    // Get prediction sums
    const int LATE_GAMES = 1000;
    double match_sum = 0;
    double match_sum_late = 0;
    for (int i = 0; i < sim.prediction_difference->size(); i++)
    {
        match_sum += (*(sim.prediction_difference))[i];
        if (i + LATE_GAMES >= sim.prediction_difference->size())
            match_sum_late += (*(sim.prediction_difference))[i];
    }

    PyObject *Result = PyList_New(0);
    PyList_Append(Result, Py_BuildValue("f", match_sum / 100000));
    PyList_Append(Result, Py_BuildValue("f", match_sum_late / LATE_GAMES));
    return Result;
}

std::vector<double> parameter_optimization_worker(int players, int iterations, int sp1, int sp2, int sp3, double sp4, const std::string &strategy_type, int LATE_GAMES)
{
    double match_sum = 0;
    double match_sum_late = 0;

    // Run simulation
    Simulation sim = run_sim(players, iterations, sp1, sp2, sp3, sp4, strategy_type);

    // Get prediction sums
    for (int i = 0; i < sim.prediction_difference->size(); i++)
    {
        match_sum += (*(sim.prediction_difference))[i];
        if (i + LATE_GAMES >= sim.prediction_difference->size())
            match_sum_late += (*(sim.prediction_difference))[i];
    }
    std::vector<double> out = {match_sum, match_sum_late};
    return out;
}

// Runs parameter optimization multithreaded and returns its data
static PyObject *run_parameter_optimization_nt(PyObject *self, PyObject *args)
{
    const int ITERATIONS = 3;
    const int LATE_GAMES = 1000;
    double total_match_sum = 0;
    double total_match_sum_late = 0;

    // Parse python data here
    int iterations, players;
    int sp1 = -1;
    int sp2 = -1;
    int sp3 = -1;
    double sp4 = -1;
    const char *strategy_type = "default";
    if (!PyArg_ParseTuple(args, "ii|siiid", &players, &iterations, &strategy_type, &sp1, &sp2, &sp3, &sp4))
        return NULL;

    // Release GIL here, doesn't hurt Python multiprocessing and improves
    // performance when using Python multithreading
    Py_BEGIN_ALLOW_THREADS
        std::vector<std::future<std::vector<double>>>
            futures;

    for (int iter = 0; iter < ITERATIONS; iter++)
        futures.push_back(std::async(std::launch::async, parameter_optimization_worker, players, iterations, sp1, sp2, sp3, sp4, strategy_type, LATE_GAMES));

    std::vector<double> results;
    for (auto &f : futures)
    {
        results = f.get();
        total_match_sum += results[0];
        total_match_sum_late += results[1];
    }

    total_match_sum /= ITERATIONS * 100000;
    total_match_sum_late /= ITERATIONS * LATE_GAMES;
    Py_END_ALLOW_THREADS

        PyObject *Result = PyList_New(0);
    PyList_Append(Result, Py_BuildValue("f", total_match_sum));
    PyList_Append(Result, Py_BuildValue("f", total_match_sum_late));
    return Result;
}

// TESTING CALLING PYTHON FUNCTION

static PyObject *my_python_function = NULL;

static PyObject *
set_my_python_function(PyObject *dummy, PyObject *args)
{
    PyObject *result = NULL;
    PyObject *temp;

    if (PyArg_ParseTuple(args, "O:set_callback", &temp))
    {
        if (!PyCallable_Check(temp))
        {
            PyErr_SetString(PyExc_TypeError, "parameter must be callable");
            return NULL;
        }
        Py_XINCREF(temp);               /* Add a reference to new callback */
        Py_XDECREF(my_python_function); /* Dispose of previous callback */
        my_python_function = temp;      /* Remember new callback */
        /* Boilerplate to return "None" */
        Py_INCREF(Py_None);
        result = Py_None;
    }
    return result;
}

static PyObject *get_c_twice(PyObject *self, PyObject *args)
{
    PyObject *arglist = Py_BuildValue("(i)", 123);
    PyObject *result = PyObject_CallObject(my_python_function, arglist);
    Py_DECREF(arglist);

    // Pass error back if there was one
    if (result == NULL)
        return NULL;

    // Decref result after parsing it
    // Py_DECREF(result);

    // Or return it (this usually wouldn't make sense right away)
    return result;
}

/* Module methods (how it's called for python | how it's called here | arg-type | docstring) METH_VARARGS/METH_KEYWORDS/METH_NOARGS */
static PyMethodDef module_methods[] = {
    {"run_simulation", run_simulation, METH_VARARGS, "Runs a simulation with `players` and `iterations`"},
    {"run_parameter_optimization", run_parameter_optimization, METH_VARARGS, "Runs parameter optimization`"},
    {"run_parameter_optimization_nt", run_parameter_optimization_nt, METH_VARARGS, "Runs parameter optimization NT`"},
    {"set_my_python_function", set_my_python_function, METH_VARARGS, "set_my_python_function doc"},
    {"get_c_twice", get_c_twice, METH_VARARGS, "METH_VARARGS doc"},
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