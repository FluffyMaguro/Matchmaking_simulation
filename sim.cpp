#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "main.cpp" // dirty direct include
#include "mutils.h" // random helper functions

static char module_docstring[] =
    "Module simulating various strategies for matchmaking";

// Declare first functions
static PyObject *run_simulation(PyObject *self, PyObject *args);
static PyObject *export_prediction_diff(PyObject *self, PyObject *args);

/* Module methods (how it's called for python | how it's called here | arg-type | docstring) */
static PyMethodDef module_methods[] = {
    {"run_simulation", run_simulation, METH_VARARGS, "Runs a simulation with `players` and `iterations`"},
    {"export_prediction_diff", export_prediction_diff, METH_NOARGS, "get prediction differences"},
    {NULL, NULL, 0, NULL} // Last needs to be this
};

/*
METH_VARARGS - typical
METH_KEYWORDS - also keywords
METH_NOARGS  - no arguments
*/

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

    return PyModule_Create(&moduledef);
}

// Helper function
// Creares a dictionary of all player data
PyObject *get_player_data(const Player &p)
{
    PyObject *opponent_history = PyList_New(0);
    PyObject *mmr_history = PyList_New(0);
    PyObject *predicted_chances = PyList_New(0);

    for (int i = 0; i < p.opponent_history.size(); i++)
    {
        PyList_Append(opponent_history, Py_BuildValue("d", p.opponent_history[i]));
        PyList_Append(mmr_history, Py_BuildValue("d", p.mmr_history[i]));
        PyList_Append(predicted_chances, Py_BuildValue("d", p.predicted_chances[i]));
    }
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
    print("Creating Python objects for players finished in", t.ms() / 1000, "seconds");
    return Data;
}

static PyObject *export_prediction_diff(PyObject *self, PyObject *args)
{
    std::vector<double> &prediction_diff = get_prediction_diff();
    PyObject *pylist = PyList_New(0);
    for (const double &i : prediction_diff)
    {
        PyList_Append(pylist, Py_BuildValue("d", i));
    }
    return pylist;
}

// Now that all mess is initialized, lets look at actual functions

/* Python type parsing:

c   char            A Python string of length 1 becomes a C char.
d   double          A Python float becomes a C double.
f   float           A Python float becomes a C float.
i   int             A Python int becomes a C int.
l   long            A Python int becomes a C long.
L   long long       A Python int becomes a C long long
O   PyObject*       Gets non-NULL borrowed reference to Python argument.
s   char*           Python string without embedded nulls to C char*.
s#  char*+int       Any Python string to C address and length.
t#  char*+int       Read-only single-segment buffer to C address and length.
u   Py_UNICODE*     Python Unicode without embedded nulls to C.
u#  Py_UNICODE*+int     Any Python Unicode C address and length.
w#  char*+int       Read/write single-segment buffer to C address and length.
z   char*           Like s, also accepts None (sets C char* to NULL).
z#  char*+int       Like s#, also accepts None (sets C char* to NULL).
(...)               as per ...  A Python sequence is treated as one argument per item.
|                   The following arguments are optional.
:                   Format end, followed by function name for error messages.
;                   Format end, followed by entire error message text.

Python type returning:

c   char            A C char becomes a Python string of length 1.
d   double          A C double becomes a Python float.
f   float           A C float becomes a Python float.
i   int             A C int becomes a Python int.
l   long            A C long becomes a Python int.
N   PyObject*       Passes a Python object and steals a reference.
O   PyObject*       Passes a Python object and INCREFs it as normal.
O&  convert+void*   Arbitrary conversion
s   char*           C 0-terminated char* to Python string, or NULL to None.
s#  char*+int       C char* and length to Python string, or NULL to None.
u   Py_UNICODE*     C-wide, null-terminated string to Python Unicode, or NULL to None.
u#  Py_UNICODE*+int C-wide string and length to Python Unicode, or NULL to None.
w#  char*+int       Read/write single-segment buffer to C address and length.
z   char*           Like s, also accepts None (sets C char* to NULL).
z#  char*+int       Like s#, also accepts None (sets C char* to NULL).
(...)               as per ...  Builds Python tuple from C values.
[...]               as per ...  Builds Python list from C values.
{...}               as per ...  Builds Python dictionary from C values, alternating keys and values.

*/