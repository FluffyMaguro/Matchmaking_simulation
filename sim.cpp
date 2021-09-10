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

// Initialize and run simulation based on given arguments
std::unique_ptr<Simulation> initialize_simulation(PyObject *args)
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
    std::string strategy_type_s(strategy_type);
    return run_sim(players, iterations, sp1, sp2, sp3, sp4, strategy_type_s);
}

// Creates a numpy array from a vector of doubles
PyObject *get_np_array(std::vector<double> vect)
{
    double *carray = new double[vect.size()];
    std::copy(vect.begin(), vect.end(), carray);
    npy_intp m = vect.size();
    return PyArray_SimpleNewFromData(1, &m, NPY_DOUBLE, carray);
}

// Runs simulation and returns its data
static PyObject *run_simulation(PyObject *self, PyObject *args)
{
    std::unique_ptr<Simulation> sim = initialize_simulation(args);
    Timeit t;

    // Get data for players
    PyObject *Result_Players = PyList_New(0);
    for (const Player &p : sim->players)
        PyList_Append(Result_Players, Py_BuildValue("O", get_player_data(p)));

    PyObject *Result_Predictions = get_np_array(sim->prediction_difference);
    PyObject *Result_MatchAccuracy = get_np_array(sim->match_accuracy);

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
    std::unique_ptr<Simulation> sim = initialize_simulation(args);
    // Get prediction sums
    const int LATE_GAMES = 1000;
    double pred_sum = 0;
    double pred_sum_late = 0;
    for (int i = 0; i < sim->prediction_difference.size(); i++)
    {
        pred_sum += sim->prediction_difference[i];
        if (i + LATE_GAMES >= sim->prediction_difference.size())
            pred_sum_late += sim->prediction_difference[i];
    }

    PyObject *Result = PyList_New(0);
    PyList_Append(Result, Py_BuildValue("f", pred_sum / 100000));
    PyList_Append(Result, Py_BuildValue("f", pred_sum_late / LATE_GAMES));
    return Result;
}

/* Module methods (how it's called for python | how it's called here | arg-type | docstring) METH_VARARGS/METH_KEYWORDS/METH_NOARGS */
static PyMethodDef module_methods[] = {
    {"run_simulation", run_simulation, METH_VARARGS, "Runs a simulation with `players` and `iterations`"},
    {"run_parameter_optimization", run_parameter_optimization, METH_VARARGS, "Runs parameter optimization`"},
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