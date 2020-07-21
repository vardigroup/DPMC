class NumpyAPI:
    def __init__(self, entry_type, thread_limit=None):
        self._thread_limit = thread_limit
        if thread_limit is not None:
            import threadpoolctl

            self._thread_limiter = threadpoolctl

        import numpy

        self._numpy = numpy
        self._entry_type = self._get_numpy_type(entry_type)

    def create_tensor(self, shape, default_value=None):
        if default_value is None:
            return self._numpy.empty(shape, dtype=self._entry_type)
        else:
            return self._numpy.full(shape, default_value, dtype=self._entry_type)

    def stack(self, array, **kwargs):
        return self._numpy.stack(array, **kwargs)

    def tensordot(self, a, b, axes):
        return self._numpy.tensordot(a, b, axes)

    def contract(self, network, contraction_tree, log):
        try:
            if self._thread_limit is not None:
                with self._thread_limiter.threadpool_limits(
                    limits=self._thread_limit, user_api="blas"
                ):
                    return network.identify(contraction_tree, self, log)
            else:
                return network.identify(contraction_tree, self, log)
        except MemoryError:
            raise OutOfMemoryError

    def _get_numpy_type(self, entry_type):
        types = {
            "float64": self._numpy.float64,
            "float32": self._numpy.float32,
            "float16": self._numpy.float16,
            "uint": self._numpy.uint64,
            "int": self._numpy.int64,
            "bigint": self._numpy.object,
        }

        if entry_type in types:
            return types[entry_type]
        else:
            raise ValueError("Unknown numpy type %s" % entry_type)

    def get_entry_size(self):
        return self._numpy.dtype(self._entry_type).itemsize


ALL_APIS = {
    "numpy": NumpyAPI,
}
