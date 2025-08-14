static void reap_obj ( pylist fd2obj [FD_SETSIZE +3 ]);
static int list2set ( PyObject *list , fd_set *set , pylist fd2obj [FD_SETSIZE +3 ]);
static PyObject *set2list ( fd_set *set , pylist fd2obj [FD_SETSIZE +3 ]);
static PyObject *select_select ( PyObject *self , PyObject *args );
