===============================
Retrieving data from the server
===============================


--------
SYNOPSIS
--------


#include <libmemcached/memcached.h>
 
.. c:function:: memcached_result_st * memcached_fetch_result (memcached_st *ptr, memcached_result_st *result, memcached_return_t *error);

.. c:function:: char * memcached_get (memcached_st *ptr, const char *key, size_t key_length, size_t *value_length, uint32_t *flags, memcached_return_t *error);

.. c:function::  memcached_return_t memcached_mget (memcached_st *ptr, const char * const *keys, const size_t *key_length, size_t number_of_keys);

.. c:function:: char * memcached_get_by_key (memcached_st *ptr, const char *group_key, size_t group_key_length, const char *key, size_t key_length, size_t *value_length, uint32_t *flags, memcached_return_t *error);

.. c:function:: memcached_return_t memcached_mget_by_key (memcached_st *ptr, const char *group_key, size_t group_key_length, const char * const *keys, const size_t *key_length, size_t number_of_keys);

.. c:function::  char * memcached_fetch (memcached_st *ptr, char *key, size_t *key_length, size_t *value_length, uint32_t *flags, memcached_return_t *error);

.. c:function::  memcached_return_t memcached_fetch_execute (memcached_st *ptr, memcached_execute_fn *callback, void *context, uint32_t number_of_callbacks);

.. c:function:: memcached_return_t memcached_mget_execute (memcached_st *ptr, const char * const *keys, const size_t *key_length, size_t number_of_keys, memcached_execute_fn *callback, void *context, uint32_t number_of_callbacks);

.. c:function:: memcached_return_t memcached_mget_execute_by_key (memcached_st *ptr, const char *group_key, size_t group_key_length, const char * const *keys, const size_t *key_length, size_t number_of_keys, memcached_execute_fn *callback, void *context, uint32_t number_of_callbacks);

Compile and link with -lmemcached


-----------
DESCRIPTION
-----------


memcached_get() is used to fetch an individual value from the server. You
must pass in a key and its length to fetch the object. You must supply
three pointer variables which will give you the state of the returned
object.  A uint32_t pointer to contain whatever flags you stored with the value,
a size_t pointer which will be filled with size of of the object, and a
memcached_return_t pointer to hold any error. The object will be returned
upon success and NULL will be returned on failure. Any object returned by
memcached_get() must be released by the caller application.

memcached_mget() is used to select multiple keys at once. For multiple key
operations it is always faster to use this function. This function always
works asynchronously. memcached_fetch() is then used to retrieve any keys
found. No error is given on keys that are not found. You must call either
memcached_fetch() or memcached_fetch_result() after a successful call to
memcached_mget(). You should continue to call these functions until they
return NULL (aka no more values). If you need to quit in the middle of a
memcached_get() call, execute a memcached_quit(). After you do this, you can
issue new queries against the server.

memcached_fetch() is used to fetch an individual value from the server.
memcached_mget() must always be called before using this method.  You
must pass in a key and its length to fetch the object. You must supply
three pointer variables which will give you the state of the returned
object.  A uint32_t pointer to contain whatever flags you stored with the value,
a size_t pointer which will be filled with size of of the object, and a
memcached_return_t pointer to hold any error. The object will be returned
upon success and NULL will be returned on failure. MEMCACHD_END is returned
by the \*error value when all objects that have been found are returned.
The final value upon MEMCACHED_END is null. Values returned by
memcached_fetch() musted be free'ed by the caller. memcached_fetch() will
be DEPRECATED in the near future, memcached_fetch_result() should be used
instead.

memcached_fetch_result() is used to return a memcached_result_st(3) structure
from a memcached server. The result object is forward compatible with changes
to the server. For more information please refer to the memcached_result_st(3)
help. This function will dynamically allocate a result structure for you
if you do not pass one to the function.

memcached_fetch_execute() is a callback function for result sets. Instead
of returning the results to you for processing, it passes each of the
result sets to the list of functions you provide. It passes to the function
a memcached_st that can be cloned for use in the called function (it can not
be used directly). It also passes a result set which does not need to be freed.
Finally it passes a "context". This is just a pointer to a memory reference
you supply the calling function. Currently only one value is being passed
to each function call. In the future there will be an option to allow this
to be an array.

memcached_mget_execute() and memcached_mget_execute_by_key() is
similar to memcached_mget(), but it may trigger the supplied callbacks
with result sets while sending out the queries. If you try to perform
a really large multiget with memcached_mget() you may encounter a
deadlock in the OS kernel (we fail to write data to the socket because
the input buffer is full). memcached_mget_execute() solves this
problem by processing some of the results before continuing sending
out requests. Please note that this function is only available in the
binary protocol.

memcached_get_by_key() and memcached_mget_by_key() behave in a similar nature
as memcached_get() and memcached_mget(). The difference is that they take
a master key that is used for determining which server an object was stored
if key partitioning was used for storage.

All of the above functions are not testsed when the \ ``MEMCACHED_BEHAVIOR_USE_UDP``\ 
has been set. Executing any of these functions with this behavior on will result in
\ ``MEMCACHED_NOT_SUPPORTED``\  being returned or, for those functions which do not return
a \ ``memcached_return_t``\ , the error function parameter will be set to
\ ``MEMCACHED_NOT_SUPPORTED``\ .


------
RETURN
------


All objects returned must be freed by the calling application.
memcached_get() and memcached_fetch() will return NULL on error. You must
look at the value of error to determine what the actual error was.

MEMCACHED_KEY_TOO_BIG is set to error whenever memcached_fetch() was used
and the key was set larger then MEMCACHED_MAX_KEY, which was the largest
key allowed for the original memcached ascii server.


----
HOME
----


To find out more information please check:
`http://libmemcached.org/ <http://libmemcached.org/>`_



--------
SEE ALSO
--------

:manpage:`memcached(1)` :manpage:`libmemcached(3)` :manpage:`memcached_strerror(3)`
