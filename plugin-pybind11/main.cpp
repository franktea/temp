#include<iostream>
#include <pybind11/embed.h>

namespace py = pybind11;

int main() {
    py::scoped_interpreter guard{};

    // 导入Python模块
    py::module main_module = py::module::import("enc_dec_test");

    // 创建CustomEncDec类的实例
    py::object custom_enc_dec_class = main_module.attr("CustomEncDec");
    py::object custom_enc_dec_instance = custom_enc_dec_class();

    // 调用加密和解密方法
    std::string text = "Hello, World!";
    py::object encrypted_text = custom_enc_dec_instance.attr("Encrypt")(text);
    py::object decrypted_text = custom_enc_dec_instance.attr("Decrypt")(encrypted_text);

    // 输出结果
    std::cout << "Encrypted text: "<< encrypted_text.cast<std::string>()<< std::endl;
    std::cout << "Decrypted text: "<< decrypted_text.cast<std::string>()<< std::endl;

    return 0;
}
