from conan import ConanFile

class InfluxdbCxxConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    options = {
        "tests": [True, False],
        "system": [True, False],
        "boost": [True, False]
    }
    default_options = {
        "tests": True,
        "system": False,
        "boost": True,
        "boost/*:shared": True,
    }

    def requirements(self):
        self.requires("cpr/1.11.0")
        if not self.options.system and self.options.boost:
            self.requires("boost/1.85.0")
        if self.options.tests:
            self.requires("catch2/3.7.1")
            self.requires("trompeloeil/49")
