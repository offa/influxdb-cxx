from conans import ConanFile, CMake, tools


class InfluxdbCxxConan(ConanFile):
    name = "influxdb-cxx"
    version = "0.5.1"
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
