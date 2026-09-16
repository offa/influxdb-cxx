from conan import ConanFile


class InfluxdbCxxConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    options = {"tests": [True, False], "system": [True, False], "boost": [True, False]}
    default_options = {
        "tests": True,
        "system": False,
        "boost": True,
        "boost/*:shared": True,
    }

    def requirements(self):
        self.requires("cpr/1.14.2")
        if not self.options.system and self.options.boost:
            self.requires("boost/1.90.0")
        if self.options.tests:
            self.requires("catch2/3.16.0")
            self.requires("trompeloeil/49")

    # Workaround for conan-center-index #29193
    def configure(self):
        self.options["boost"].without_cobalt = True
