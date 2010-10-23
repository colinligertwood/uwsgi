#include "../../uwsgi.h"
#include <Python.h>


#define MAX_PYTHONPATH 64

#ifdef UWSGI_STACKLESS
#include <stackless_api.h>
#endif

#if PY_MINOR_VERSION == 4 && PY_MAJOR_VERSION == 2
#define Py_ssize_t ssize_t
#endif

#if PY_MAJOR_VERSION > 2
#define PYTHREE
#endif

#ifdef UWSGI_THREADING
#define UWSGI_GET_GIL (*up.gil_get)();
#define UWSGI_RELEASE_GIL (*up.gil_release)();
#else
#define UWSGI_GET_GIL
#define UWSGI_RELEASE_GIL
#endif


PyAPI_FUNC(PyObject *) PyMarshal_WriteObjectToString(PyObject *, int);
PyAPI_FUNC(PyObject *) PyMarshal_ReadObjectFromString(char *, Py_ssize_t);

#ifdef PYTHREE
#define UWSGI_PYFROMSTRING(x) PyUnicode_FromString(x)
#define UWSGI_PYFROMSTRINGSIZE(x, y) PyUnicode_FromStringAndSize(x, y)
#define PyInt_FromLong	PyLong_FromLong
#define PyInt_AsLong	PyLong_AsLong
#define PyInt_Check	PyLong_Check
#define PyString_Check	PyBytes_Check
#define	PyString_FromStringAndSize	PyBytes_FromStringAndSize
#define	PyString_FromFormat	PyBytes_FromFormat
#define	PyString_FromString	PyBytes_FromString
#define	PyString_Size		PyBytes_Size
#define	PyString_Concat		PyBytes_Concat
#define	PyString_AsString	(char *) PyBytes_AsString
#define PyFile_FromFile(A,B,C,D) PyFile_FromFd(fileno((A)), (B), (C), -1, NULL, NULL, NULL, 0)

#else
#define UWSGI_PYFROMSTRING(x) PyString_FromString(x)
#define UWSGI_PYFROMSTRINGSIZE(x, y) PyString_FromStringAndSize(x, y)
#endif

struct uwsgi_python {

        char *home;
        int optimize;

	char *argv;
	int argc;

        PyObject *wsgi_spitout;
        PyObject *wsgi_writeout;

        PyThreadState *main_thread;

        char *test_module;

	char *python_path[MAX_PYTHONPATH];
	int python_path_cnt;

	PyObject *loader_dict;
	PyObject* (*loaders[LOADER_MAX]) (void *);

	char *wsgi_config;
	char *file_config;
	char *paste;
	char *eval;


	char *callable;

	int ignore_script_name;
	int catch_exceptions;


#ifdef UWSGI_THREADING
	pthread_key_t upt_save_key;
	pthread_mutex_t lock_pyloaders;
        void (*gil_get) (void);
        void (*gil_release) (void);
#endif
};



void init_uwsgi_vars(void);
void init_uwsgi_embedded_module(void);


void uwsgi_wsgi_config(char *);
#ifdef UWSGI_PASTE
void uwsgi_paste_config(char *);
#endif
void uwsgi_file_config(char *);
void uwsgi_eval_config(char *);

int init_uwsgi_app(int, void *, struct wsgi_request *wsgi_req, int);


PyObject *py_eventfd_read(PyObject *, PyObject *) ;
PyObject *py_eventfd_write(PyObject *, PyObject *) ;


#ifdef UWSGI_STACKLESS
PyObject *py_uwsgi_stackless(PyObject *, PyObject *) ;
#endif

int manage_python_response(struct wsgi_request *);
int uwsgi_python_call(struct wsgi_request *, PyObject *, PyObject *);
PyObject *python_call(PyObject *, PyObject *, int);

#ifdef UWSGI_SENDFILE
PyObject *py_uwsgi_sendfile(PyObject *, PyObject *) ;
ssize_t uwsgi_sendfile(struct wsgi_request *);
ssize_t uwsgi_do_sendfile(int, int, size_t, size_t, off_t*, int);
#endif

PyObject *py_uwsgi_write(PyObject *, PyObject *) ;
PyObject *py_uwsgi_spit(PyObject *, PyObject *) ;

#ifdef UWSGI_STACKLESS
struct stackless_req {
	PyTaskletObject *tasklet;
	struct wsgi_request *wsgi_req;
	PyChannelObject *channel;
};
struct wsgi_request *find_request_by_tasklet(PyTaskletObject *);

void stackless_init(void);
void stackless_loop(void);
#endif

void init_pyargv(void);

#ifdef UWSGI_WEB3
void *uwsgi_request_subhandler_web3(struct wsgi_request *, struct uwsgi_app *);
int uwsgi_response_subhandler_web3(struct wsgi_request *);
#endif

PyObject *uwsgi_uwsgi_loader(void *);
PyObject *uwsgi_dyn_loader(void *);
PyObject *uwsgi_file_loader(void *);
PyObject *uwsgi_eval_loader(void *);
PyObject *uwsgi_paste_loader(void *);
PyObject *uwsgi_callable_loader(void *);
PyObject *uwsgi_string_callable_loader(void *);
PyObject *uwsgi_mount_loader(void *);

char *get_uwsgi_pymodule(char *);
PyObject *get_uwsgi_pydict(char *);

int uwsgi_request_wsgi(struct wsgi_request *);
void uwsgi_after_request_wsgi(struct wsgi_request *);

void *uwsgi_request_subhandler_wsgi(struct wsgi_request *, struct uwsgi_app*);
int uwsgi_response_subhandler_wsgi(struct wsgi_request *);

void gil_real_get(void);
void gil_real_release(void);
void gil_fake_get(void);
void gil_fake_release(void);