#include "trueskill.h"
#include "mutils.h"

// Python function used to update player data after a match
PyObject *trueskill_rate_1v1;

// Links Python function to the pointer above. This needs to be done from Python
PyObject *set_trueskill_rate_1v1(PyObject *dummy, PyObject *args)
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
        Py_XINCREF(temp);               // Add a reference to new callback
        Py_XDECREF(trueskill_rate_1v1); // Dispose of previous callback
        trueskill_rate_1v1 = temp;      // Remember new callback

        // Boilerplate to return "None"
        Py_INCREF(Py_None);
        result = Py_None;
    }
    return result;
}

// Calls the Python function to update given pair according to the trueskill algorithm
match_pair trueskill_update(match_pair pair)
{
    PyObject *arglist = Py_BuildValue("(ddddi)", pair.winner_mu, pair.winner_sigma, pair.loser_mu, pair.loser_sigma, pair.draw);
    PyObject *result = PyObject_CallObject(trueskill_rate_1v1, arglist);
    Py_DECREF(arglist);
    match_pair new_data;
    if (!PyArg_ParseTuple(result, "dddd", &new_data.winner_mu, &new_data.winner_sigma, &new_data.loser_mu, &new_data.loser_sigma))
        print("ERROR: Failed to parse tuple");
    if (result == NULL)
        print("ERROR: Failed to parse tuple");
    Py_DecRef(result);
    return new_data;
}