static PyObject *syslog_syslog ( PyObject *self , PyObject *args );
#ifndef INET225
static PyObject *syslog_openlog ( PyObject *self , PyObject *args );
static PyObject *syslog_closelog ( PyObject *self , PyObject *args );
static PyObject *syslog_setlogmask ( PyObject *self , PyObject *args );
static PyObject *syslog_log_mask ( PyObject *self , PyObject *args );
static PyObject *syslog_log_upto ( PyObject *self , PyObject *args );
#endif
static void ins ( PyObject *d , char *s , long x );
