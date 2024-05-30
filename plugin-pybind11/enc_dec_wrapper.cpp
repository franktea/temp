#include <pybind11/pybind11.h>
#include "enc_dec.h"

namespace py = pybind11;

PYBIND11_MODULE(enc_dec_wrapper, m) {
    py::class_<EncDec>(m, "EncDec")
        .def(py::init<>())
        .def("Encrypt", &EncDec::Encrypt)
        .def("Decrypt", &EncDec::Decrypt);
}
