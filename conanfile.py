import re
import os
from conans import ConanFile, CMake, tools
from conans.errors import ConanInvalidConfiguration

class InfluxdbCxxConan(ConanFile):
    name = "influxdb-cxx"
    license = "MIT"
    author = "offa <offa@github>"
    url = "https://github.com/offa/influxdb-cxx"
    hompage = url
    description = "InfluxDB C++ client library."
    topics = ("influxdb", "influxdb-client")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = {"shared": False,
                       "boost:shared":True,
                       "libcurl:with_openssl": True,
                       "libcurl:shared": True}
    exports = ["LICENSE"]
    exports_sources = ("CMakeLists.txt", "src/*", "include/*", "cmake/*")
    requires = (
        "boost/1.71.0",
        "libcurl/7.72.0 "
    )


    def set_version(self):
        cmake_lists_content = tools.load(os.path.join(self.recipe_folder, "CMakeLists.txt"))
        project_match = re.search(r'project\s*\((.+?)\)', cmake_lists_content, re.DOTALL)

        if not project_match:
            raise ConanInvalidConfiguration("No valid project() statement found in CMakeLists.txt")

        project_params = project_match.group(1).split()
        version_string = project_params[project_params.index("VERSION") + 1]

        if not re.search(r'\d+\.\d+\.\d+(?:\.\d)?', version_string):
            raise ConanInvalidConfiguration("No valid version found in CMakeLists.txt")

        self.version = version_string
        self.output.info("Project version from CMakeLists.txt: '{}'".format(self.version))

    def _configure_cmake(self):
        cmake = CMake(self)
        cmake.definitions["INFLUXCXX_TESTING"] = "OFF"
        cmake.configure(build_folder="build")
        return cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()
        self.copy("LICENSE", dst="licenses")

    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)
