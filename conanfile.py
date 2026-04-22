from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout
from conan.tools.scm import Git


class LogexConan(ConanFile):
    name = "logex"
    version = "1.0.0"
    url = "https://github.com/stigranyan/Logex"

    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = {"shared": False}

    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("fmt/12.1.0")

    def layout(self):
        cmake_layout(self)

    def source(self):
        git = Git(self)
        git.clone(url=self.url, target=".")
        git.checkout(f"v{self.version}")

    def build(self):
        cmake = CMake(self)
        cmake.configure(variables={"LGX_BUILD_TESTBED": "OFF"})
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.set_property("cmake_target_name", "logex::logex-static")
        self.cpp_info.libs = ["logex-static"]
        self.cpp_info.includedirs = ["include/logex"]
