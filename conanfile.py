import re
import os
from conan import ConanFile
from conan.tools.files import load, copy, collect_libs
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps
from conan.errors import ConanInvalidConfiguration


class InfluxdbCxxConan(ConanFile):
    name = "influxdb-cxx"
    license = "MIT"
    author = "offa <offa@github>"
    url = "https://github.com/offa/influxdb-cxx"
    hompage = url
    description = "InfluxDB C++ client library."
    topics = ("influxdb", "influxdb-client")
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "tests": [True, False],
        "system": [True, False],
        "boost": [True, False]
    }
    default_options = {
        "shared": False,
        "tests": False,
        "system": False,
        "boost": True,
        "boost/*:shared": True,
    }
    exports = ["LICENSE"]
    exports_sources = ("CMakeLists.txt", "src/*", "include/*", "test/*",
                       "cmake/*", "3rd-party/*")

    def set_version(self):
        cmake_lists_content = load(
            self, os.path.join(self.recipe_folder, "CMakeLists.txt"))
        project_match = re.search(r'project\s*\((.+?)\)', cmake_lists_content,
                                  re.DOTALL)

        if not project_match:
            raise ConanInvalidConfiguration(
                "No valid project() statement found in CMakeLists.txt")

        project_params = project_match.group(1).split()
        version_string = project_params[project_params.index("VERSION") + 1]

        if not re.search(r'\d+\.\d+\.\d+(?:\.\d)?', version_string):
            raise ConanInvalidConfiguration(
                "No valid version found in CMakeLists.txt")

        self.version = version_string
        self.output.info(
            f"Project version from CMakeLists.txt: '{self.version}'")

    def requirements(self):
        self.requires("cpr/1.10.0")
        if not self.options.system and self.options.boost:
            self.requires("boost/1.81.0")
        if self.options.tests:
            self.requires("catch2/3.3.1")
            self.requires("trompeloeil/43")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.cache_variables["INFLUXCXX_TESTING"] = self.options.tests
        tc.cache_variables["INFLUXCXX_WITH_BOOST"] = self.options.boost
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

        if self.options.tests:
            cmake.test()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()
        copy(self, pattern="LICENSE", dst="licenses", src=self.source_folder)

    def package_info(self):
        self.cpp_info.libs = collect_libs(self)
        self.cpp_info.set_property("cmake_file_name", "InfluxDB")
        self.cpp_info.set_property("cmake_target_name", "InfluxData::InfluxDB")

    def _configure_cmake(self):
        cmake = CMake(self)
        cmake.configure()
        return cmake
