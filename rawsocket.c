/*
 * Python module to create raw sockets as unprivileged user.
 *
 * Copyright (c) 2015, Michael Walle <michael@walle.cc>
 * See LICENSE for licensing terms.
 */

#include <Python.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#define RAW_SOCK_HELPER "rawsocket-helper"

static int spawn_helper(int fd)
{
	int status;
	pid_t pid;

	pid = fork();
	if (pid < 0) {
		PyErr_SetFromErrno(PyExc_IOError);
		return -1;
	}

	if (pid == 0) {
		/* child */
		char fd_str[8];
		char *argv[] = { RAW_SOCK_HELPER, fd_str, NULL };
		char *pathenv = NULL;
		char *envp[] = { pathenv, NULL };
		char *path = getenv("PATH");

		if (path) {
			pathenv = malloc(6+strlen(path));
			if (!pathenv) {
				exit(1);
			}
			sprintf(pathenv, "PATH=%s", path);
			envp[0] = pathenv;
		}

		snprintf(fd_str, sizeof(fd_str), "%d", fd);
		execvpe(argv[0], argv, envp);
		exit(1);
	}

	waitpid(pid, &status, 0);
	if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		return 0;

	PyErr_Format(PyExc_IOError, "Could not spawn helper '%s' (not SUID root?).",
			RAW_SOCK_HELPER);

	return -1;
}

static int raw_socket(void)
{
	int sv[2];
	int rc;
	struct iovec iov;
	struct msghdr msg = {0};
	struct cmsghdr *cmsg;
	char buf[CMSG_SPACE(sizeof(int))];
	char dummy;

	/* dummy read */
	iov.iov_base = &dummy;
	iov.iov_len = 1;

	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_control = buf;
	msg.msg_controllen = sizeof(buf);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	/* create communication socket */
	rc = socketpair(AF_UNIX, SOCK_DGRAM, 0, sv);
	if (rc == -1) {
		PyErr_SetFromErrno(PyExc_IOError);
		return -1;
	}

	/* fork our privileged helper program */
	rc = spawn_helper(sv[1]);
	if (rc == -1)
		return -1;

	/* receive the privileged socket */
	rc = recvmsg(sv[0], &msg, 0);
	if (rc == -1) {
		PyErr_SetFromErrno(PyExc_IOError);
		return -1;
	}

	cmsg = CMSG_FIRSTHDR(&msg);
	if (!cmsg
			|| cmsg->cmsg_len != CMSG_LEN(sizeof(int))
			|| cmsg->cmsg_level != SOL_SOCKET
			|| cmsg->cmsg_type != SCM_RIGHTS) {
		PyErr_SetFromErrno(PyExc_IOError);
		return -1;
	}

	close(sv[0]);
	close(sv[1]);

	return *((int *)CMSG_DATA(cmsg));
}

static PyObject *
rawsocket_fd(PyObject *self, PyObject *args)
{
	int fd;

	if (!PyArg_ParseTuple(args, ""))
		return NULL;

	fd = raw_socket();
	if (fd < 0)
		return NULL;

	return PyInt_FromLong(fd);
}

PyDoc_STRVAR(rawsocket_fd_doc,
"rawsocket_fd()\n\
\n\
Create a new raw socket using a privileged helper program.");

static PyMethodDef rawsocket_methods[] = {
	{"rawsocket_fd", rawsocket_fd,
	 METH_VARARGS, rawsocket_fd_doc},
	{NULL, NULL}
};

PyMODINIT_FUNC
initrawsocket(void)
{
	PyObject *m;

	m = Py_InitModule("rawsocket", rawsocket_methods);
	if (m == NULL)
		return;
}
